/**
* Copyright (C) 2025 ROS-Industrial Consortium Asia Pacific
* Advanced Remanufacturing and Technology Centre
* A*STAR Research Entities (Co. Registration No. 199702110H)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <iostream>
#include <stdexcept>
#include <memory>
#include <variant>
#include <algorithm>

#include "vda5050_core/client/order/order_manager.hpp"
#include "vda5050_core/client/order/validation_result.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_types/action_status.hpp"
#include "vda5050_types/error.hpp"
#include "vda5050_types/error_reference.hpp"
#include "vda5050_types/error_level.hpp"
#include "vda5050_types/node.hpp"
#include "vda5050_types/order.hpp"
#include "vda5050_types/state.hpp"

namespace vda5050_core {
namespace client {
namespace order {

OrderManager::OrderManager()
: current_graph_element_index_{0} {};

ValidationResult OrderManager::update_current_order(vda5050_types::Order& order, const vda5050_types::State& state)
{
  Order received_order{order};
  ValidationResult res {true, {}};

  /// Check that this is actually an update order
  if (
    current_order_.has_value() &&
    received_order.order_id == current_order_->order_id)
  {
    /// check if received order is deprecated
    if (received_order.order_update_id < current_order_->order_update_id)
    {
      reject_order();

      VDA5050_ERROR("Order Manager Error: orderUpdateError. Received order's order_update_id is less than the vehicle's current order order_update_id.");

      /// create error struct
      vda5050_types::Error order_update_deprecated_error {};
      order_update_deprecated_error.error_type = "Order Manager Error: orderUpdateError";
      order_update_deprecated_error.error_description = "order_update_id of the incoming order is less than the order_update_id of the order currently on the vehicle";
      order_update_deprecated_error.error_level = vda5050_types::ErrorLevel::WARNING;

      /// create error references for this error
      vda5050_types::ErrorReference order_id_error_reference {"orderId", received_order.order_id};
      vda5050_types::ErrorReference order_update_id_error_reference {"orderUpdateId", std::to_string(received_order.order_update_id)};
      
      /// add error references to this error
      order_update_deprecated_error.error_references = {order_id_error_reference, order_update_id_error_reference};

      res.valid = false;
      res.errors.push_back(order_update_deprecated_error);
      return res;
    }

    /// check if order update is currently on the vehicle
    else if (
      received_order.order_update_id == current_order_->order_update_id)
    {
      VDA5050_WARN("Order Manager Warning: Received a duplicate order update. Discarding Order message.");
      
      /// create error struct
      vda5050_types::Error duplicate_order_update_error {};
      duplicate_order_update_error.error_type = "Order Manager Error";
      duplicate_order_update_error.error_description = "order_update_id of the incoming order is equal to the order_update_id of the order currently on the vehicle.";
      duplicate_order_update_error.error_level = vda5050_types::ErrorLevel::WARNING;

      /// create error references for this error
      vda5050_types::ErrorReference order_id_error_reference {"orderId", received_order.order_id};
      vda5050_types::ErrorReference order_update_id_error_reference {"orderUpdateId", std::to_string(received_order.order_update_id)};

      /// add error references to this error
      duplicate_order_update_error.error_references = {order_id_error_reference, order_update_id_error_reference};
      
      res.valid = false;
      res.errors.push_back(duplicate_order_update_error);
      return res;
    }

    else if (is_vehicle_still_executing(state) && is_vehicle_waiting_for_update())
    {
      if (
        received_order.nodes.front().node_id !=
        current_order_->decision_point().node_id)
      {
        VDA5050_ERROR("Order Manager Error: orderUpdateError. Order update has been rejected as nodeIds of the stitching nodes do not match.");

        reject_order();

        vda5050_types::Error mismatched_stitching_node_error {};
        mismatched_stitching_node_error.error_type = "Order Manager Error: orderUpdateError";
        mismatched_stitching_node_error.error_description = "node_id of the received order's first node does not match the node_id of the vehicle's current decision point. Order update is unable to be stitched with the current order on the vehicle.";
        mismatched_stitching_node_error.error_level = vda5050_types::ErrorLevel::WARNING;

        vda5050_types::ErrorReference order_id_error_reference {"orderId", received_order.order_id};
        vda5050_types::ErrorReference order_update_id_error_reference {"orderUpdateId", std::to_string(received_order.order_update_id)};
        
        mismatched_stitching_node_error.error_references = {order_id_error_reference, order_update_id_error_reference};

        res.valid = false;
        res.errors.push_back(mismatched_stitching_node_error);
        return res;
      }

      else
      {
        /// order update can be accepted as it is a continuation of the currently running order
        current_order_->stitch_and_set_order_update_id(received_order);
        return res;
      }
    }
    else
    {
      if (received_order.nodes.front().sequence_id != state.last_node_sequence_id && received_order.nodes.front().node_id != state.last_node_id)
      {
        VDA5050_ERROR("Order Manager Error: orderUpdateError. Order update has been rejected as it is not a valid continuation of the previously completed order.");
        
        reject_order();

        res.valid = false;
        res.errors = invalid_continuation_errors(received_order, state);
        return res;
      }
      else
      {
        /// order update can be accepted as it is a continuation of the previously executed order
        /// TODO: Update internal state
        current_order_->stitch_and_set_order_update_id(received_order);
        return res;
      }
    }
  }
  else
  {
    VDA5050_ERROR("Order Manager Error: Update order has been rejected as its order_id does not match the order_id of the order currently on the vehicle.");

    reject_order();

    vda5050_types::Error not_an_update_order_error {};
    not_an_update_order_error.error_type = "Order Manager Error";
    not_an_update_order_error.error_description = "order_id of the received order does not match the order_id of the order currently on the vehicle.";

    vda5050_types::ErrorReference order_id_error_reference {"orderId", received_order.order_id};

    not_an_update_order_error.error_references = {order_id_error_reference};

    res.valid = false;
    res.errors.push_back(not_an_update_order_error);
    return res;
  }
}

ValidationResult OrderManager::make_new_order(vda5050_types::Order& order, const vda5050_types::State& state)
{
  Order received_order { order };
  ValidationResult res {true, {}};

  if (
    !current_order_.has_value() ||
    (current_order_.has_value() &&
     received_order.order_id != current_order_->order_id))
  {
    /// if no current order exists, the vehicle can accept a new order
    if (!current_order_)
    {
      accept_new_order(received_order);
      return res;
    }
    /// if the vehicle is not carrying out an action and if the vehicle has no horizon, it can accept a new order
    bool vehicle_ready_for_new_order = !is_vehicle_still_executing(state) && !is_vehicle_waiting_for_update();

    /// TODO: This assumes that StateManager sets lastNodeId once the vehicle is within deviation range
    bool node_is_trivially_reachable = received_order.nodes.front().node_id == state.last_node_id;

    if (vehicle_ready_for_new_order && node_is_trivially_reachable)
    {
      accept_new_order(received_order);
      return res;
    }
    else
    {
      /// reject order and throw error
      reject_order();

      if (!vehicle_ready_for_new_order)
      {
        VDA5050_ERROR("Order Manager Error: Vehicle is not ready to accept a new order. Vehicle is either still executing or waiting for an order update.");

        vda5050_types::Error not_ready_for_new_order_error {};
        not_ready_for_new_order_error.error_type = "Order Manager Error";
        not_ready_for_new_order_error.error_level = vda5050_types::ErrorLevel::WARNING;
        not_ready_for_new_order_error.error_description = "Vehicle is not ready for a new order as it is still executing its current order or still has a horizon and hence requires and order update first.";

        res.errors.push_back(not_ready_for_new_order_error);
      }

      if (!node_is_trivially_reachable)
      {
        VDA5050_ERROR("Order Manager Error: noRouteErrror. Received order's start node is not trivially reachable.");

        vda5050_types::Error not_trivially_reachable_error {};
        not_trivially_reachable_error.error_type = "Order Manager Error: noRouteError";
        not_trivially_reachable_error.error_level = vda5050_types::ErrorLevel::WARNING;
        not_trivially_reachable_error.error_description = "Vehicle cannot reach the new order's first node from the vehicle's last reached node";

        vda5050_types::ErrorReference first_node_id_error_reference {"nodeId", received_order.nodes.front().node_id};

        not_trivially_reachable_error.error_references = {first_node_id_error_reference};
        
        res.errors.push_back(not_trivially_reachable_error);
      }

      res.valid = false;
      return res;
    }
  }
  else
  {
    reject_order();

    VDA5050_ERROR("Order Manager Error: Expected a new order but was given an order with the same order_id");

    vda5050_types::Error duplicate_order_error {};
    duplicate_order_error.error_type = "Order Manager Error";
    duplicate_order_error.error_level = vda5050_types::ErrorLevel::WARNING;
    duplicate_order_error.error_description = "OrderManager::make_new_order() was given an order with the same orderId as the order currently on the vehicle.";

    vda5050_types::ErrorReference new_order_id_error_reference {"orderId", received_order.order_id};
    vda5050_types::ErrorReference current_order_id_error_reference {"orderId", current_order_->order_id};

    duplicate_order_error.error_references = {new_order_id_error_reference, current_order_id_error_reference};

    res.valid = false;
    res.errors.push_back(duplicate_order_error);
    return res;
  }
}

const std::optional<std::variant<vda5050_types::Node, vda5050_types::Edge>> OrderManager::next_graph_element()
{
  if (current_graph_element_index_ >= current_order_->base().size())
  {
    return std::nullopt;
  }
  std::variant<vda5050_types::Node, vda5050_types::Edge> graph_element = current_order_->base().at(current_graph_element_index_);

  current_graph_element_index_++;
  
  return graph_element;
}

bool OrderManager::is_vehicle_still_executing(const vda5050_types::State& state)
{
  bool node_states_empty = state.node_states.empty();
  bool action_states_executing { false };

  if (!state.action_states.empty())
  {
    for (const auto& action_state : state.action_states)
    {
      if (action_state.action_status != vda5050_types::ActionStatus::FINISHED && action_state.action_status != vda5050_types::ActionStatus::FAILED)
      {
        action_states_executing = true;
        break;
      } 
    }
  }
  return !node_states_empty || action_states_executing;
}

bool OrderManager::is_vehicle_waiting_for_update()
{
  /// if horizon size is not 0, vehicle is waiting on an update
  if (current_order_->horizon().size() != 0)
  {
    return true;
  }
  return false;
}

void OrderManager::accept_new_order(vda5050_types::Order order)
{
  /// set the state of the current order to the newly accepted order. order_id_ and order_update_id will be updated.
  current_order_ = order;

  /// set the index to the start of the new order
  current_graph_element_index_ = 0;
}

void OrderManager::reject_order()
{
  /// TODO: this will eventually return some sort of struct that BTree will use 
  return;
}

std::vector<vda5050_types::Error> OrderManager::invalid_continuation_errors(const Order& received_order, const vda5050_types::State& state)
{
  uint32_t order_start_node_sequence_id {received_order.nodes.front().sequence_id};
  uint32_t state_last_node_sequence_id {state.last_node_sequence_id};
  std::string order_start_node_node_id {received_order.nodes.front().node_id};

  /// vector to hold all errors generated from an invalid continuation
  std::vector<vda5050_types::Error> errors;

  /// error references for the received order
  vda5050_types::ErrorReference order_id_error_reference {"orderId", received_order.order_id};
  vda5050_types::ErrorReference order_update_id_error_reference {"orderUpdateId", std::to_string(received_order.order_update_id)};

  /// append Error struct to errors array if the sequence_ids are not equal
  vda5050_types::Error mismatched_sequence_id_error {};
  if (order_start_node_sequence_id != state_last_node_sequence_id)
  {
    mismatched_sequence_id_error.error_type = "Order Manager Error: orderUpdateError";
    mismatched_sequence_id_error.error_description = "sequence_id of the incoming order's first node does not match the vehicle state's last_node_sequence_id.";
    mismatched_sequence_id_error.error_level = vda5050_types::ErrorLevel::WARNING;
    mismatched_sequence_id_error.error_references = {order_id_error_reference, order_update_id_error_reference};
    
    errors.push_back(mismatched_sequence_id_error);
  }
  
  /// append Error struct to errors array if node_ids are not equal
  vda5050_types::Error mismatched_node_id_error {};
  if (order_start_node_node_id != state.last_node_id)
  {
    mismatched_node_id_error.error_type = "Order Manager Error: orderUpdateError";
    mismatched_node_id_error.error_description = "node_id of the incoming order's first node does not match the vehicle state's last_node_id.";
    mismatched_node_id_error.error_level = vda5050_types::ErrorLevel::WARNING;
    mismatched_node_id_error.error_references = {order_id_error_reference, order_update_id_error_reference};

    errors.push_back(mismatched_node_id_error);
  }

  return errors;
}

OrderManager::Order::Order(vda5050_types::Order& order)
: vda5050_types::Order(order)
{
  populate_graph();
  populate_base_and_horizon();
  set_decision_point();
}

void OrderManager::Order::stitch_and_set_order_update_id(Order order)
{
  try
  {
    stitch_order(order);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }

  set_order_update_id(order.order_update_id);
}

void OrderManager::Order::populate_graph()
{
  auto getSequenceId
  {
    [](auto const& graph_element)
    {
      return graph_element.sequence_id;
    }
  };

  auto sequenceIdComparison
  {
    [&getSequenceId](auto const& a, auto const& b)
    {
      return std::visit(getSequenceId, a) < std::visit(getSequenceId, b);
    }
  };

  for (vda5050_types::Node& n : nodes)
  {
    graph_.push_back(n);
  }

  for (vda5050_types::Edge& e : edges)
  {
    graph_.push_back(e);
  }

  std::sort(graph_.begin(), graph_.end(), sequenceIdComparison);
}

void OrderManager::Order::populate_base_and_horizon()
{
  auto isReleased
  {
    [](auto&& graph_element) -> bool
    {
      return graph_element.released;
    }
  };

  for (const auto& graph_element : graph_)
  {
    bool is_released = std::visit(isReleased, graph_element);

    if (is_released)
    {
      base_.push_back(graph_element);
    }
    else
    {
      horizon_.push_back(graph_element);
    }
  }
}

void OrderManager::Order::set_decision_point()
{
  if (base_.empty())
  {
    VDA5050_FATAL("Order Manager Error: Base of the given order is empty.");
    return;
  }

  if (const vda5050_types::Node* pNode = std::get_if<vda5050_types::Node>(&base_.back()))
  {
    decision_point_ = *pNode;
  }
  else
  {
    VDA5050_FATAL("Order Manager Error: Last graph element in the given order is not of type vda50505_types::Node.");
  }
}

/// stitch this order with another order. stitching node could be horizon or base node.
void OrderManager::Order::stitch_order(Order order)
{
  /// check that decision point of this order matches with the first node of the incoming order and if the first node of the incoming order is released
  vda5050_types::Node stitching_node{ order.nodes.front() };
  if (
    decision_point_.node_id != stitching_node.node_id ||
    !stitching_node.released)
  {
    VDA5050_FATAL("Order Manager Error: Stitching node of the incoming order does not match the current order's decision point.");
    return;
  }

  /// concatenate the current and incoming graphs
  graph_.pop_back();  /// pop the last node from the current graph
  graph_.insert(graph_.end(), order.graph().begin(), order.graph().end());

  /// update the base and horizon of the current order
  populate_base_and_horizon();
}

void OrderManager::Order::set_order_update_id(uint32_t new_order_update_id)
{
  order_update_id = new_order_update_id;
}

}  // namespace order
}  // namespace client
}  // namespace vda5050_core
