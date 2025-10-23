/*
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

#ifndef VDA5050_CORE__STATUS_MANAGER__STATUS_MANAGER_HPP_
#define VDA5050_CORE__STATUS_MANAGER__STATUS_MANAGER_HPP_

#include <cstdint>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>

#include "vda5050_core/order_execution/edge.hpp"
#include "vda5050_core/order_execution/node.hpp"
#include "vda5050_core/order_execution/order.hpp"

#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/battery_state.hpp"
#include "vda5050_core/types/error.hpp"
#include "vda5050_core/types/info.hpp"
#include "vda5050_core/types/load.hpp"
#include "vda5050_core/types/operating_mode.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/safety_state.hpp"
#include "vda5050_core/types/state.hpp"

namespace vda5050_core {

namespace state_manager {

using namespace vda5050_core::types;

class StateManager
{
private:
  /// \brief the mutex protecting the data
  mutable std::shared_mutex mutex_;

  /// \brief Internal State of the AGV
  State robot_state_;

  /// \brief the distance since the last node of the AGV
  std::optional<double> distance_since_last_node_;

public:
  /// \brief Set the current AGV position
  /// \param agv_position the agv position
  void set_agv_position(const std::optional<AgvPosition>& agv_position);

  /// \brief Get the current AGV position (if set)
  /// \return std::optional<AGVPosition> the current AGV position of std::nullopt
  std::optional<AgvPosition> get_agv_position();

  /// \brief Set the current velocity
  /// \param velocity the velocity
  void set_velocity(const std::optional<Velocity>& velocity);

  /// \brief Get the Velocity (if set)
  /// \return std::optional<Velocity> the velocity of std::nullopt
  std::optional<Velocity> get_velocity() const;

  /// \brief Set the driving flag of the AGV
  /// \param driving is the agv driving?
  /// \return true, if the driving flag changed
  bool set_driving(bool driving);

  /// \brief Set the distance since the last node as in vda5050
  /// \param distance_since_last_node the new distance since the last node
  void set_distance_since_last_node(double distance_since_last_node);

  /// \brief Reset the distance since the last node
  void reset_distance_since_last_node();

  /// \brief Get the current distance since the last node.
  /// \return The current distance since the last node, or std::nullopt if not set.
  std::optional<double> get_distance_since_last_node() const;

  /// \brief Add a new load to the state
  /// \param load  the load to add
  /// \return true if the loads changed (in this case always)
  bool add_load(const Load& load);

  /// \brief Remove a load by it's id from the loads array
  /// \param load_id the id of the load to remove
  /// \return true if at least one load was removed
  bool remove_load(std::string_view load_id);

  /// \brief Get the current loads
  /// \return const std::vector<Load>& the current loads
  const std::vector<Load>& get_loads();

  /// \brief Set the current operating mode of the AGV
  /// \param operating_mode the new operating mode
  /// \return true if the operating mode changed
  bool set_operating_mode(OperatingMode operating_mode);

  /// \brief Get the current operating mode from the state
  /// \return OperatingMode the current operating mode
  OperatingMode get_operating_mode();

  /// \brief Set the current battery state of the AGV
  /// \param battery_state the battery state
  void set_battery_state(const BatteryState& battery_state);

  /// \brief Get the current battery state from the state
  /// \return const BatteryState& the current battery state
  const BatteryState& get_battery_state();

  /// \brief Set the current safety state of the AGV
  /// \param safety_state the safety state
  /// \return true if the state changed
  bool set_safety_state(const SafetyState& safety_state);

  /// \brief Get the current safety state from the state
  /// \return const SafetyState& the current safety state
  const SafetyState& get_safety_state();

  /// \brief Set the request new base flag to true
  void request_new_base();

  /// \brief Add an error to the state
  /// \param error the error to add
  /// \return true if the errors array changed (in this case always)
  bool add_error(const Error& error);

  /// \brief Get a copy of the current errors
  /// \return std::vector<Error>
  std::vector<Error> get_errors() const;

  /// \brief Add a new information to the state
  /// \param info the information to add
  void add_info(const Info& info);

  /// \brief Dump all data to a vda5050::State
  /// \param state the state to write to
  void dump_to(State& state);

  /// \brief Get the nodeId of the latest node the AGV has reached.
  /// \return The last reached node's ID.
  std::string get_last_node_id() const;

  /// \brief Get the sequenceId of the latest node the AGV has reached.
  /// \return The last reached node's sequence ID.
  uint32_t get_last_node_sequence_id() const;

  /// \brief Check whether the maintained nodeStates array is empty.
  /// \return True if there are zero nodes, otherwise false.
  bool is_node_states_empty() const;

  /// \brief Check if any actionStates are still executing (not FINISHED or FAILED).
  /// \return True if at least one action is still executing, otherwise false.
  bool are_action_states_still_executing() const;

  /// \brief Clear all state related to the currently stored order.
  void cleanup_previous_order();

  /// \brief Set a new order on the vehicle (after clearing any existing order).
  /// \param order The new order to accept and store.
  void set_new_order(const Order& order);

  /// \brief Set a new order on the vehicle (after clearing any existing order).
  /// \param order The new order to accept and store.
  void set_new_order(const vda5050_core::order::Order& order);

  /// \brief Clear the horizon nodes/edges from the current nodeStates and edgeStates.
  void clear_horizon();

  /// \brief Append an order update to the vehicle's current order (nodeStates/edgeStates).
  /// \param order_update The order update to append.
  void append_states_for_update(Order& order_update);

  /// \brief Append an order update to the vehicle's current order (nodeStates/edgeStates).
  /// \param order_update The order update to append.
  void append_states_for_update(vda5050_core::order::Order& order_update);
};

}  // namespace state_manager
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATUS_MANAGER__STATUS_MANAGER_HPP_
