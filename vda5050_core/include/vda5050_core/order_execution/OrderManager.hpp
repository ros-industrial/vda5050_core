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

/// \brief Class that handles incoming orders on an AGV.
class OrderManager
{
public:
  OrderManager(IStateManager& sm);

  /// \brief Checks that an order is valid and processes the order
  void validate_and_parse_order(order::Order);

  /// \brief Returns the next graph element of the current order that is to be executed
  ///
  /// \return A Node or Edge object that is to be executed next. Returns
  std::optional<order_graph_element::OrderGraphElement> next_graph_element();

private:
  /// \brief Reference to the StateManager running on the AGV
  IStateManager& state_manager_;

  /// \brief The order that is currently on the vehicle
  std::optional<order::Order> current_order_;

  /// \brief The index of current order's graph element that is to be dispatched next
  size_t current_graph_element_index_;

  bool
    json_validator_;  /// TODO: (shawnkchan) I assume this needs to be modular, but using this as a placeholder for now

  order_graph_validator::OrderGraphValidator graph_validator_;

  /// \brief Checks that vehicle is ready to accept a new order
  ///
  /// \return
  bool is_vehicle_ready_for_new_order();

  /// \brief Checks that vehicle is no longer executing an order
  ///
  /// \return
  bool is_vehicle_still_executing();

  /// \brief Checks that vehicle is not waiting for an update (vehicle has no horizon)
  ///
  /// \return
  bool is_vehicle_waiting_for_update();

  /// \brief Checks if the received order's first node is within range of the AGV's current position
  ///
  /// \param start_node
  ///
  /// \return
  bool is_node_trivially_reachable(node::Node& start_node);

  bool is_update_order_valid_continuation(order::Order& order);

  /// \brief Accept a validated order
  ///
  /// \param valid_order The validated order
  void accept_new_order(order::Order order);

  /// \brief Checks if an order's graph is valid
  bool is_graph_valid();

  void reject_order();

  /// \brief Checks if orderId of order is different to the orderId of the order that the vehicle currently holds
  //
  /// \param order the newly received order
  ///
  /// \return
  bool is_new_order(order::Order order);
};

}  // namespace order_manager
}  // namespace vda5050_core
#endif  // VDA5050_CORE__ORDER_EXECUTION__ORDER_MANAGER_HPP_
