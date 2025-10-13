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

#ifndef VDA5050_CORE__ORDER_EXECUTION__ORDER_MANAGER_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__ORDER_MANAGER_HPP_

#include <optional>
#include <vector>

#include "vda5050_core/order_execution/IStateManager.hpp"
#include "vda5050_core/order_execution/Order.hpp"
#include "vda5050_core/order_execution/OrderGraphValidator.hpp"

namespace vda5050_core {
namespace order_manager {

/// \brief Class that handles incoming orders on a vehicle.
class OrderManager
{
public:
  OrderManager(IStateManager& sm);

  /// \brief Checks that an order is valid and processes the order
  void validate_and_parse_order(order::Order);

  /// \brief Returns the next graph element of the current order that is to be executed
  ///
  /// \return The graph element that is to be executed next.
  std::optional<order_graph_element::OrderGraphElement> next_graph_element();

private:
  /// \brief Reference to the StateManager running on the vehicle
  IStateManager& state_manager_;

  /// \brief The order that is currently on the vehicle
  std::optional<order::Order> current_order_;

  /// \brief The index of current order's graph element that is to be dispatched next
  size_t current_graph_element_index_;

  bool
    json_validator_;  /// TODO: (shawnkchan) I assume this needs to be modular, but using this as a placeholder for now

  order_graph_validator::OrderGraphValidator graph_validator_;

  /// \brief Checks that the vehicle is ready to accept a new order
  ///
  /// \return True if ready to accept a new order
  bool is_vehicle_ready_for_new_order();

  /// \brief Checks that vehicle is no longer executing an order
  ///
  /// \return True if vehicle is not executing an order
  bool is_vehicle_still_executing();

  /// \brief Checks that vehicle is not waiting for an update to its current order (i.e: the vehicle's current order has no horizon)
  ///
  /// \return True if vehicle is waiting for an update to its current order
  bool is_vehicle_waiting_for_update();

  /// \brief Checks if the received order's first node is within range of the vehicle's current position
  ///
  /// \param start_node The first node of the received order
  ///
  /// \return True if start_node can be reached from the vehicle's current position
  bool is_node_trivially_reachable(node::Node& start_node);

  /// \brief Checks if order_update is a valid continuation from the current order on the vehicle
  ///
  /// \param order_update The incoming order update
  ///
  /// \return True if order_update is a valid continuation
  bool is_update_order_valid_continuation(order::Order& order_update);

  /// \brief Accept a validated order and set it to the vehicle's current_order_
  ///
  /// \param valid_order The validated order
  void accept_new_order(order::Order order);

  /// \brief Checks if an order's graph is valid
  bool is_graph_valid();

  /// \brief Reject an order
  /// TODO This is currently incomplete. To find out what logic needs to be handled during order rejection
  void reject_order();

  /// \brief Checks if orderId of order is different to the orderId of the order that the vehicle currently holds
  //
  /// \param order The received order
  ///
  /// \return True if the received order's orderId is different
  bool is_new_order(order::Order order);
};

}  // namespace order_manager
}  // namespace vda5050_core
#endif  // VDA5050_CORE__ORDER_EXECUTION__ORDER_MANAGER_HPP_
