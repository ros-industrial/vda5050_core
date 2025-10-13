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

#include "vda5050_core/order_execution/OrderManager.hpp"

namespace vda5050_core {
namespace order_manager {

OrderManager::OrderManager(IStateManager& sm)
: state_manager_{sm}, current_graph_element_index_{0} {};

void OrderManager::validate_and_parse_order(order::Order received_order)
{
  /// TODO validate JSON schema using validator

  /// check that graph of the received order is valid
  if (!graph_validator_.is_valid_graph(
        received_order.nodes(), received_order.edges()))
  {
    reject_order();
    throw std::runtime_error(
      "OrderManager error: Graph of the received order is invalid.");
  }
  /// received order is an update of a current order
  if (
    current_order_.has_value() &&
    received_order.order_id() == current_order_->order_id())
  {
    /// check if received order is deprecated
    if (received_order.order_update_id() < current_order_->order_update_id())
    {
      reject_order();
      throw std::runtime_error(
        "OrderManager error: Order update is deprecated.");
    }

    /// check if order update is currently on the vehicle
    else if (
      received_order.order_update_id() == current_order_->order_update_id())
    {
      /// discard message as vehicle already has this update

      /// TODO Is this a sufficient method to notify of a discarded message?
      std::cerr << "OrderManager warning: Received duplicate order update (ID: "
                << received_order.order_update_id() << ") for order "
                << received_order.order_id() << ". Discarding message." << '\n';
    }
    /// is the vehicle still executing the current order/waiting for an update?
    else if (is_vehicle_still_executing() && is_vehicle_waiting_for_update())
    {
      if (
        received_order.nodes().front().node_id() !=
        current_order_->decision_point().node_id())
      {
        reject_order();
        throw std::runtime_error(
          "OrderManager error: nodeIds of the stitching nodes do not match.");
      }

      else
      {
        /// TODO call StateManager to clear horizon (OR call some API to update the state)
        state_manager_.clear_horizon();

        current_order_->stitch_and_set_order_update_id(received_order);

        /// TODO call StateManager to append states to the ones that are currently running
        state_manager_.append_states_for_update(received_order);
      }
    }
    else
    {
      if (
        (received_order.nodes().front().sequence_id() !=
         state_manager_.last_node_sequence_id()) &&
        (received_order.nodes().front().node_id() !=
         state_manager_.last_node_id()))
      {
        reject_order();
        throw std::runtime_error(
          "OrderManager error: Order update is not a valid continuation of the "
          "previously compeleted order.");
      }
      else
      {
        /// TODO call StateManager to populate newly added states
        state_manager_.update_current_order(received_order);

        current_order_->stitch_and_set_order_update_id(received_order);
      }
    }
  }
  /// received order is new
  else
  {
    bool vehicle_ready_for_new_order = is_vehicle_ready_for_new_order();
    bool node_is_trivially_reachable =
      is_node_trivially_reachable(received_order.nodes().front());

    /// if no current order exists we can immediately accept it
    if (
      !current_order_ ||
      (vehicle_ready_for_new_order && node_is_trivially_reachable))
    {
      accept_new_order(received_order);
    }
    else
    {
      /// reject order and throw error
      reject_order();

      if (!vehicle_ready_for_new_order && !node_is_trivially_reachable)
      {
        throw std::runtime_error(
          "OrderManager error: Vehicle is not ready to accept a new order and "
          "received order's start node is not trivially reachable.");
      }
      else if (!vehicle_ready_for_new_order)
      {
        throw std::runtime_error(
          "OrderManager error: Vehicle is not ready to accept a new order. "
          "Vehicle is either still executing or waiting for an order update to "
          "its order's Horizon.");
      }
      else if (!node_is_trivially_reachable)
      {
        throw std::runtime_error(
          "OrderManager error: Received order's start node is not trivially "
          "reachable.");
      }
    }
  }
  return;
}

std::optional<order_graph_element::OrderGraphElement>
OrderManager::next_graph_element()
{
  /// TODO Might need to redesign this to enable the nodeId, edgeId, etc to be accessed.
  if (current_graph_element_index_ >= current_order_->base().size())
  {
    return std::nullopt;
  }
  order_graph_element::OrderGraphElement graph_element =
    current_order_->base().at(current_graph_element_index_);

  current_graph_element_index_++;

  return graph_element;
}

bool OrderManager::is_vehicle_ready_for_new_order()
{
  if (is_vehicle_still_executing() && is_vehicle_waiting_for_update())
  {
    return false;
  }
  return true;
}

bool OrderManager::is_vehicle_still_executing()
{
  bool node_states_empty =
    state_manager_.node_states_empty();  /// check if node states are empty
  bool action_states_executing = state_manager_.action_states_still_executing();
  bool vehicle_is_executing = !node_states_empty && action_states_executing;

  return vehicle_is_executing;
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

bool OrderManager::is_node_trivially_reachable(node::Node& start_node)
{
  /// check if the vehicle is on the received order's start node
  std::string last_node_id =
    state_manager_
      .last_node_id();  /// query for lastNodeId, or the current node that the vehicle is on (Note to self: this node is guaranteed to be in the current order)
  std::string start_node_id = start_node.node_id();
  if (last_node_id == start_node_id)
  {
    return true;
  }

  /// No need to check if within deviation range as the StateManager should set lastNodeId appropriately if the vehicle is within deviation range
  return false;
}

void OrderManager::accept_new_order(order::Order order)
{
  /// TODO tell StateManager to cleanup anything to do with previous order
  state_manager_.cleanup_previous_order();

  /// TODO pass stateManager the new order (set orderId, orderUpdateId, populate new states)
  state_manager_.set_new_order(order);

  /// update the current order on the AGV to the newly accepted order. orderId and orderUpdateId will be updated.
  current_order_ = order;

  /// set the index to the start of the new order
  current_graph_element_index_ = 0;
}

void OrderManager::reject_order()
{
  /// TODO: Implement any logic for order rejection
  return;
}

}  // namespace order_manager
}  // namespace vda5050_core
