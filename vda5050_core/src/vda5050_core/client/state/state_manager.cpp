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

#include <algorithm>
#include <mutex>

#include "vda5050_core/client/state/state_manager.hpp"

namespace vda5050_core {

namespace state {

//=============================================================================
void StateManager::set_header(const Header& header)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.header = header;
}

//=============================================================================
Header StateManager::get_header() const
{
  std::unique_lock lock(this->mutex_);
  return this->robot_state_.header;
}

//=============================================================================
void StateManager::set_order_id(const std::string& order_id)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.order_id = order_id;
}

//=============================================================================
std::string StateManager::get_order_id() const
{
  std::shared_lock lock(this->mutex_);

  return this->robot_state_.order_id;
}

//=============================================================================
void StateManager::set_order_update_id(uint32_t order_update_id)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.order_update_id = order_update_id;
}
//=============================================================================
uint32_t StateManager::get_order_update_id() const
{
  std::shared_lock lock(this->mutex_);

  return this->robot_state_.order_update_id;
}

//=============================================================================
void StateManager::set_zone_set_id(const std::string& zone_set_id)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.zone_set_id = zone_set_id;
}

//=============================================================================
std::optional<std::string> StateManager::get_zone_set_id() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.zone_set_id;
}

//=============================================================================
void StateManager::set_last_node_id(const std::string& last_node_id)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.last_node_id = last_node_id;
}

//=============================================================================
std::string StateManager::get_last_node_id() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.last_node_id;
}

//=============================================================================
void StateManager::set_last_node_sequence_id(uint32_t last_node_sequence_id)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.last_node_sequence_id = last_node_sequence_id;
}

//=============================================================================
uint32_t StateManager::get_last_node_sequence_id() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.last_node_sequence_id;
}

//=============================================================================
void StateManager::set_agv_position(const AGVPosition& agv_position)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.agv_position = agv_position;
}

//=============================================================================
std::optional<AGVPosition> StateManager::get_agv_position() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.agv_position;
}

//=============================================================================
void StateManager::set_velocity(const Velocity& velocity)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.velocity = velocity;
}

//=============================================================================
std::optional<Velocity> StateManager::get_velocity() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.velocity;
}

//=============================================================================
void StateManager::set_driving_status(bool driving)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.driving = driving;
}

//=============================================================================
bool StateManager::get_driving_status() const
{
  std::unique_lock lock(this->mutex_);
  return this->robot_state_.driving;
}

//=============================================================================
void StateManager::set_distance_since_last_node(double distance_since_last_node)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.distance_since_last_node = distance_since_last_node;
}

//=============================================================================
void StateManager::reset_distance_since_last_node()
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.distance_since_last_node.reset();
}

//=============================================================================
std::optional<double> StateManager::get_distance_since_last_node() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.distance_since_last_node;
}

//=============================================================================
void StateManager::add_load(const Load& load)
{
  std::unique_lock lock(this->mutex_);

  if (!this->robot_state_.loads.has_value())
  {
    this->robot_state_.loads = {load};
  }
  else
  {
    this->robot_state_.loads->push_back(load);
  }
}

//=============================================================================
void StateManager::set_load(const std::vector<Load>& loads)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.loads = loads;
}

//=============================================================================
bool StateManager::remove_load(std::string_view load_id)
{
  std::unique_lock lock(this->mutex_);

  if (!this->robot_state_.loads.has_value())
  {
    return false;
  }

  auto match_id = [load_id](const Load& load) {
    return load.load_id == load_id;
  };

  auto before_size = this->robot_state_.loads->size();

  this->robot_state_.loads->erase(
    std::remove_if(
      this->robot_state_.loads->begin(), this->robot_state_.loads->end(),
      match_id),
    this->robot_state_.loads->end());

  return this->robot_state_.loads->size() != before_size;
}

//=============================================================================
std::optional<std::vector<Load>> StateManager::get_loads() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.loads;
}

//=============================================================================
void StateManager::set_operating_mode(OperatingMode operating_mode)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.operating_mode = operating_mode;
}

//=============================================================================
OperatingMode StateManager::get_operating_mode() const
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that mode is not being altered at the moment
  return this->robot_state_.operating_mode;
}

//=============================================================================
void StateManager::set_battery_state(const BatteryState& battery_state)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.battery_state = battery_state;
}

//=============================================================================
BatteryState StateManager::get_battery_state() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.battery_state;
}

//=============================================================================
void StateManager::set_safety_state(const SafetyState& safety_state)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.safety_state = safety_state;
}

//=============================================================================
SafetyState StateManager::get_safety_state() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.safety_state;
}

void StateManager::set_request_new_base(bool request_new_base)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.new_base_request = request_new_base;
}

//=============================================================================
void StateManager::add_error(const Error& error)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.errors.push_back(error);
}

//=============================================================================
std::vector<Error> StateManager::get_errors() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.errors;
}

//=============================================================================
void StateManager::clear_errors()
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.errors.clear();
}

//=============================================================================
void StateManager::add_info(const Info& info)
{
  std::unique_lock lock(this->mutex_);

  if (!this->robot_state_.information.has_value())
  {
    this->robot_state_.information = std::vector<Info>{};
  }

  this->robot_state_.information->push_back(info);
}

