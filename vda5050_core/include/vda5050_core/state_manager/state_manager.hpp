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

#ifndef VDA5050_CORE__STATE_MANAGER__STATE_MANAGER_HPP_
#define VDA5050_CORE__STATE_MANAGER__STATE_MANAGER_HPP_

#include <cstdint>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

#include "vda5050_types/action_state.hpp"
#include "vda5050_types/agv_position.hpp"
#include "vda5050_types/battery_state.hpp"
#include "vda5050_types/error.hpp"
#include "vda5050_types/header.hpp"
#include "vda5050_types/info.hpp"
#include "vda5050_types/load.hpp"
#include "vda5050_types/operating_mode.hpp"
#include "vda5050_types/order.hpp"
#include "vda5050_types/safety_state.hpp"
#include "vda5050_types/state.hpp"

namespace vda5050_core {

namespace state_manager {

using vda5050_types::ActionState;
using vda5050_types::ActionStatus;
using vda5050_types::AGVPosition;
using vda5050_types::BatteryState;
using vda5050_types::EdgeState;
using vda5050_types::Error;
using vda5050_types::Header;
using vda5050_types::Info;
using vda5050_types::Load;
using vda5050_types::NodeState;
using vda5050_types::OperatingMode;
using vda5050_types::Order;
using vda5050_types::SafetyState;
using vda5050_types::State;
using vda5050_types::Velocity;

class StateManager
{
private:
  /// \brief the mutex protecting the data
  mutable std::shared_mutex mutex_;

  /// \brief Internal State of the AGV
  State robot_state_;

  /// \brief Clear current robot order state
  void cleanup();

public:
  /// \brief set the AGV state header
  /// \param header of the the AGV
  void set_header(const Header& header);

  /// \brief get the AGV State header
  /// \return header of the AGV
  Header get_header() const;

  /// \brief set the current order_id
  /// \param order_id of the current order
  void set_order_id(const std::string& order_id);

  /// \brief get the current order_id
  /// \return order_id of the current order
  std::string get_order_id() const;

  /// \brief set the current order_update_id
  /// \param order_update_id the current order update_id to set
  void set_order_update_id(uint32_t order_update_id);

  /// \brief get the current order_update_id
  /// \return order_update_id of the current order
  uint32_t get_order_update_id() const;

  /// \brief set the current zone_set_id
  /// \param zone_set_id zone_set_id the current order to set
  void set_zone_set_id(const std::string& zone_set_id);

  /// \brief get the current zone_set_id
  /// \return zone_set_id of the current order
  std::optional<std::string> get_zone_set_id() const;

  /// \brief Get the nodeId of the latest node the AGV has reached.
  /// \param last_node_id last reached node's ID.
  void set_last_node_id(const std::string& last_node_id);

  /// \brief Get the nodeId of the latest node the AGV has reached.
  /// \return The last reached node's ID.
  std::string get_last_node_id() const;

  /// \brief Get the sequenceId of the latest node the AGV has reached.
  /// \param last_node_sequence_id last reached node's sequence ID.
  void set_last_node_sequence_id(uint32_t last_node_sequence_id);

  /// \brief Get the sequenceId of the latest node the AGV has reached.
  /// \return The last reached node's sequence ID.
  uint32_t get_last_node_sequence_id() const;

  /// \brief Set the current AGV position
  /// \param agv_position the agv position
  void set_agv_position(const AGVPosition& agv_position);

  /// \brief Get the current AGV position (if set)
  /// \return std::optional<AGVPosition> the current AGV position of std::nullopt
  std::optional<AGVPosition> get_agv_position() const;

  /// \brief Set the current velocity
  /// \param velocity the velocity
  void set_velocity(const Velocity& velocity);

  /// \brief Get the Velocity (if set)
  /// \return std::optional<Velocity> the velocity of std::nullopt
  std::optional<Velocity> get_velocity() const;

  /// \brief Set the driving flag of the AGV
  /// \param driving is the agv driving?
  void set_driving_status(bool driving);

  /// \brief Set the driving flag of the AGV
  /// \return is the agv driving?
  bool get_driving_status() const;

  /// \brief Set the distance since the last node as in vda5050
  /// \param distance_since_last_node the new distance since the last node
  void set_distance_since_last_node(double distance_since_last_node);

  /// \brief Reset the distance since the last node
  void reset_distance_since_last_node();

  /// \brief Get the current distance since the last node.
  /// \return The current distance since the last node, or std::nullopt if not set.
  std::optional<double> get_distance_since_last_node() const;

  /// \brief add a load to the state
  /// \param load the load to add
  void add_load(const Load& load);

  /// \brief set a vector of load to the state
  /// \param load the vector of load to add
  void set_load(const std::vector<Load>& loads);

  /// \brief Remove a load by it's id from the loads array
  /// \param load_id the id of the load to remove
  /// \return true if at least one load was removed
  bool remove_load(std::string_view load_id);

  /// \brief Get the current loads
  /// \return const std::vector<Load>& the current loads
  std::optional<std::vector<Load>> get_loads() const;

  /// \brief Set the current operating mode of the AGV
  /// \param operating_mode the new operating mode
  /// \return true if the operating mode changed
  void set_operating_mode(OperatingMode operating_mode);

  /// \brief Get the current operating mode from the state
  /// \return OperatingMode the current operating mode
  OperatingMode get_operating_mode() const;

  /// \brief Set the current battery state of the AGV
  /// \param battery_state the battery state
  void set_battery_state(const BatteryState& battery_state);

  /// \brief Get the current battery state from the state
  /// \return const BatteryState& the current battery state
  BatteryState get_battery_state() const;

  /// \brief Set the current safety state of the AGV
  /// \param safety_state the safety state
  /// \return true if the state changed
  void set_safety_state(const SafetyState& safety_state);

  /// \brief Get the current safety state from the state
  /// \return const SafetyState& the current safety state
  SafetyState get_safety_state() const;

  /// \brief Set the request new base flag
  /// \param request_new_base is AGV requesting for new base
  void set_request_new_base(bool request_new_base);

  /// \brief Add an error to the state
  /// \param error the error to add
  void add_error(const Error& error);

  /// \brief Get a copy of the current errors
  /// \return std::vector<Error>
  std::vector<Error> get_errors() const;

  /// \brief Clear list of errors
  void clear_errors();

  /// \brief Add a new information to the state
  /// \param info the information to add
  void add_info(const Info& info);

  /// \brief Get the list of information to the state
  /// \return AGV State list of information
  std::optional<std::vector<Info>> get_info() const;

  /// \brief Clear the list of information to the state
  void clear_info();

  /// \brief Set the AGV action status
  void set_action_states(std::vector<ActionState> action_states);

  /// \brief Get the AGV action status
  std::vector<ActionState> get_action_states() const;

  /// \brief Dump all data to a vda5050::State
  /// \param state the state to write to
  void dump_to(State& state);

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

  /// \brief Clear the horizon nodes/edges from the current nodeStates and edgeStates.
  void clear_horizon();

  /// @brief Get the current robot state.
  /// @return const State& the current robot state.
  State get_state();
};

}  // namespace state_manager
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE_MANAGER__STATE_MANAGER_HPP_
