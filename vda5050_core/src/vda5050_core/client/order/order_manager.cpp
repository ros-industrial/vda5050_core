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
#include "vda5050_types/state.hpp"
#include "vda5050_types/action_status.hpp"
#include "vda5050_types/order.hpp"
#include "vda5050_types/node.hpp"

namespace vda5050_core {
namespace order_manager {

OrderManager::OrderManager()
: current_graph_element_index_{0} {};

bool OrderManager::update_current_order(vda5050_types::Order& order, const vda5050_types::State& state)
{
  Order received_order{ order };

  /// Check that this is actually an update order
  if (
    current_order_.has_value() &&
    received_order.order_id == current_order_->order_id)
  {
    /// check if received order is deprecated
    if (received_order.order_update_id < current_order_->order_update_id)
    {
      reject_order();
      
      std::cerr << "OrderManager error: Order update is deprecated." << "\n";

      return false;
    }

    /// check if order update is currently on the vehicle
    else if (
      received_order.order_update_id == current_order_->order_update_id)
    {
      /// discard message as vehicle already has this update
      std::cerr << "OrderManager warning: Received duplicate order update (orderUpdateId="
                << received_order.order_update_id << ") for order "
                << received_order.order_id << ". Discarding message." << '\n';
      
      /// TODO: KIV to change this once the return struct has been decided
      return false;
    }

    else if (is_vehicle_still_executing(state) && is_vehicle_waiting_for_update())
    {
      if (
        received_order.nodes.front().node_id !=
        current_order_->decision_point().node_id)
      {
        std::cerr << "OrderManager error: Order update rejected as nodeIds of the stitching nodes do not match." << "\n";
        reject_order();
        return false;
      }

      else
      {
        /// order update can be accepted as it is a continuation of the currently running order
        /// TODO: Update internal state
        current_order_->stitch_and_set_order_update_id(received_order);
        return true;
      }
    }
    else
    {
      if (received_order.nodes.front().sequence_id != state.last_node_sequence_id && received_order.nodes.front().node_id != state.last_node_id)
      {
        std::cerr << "OrderManager error: Order update rejected as it is not a valid continuation of the previously completed order." << "\n";
        reject_order();
        return false;
      }
      else
      {
        /// order update can be accepted as it is a continuation of the previously executed order
        /// TODO: Update internal state
        current_order_->stitch_and_set_order_update_id(received_order);
        return true;
      }
    }
  }
  else
  {
    /// TODO Check if it is okay to be throwing an error and rejecting the order, as this will only occur due to how we are implementing this in a BTree
    /// update order is rejected as a new order was given instead of an update order
    reject_order();
    return false;
  }
}

bool OrderManager::make_new_order(vda5050_types::Order& order, const vda5050_types::State& state)
{
  Order received_order { order };

  if (
    !current_order_.has_value() ||
    (current_order_.has_value() &&
     received_order.order_id != current_order_->order_id))
  {
    /// if no current order exists, the vehicle can accept a new order
    if (
      !current_order_)
    {
      accept_new_order(received_order);

      return true;
    }
    /// if the vehicle is not carrying out an action and if the vehicle has no horizon, it can accept a new order
    bool vehicle_ready_for_new_order = !is_vehicle_still_executing(state) && !is_vehicle_waiting_for_update();

    /// TODO: This assumes that StateManager sets lastNodeId once the vehicle is within deviation range
    bool node_is_trivially_reachable = received_order.nodes.front().node_id == state.last_node_id;

    if (vehicle_ready_for_new_order && node_is_trivially_reachable)
    {
      accept_new_order(received_order);
      return true;
    }
    else
    {
      /// reject order and throw error
      reject_order();

      if (!vehicle_ready_for_new_order && !node_is_trivially_reachable)
      {
        std::cerr << "OrderManager error: Vehicle is not ready to accept a new order and received order's start node is not trivially reachable." << "\n";
      }
      else if (!vehicle_ready_for_new_order)
      {
        std::cerr << "OrderManager error: Vehicle is not ready to accept a new order. Vehicle is either still executing or waiting for an order update." << "\n";
      }
      else if (!node_is_trivially_reachable)
      {
        std::cerr << "OrderManager error: Received order's start node is not trivially reachable." << "\n";
      }

      return false;
    }
  }
  else
  {
    reject_order();
    throw std::runtime_error(
      "OrderManager error: Expected a new order but was given an order the "
      "same orderId.");

    return false;
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
    throw std::runtime_error("OrderManager error: Base of the given order is empty.");
  }

  if (const vda5050_types::Node* pNode = std::get_if<vda5050_types::Node>(&base_.back()))
  {
    decision_point_ = *pNode;
  }
  else
  {
    throw std::runtime_error("OrderManager error: Last graph element in the given order is not of type vda50505_types::Node.");
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
    throw std::runtime_error(
      "Order error: Stitching node of the incoming order does not match the "
      "current order's decision point.");
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


}  // namespace order_manager
}  // namespace vda5050_core