//=============================================================================
std::optional<std::vector<Info>> StateManager::get_info() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.information;
}

//=============================================================================
void StateManager::clear_info()
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.information.reset();
}

//=============================================================================
void StateManager::set_action_states(std::vector<ActionState> action_states)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.action_states = action_states;
}

//=============================================================================
std::vector<ActionState> StateManager::get_action_states() const
{
  std::unique_lock lock(this->mutex_);
  return this->robot_state_.action_states;
}

//=============================================================================
void StateManager::dump_to(State& state)
{
  std::shared_lock lock(this->mutex_);

  state.header = this->robot_state_.header;
  state.order_id = this->robot_state_.order_id;
  state.order_update_id = this->robot_state_.order_update_id;
  state.zone_set_id = this->robot_state_.zone_set_id;
  state.last_node_id = this->robot_state_.last_node_id;
  state.last_node_sequence_id = this->robot_state_.last_node_sequence_id;
  state.node_states = this->robot_state_.node_states;
  state.edge_states = this->robot_state_.edge_states;
  state.agv_position = this->robot_state_.agv_position;
  state.velocity = this->robot_state_.velocity;
  state.loads = this->robot_state_.loads;
  state.driving = this->robot_state_.driving;
  state.paused = this->robot_state_.paused;
  state.new_base_request = this->robot_state_.new_base_request;
  state.distance_since_last_node = this->robot_state_.distance_since_last_node;
  state.action_states = this->robot_state_.action_states;
  state.battery_state = this->robot_state_.battery_state;
  state.operating_mode = this->robot_state_.operating_mode;
  state.errors = this->robot_state_.errors;
  state.information = this->robot_state_.information;
  state.safety_state = this->robot_state_.safety_state;
}

//=============================================================================
bool StateManager::is_node_states_empty() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.node_states.empty();
}

//=============================================================================
bool StateManager::are_action_states_still_executing() const
{
  std::shared_lock lock(this->mutex_);

  if (this->robot_state_.action_states.empty()) return false;

  for (const auto& action_state : this->robot_state_.action_states)
  {
    if (
      action_state.action_status != ActionStatus::FINISHED &&
      action_state.action_status != ActionStatus::FAILED)
    {
      return false;
    }
  }
  return true;
}

//=============================================================================
void StateManager::cleanup_previous_order()

{
  std::unique_lock lock(this->mutex_);
  this->cleanup();
}

//=============================================================================
void StateManager::set_new_order(const Order& order)
{
  std::unique_lock lock(this->mutex_);

  this->cleanup();

  this->robot_state_.order_id = order.order_id;
  this->robot_state_.order_update_id = order.order_update_id;
  this->robot_state_.zone_set_id = order.zone_set_id;

  for (const auto& node : order.nodes)
  {
    NodeState node_state;
    node_state.node_id = node.node_id;
    node_state.sequence_id = node.sequence_id;
    node_state.node_description = node.node_description;
    node_state.node_position = node.node_position;
    node_state.released = node.released;
    this->robot_state_.node_states.push_back(node_state);
  }

  for (const auto& edge : order.edges)
  {
    EdgeState edge_state;
    edge_state.edge_id = edge.edge_id;
    edge_state.sequence_id = edge.sequence_id;
    edge_state.edge_description = edge.edge_description;
    edge_state.trajectory = edge.trajectory;
    edge_state.released = edge.released;
    this->robot_state_.edge_states.push_back(edge_state);
  }
}

//=============================================================================
void StateManager::clear_horizon()
{
  std::unique_lock lock(this->mutex_);
  auto& nodes = this->robot_state_.node_states;
  auto& edges = this->robot_state_.edge_states;

  auto node_predicate = [](const NodeState& n) { return !n.released; };
  auto edge_predicate = [](const EdgeState& e) { return !e.released; };

  nodes.erase(
    std::remove_if(nodes.begin(), nodes.end(), node_predicate), nodes.end());
  edges.erase(
    std::remove_if(edges.begin(), edges.end(), edge_predicate), edges.end());
}

State StateManager::get_state()
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_;
}

void StateManager::cleanup()
{
  Header prev_header = this->robot_state_.header;
  std::string last_node_id = this->robot_state_.last_node_id;
  std::optional<AGVPosition> prev_agv_position =
    this->robot_state_.agv_position;
  std::optional<Velocity> prev_velocity = this->robot_state_.velocity;
  bool prev_driving = this->robot_state_.driving;
  std::optional<bool> prev_paused = this->robot_state_.paused;
  BatteryState prev_battery_state = this->robot_state_.battery_state;
  OperatingMode prev_operating_mode = this->robot_state_.operating_mode;
  SafetyState prev_safety_state = this->robot_state_.safety_state;

  this->robot_state_ = State();
  this->robot_state_.header = prev_header;
  this->robot_state_.last_node_id = last_node_id;
  this->robot_state_.agv_position = prev_agv_position;
  this->robot_state_.velocity = prev_velocity;
  this->robot_state_.driving = prev_driving;
  this->robot_state_.paused = prev_paused;
  this->robot_state_.battery_state = prev_battery_state;
  this->robot_state_.operating_mode = prev_operating_mode;
  this->robot_state_.safety_state = prev_safety_state;
}

}  // namespace state
}  // namespace vda5050_core
