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

#ifndef VDA5050_CORE__CLIENT__ORDER__ORDER_MANAGER_HPP_
#define VDA5050_CORE__CLIENT__ORDER__ORDER_MANAGER_HPP_

#include <optional>
#include <vector>
#include <cstdint>
#include <memory>
#include <variant>

#include "vda5050_core/client/order/validation_result.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_types/state.hpp"
#include "vda5050_types/order.hpp"

namespace vda5050_core {
namespace client {
namespace order {

/// \brief Class that handles incoming orders on a vehicle.
class OrderManager
{
public:
  /// \brief OrderManager constructor
  ///
  /// \param sm Reference to the StateManager tracking the vehicle's current state.
  OrderManager();

  /// TODO: i think it may make more sense to pass in a snapshot of the state 
  /// \brief Updates the current order on the vehicle.
  ///
  /// \param order The order update to be applied on the current order.
  /// \param state A snapshot of the vehicle's current state 
  ///
  /// \return True if order update has been accepted by the order manager, false otherwise.
  ValidationResult update_current_order(vda5050_types::Order& order, const vda5050_types::State& state);

  /// \brief Puts a new order on the vehicle
  ///
  /// \param order The new Order object to be stored and executed by the vehicle
  /// \param state A snapshot of the vehicle's current state
  ///
  /// \return True if the new order has been accepted by the order manager, false otherwise
  ValidationResult make_new_order(vda5050_types::Order& order, const vda5050_types::State& state);

  /// \brief Returns the next graph element of the current order that is to be executed.
  ///
  /// \return The graph element that is to be executed next.
  const std::optional<std::variant<vda5050_types::Node, vda5050_types::Edge>> next_graph_element();

private:
  class Order : public vda5050_types::Order
  {
    public:
      Order(vda5050_types::Order& order);

      const vda5050_types::Node& decision_point() const
      {
        return decision_point_;
      }

      const std::vector<std::variant<vda5050_types::Node, vda5050_types::Edge>>& graph() const
      {
        return graph_;
      }

      const std::vector<std::variant<vda5050_types::Node, vda5050_types::Edge>>& base() const
      {
        return base_;
      }

      const std::vector<std::variant<vda5050_types::Node, vda5050_types::Edge>>& horizon() const
      {
        return horizon_;
      }

      /// \brief Stitch this order with another order and update the order_update_id if stitching is successful.
      ///
      /// \param order The incoming order to stitch to this order.
      void stitch_and_set_order_update_id(Order order);
    
    private:
      /// \brief The graph created by the nodes and edges of this order.
      std::vector<std::variant<vda5050_types::Node, vda5050_types::Edge>> graph_;

      /// \brief The base of this order. Contains the released nodes and edges sorted in ascending sequenceId
      std::vector<std::variant<vda5050_types::Node, vda5050_types::Edge>> base_;

      /// \brief The horizion of this order. Contains the unreleased nodes and edges sorted in ascending sequenceId
      std::vector<std::variant<vda5050_types::Node, vda5050_types::Edge>> horizon_;

      /// \brief The last node in this order's base, or the last released node according to sequenceId
      vda5050_types::Node decision_point_;

      /// \brief Populate the graph_ member variable.
      void populate_graph();

      /// \brief Populate the base_ and horizon_ member variables
      void populate_base_and_horizon();

      /// \brief Set decision_point_ to the last released node in the order.
      void set_decision_point();

      /// \brief Stitch this order with another order.
      void stitch_order(Order order);

      /// \brief Set a new order_update_id.
      void set_order_update_id(uint32_t new_order_update_id); 
  };

  /// \brief The order that is currently on the vehicle
  std::optional<Order> current_order_;

  /// \brief The index of current order's graph element that is to be dispatched next
  size_t current_graph_element_index_;

  /// TODO Should we be doing JSON validation here?
  /// \brief Reference to the JSON validator
  bool
    json_validator_;  /// TODO: (shawnkchan) I assume this needs to be modular, but using this as a placeholder for now

  /// \brief Checks that vehicle is no longer executing an order
  ///
  /// \param state A snapshot of the vehicle's current state
  ///
  /// \return True if vehicle is not executing an order
  bool is_vehicle_still_executing(const vda5050_types::State& state);

  /// \brief Checks that vehicle is not waiting for an update to its current order (i.e: the vehicle's current order has no horizon)
  ///
  /// \return True if vehicle is waiting for an update to its current order
  bool is_vehicle_waiting_for_update();

  /// \brief Checks if order_update is a valid continuation from the current order on the vehicle
  ///
  /// \param order_update The incoming order update
  ///
  /// \return True if order_update is a valid continuation
  bool is_update_order_valid_continuation(vda5050_types::Order& order_update);

  /// \brief Accept a validated order and set it to the vehicle's current_order_
  ///
  /// \param valid_order The validated order
  void accept_new_order(vda5050_types::Order order);

  /// \brief Reject an order
  /// TODO This is currently incomplete. To find out what logic needs to be handled during order rejection
  void reject_order();

  /// \brief Helper function to return a vector of errors for an invalid continuation error 
  ///
  /// \param order_start_node_sequence_id sequenceId of the first node in the incoming order
  /// \param state_last_node_sequence_id lastNodeSequenceId of the vehicle's state 
  /// \param order_start_node_node_id nodeId of the first node in the incoming order
  /// \param state_last_node_id nodeId of the vehicle's state
  /// 
  /// \return Vector of Error structs for each error that occurred.
  std::vector<vda5050_types::Error> invalid_continuation_errors(const uint32_t order_start_node_sequence_id, const uint32_t state_last_node_sequence_id, const std::string order_start_node_node_id, const std::string state_last_node_id);
};

}  // namespace order
}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__ORDER__ORDER_MANAGER_HPP_
