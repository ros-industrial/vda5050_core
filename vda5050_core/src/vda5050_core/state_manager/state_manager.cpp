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

#include "vda5050_core/state_manager/state_manager.hpp"

namespace vda5050_core {

namespace state_manager {

//=============================================================================
std::optional<std::string> StateManager::get_order_id() const
{
  std::shared_lock lock(this->mutex_);
  if (this->robot_state_.order_id.empty()) return std::nullopt;

  return this->robot_state_.order_id;
}

//=============================================================================
std::optional<uint32_t> StateManager::get_order_update_id() const
{
  std::shared_lock lock(this->mutex_);
  if (this->robot_state_.order_id.empty()) return std::nullopt;

  return this->robot_state_.order_update_id;
}

//=============================================================================
std::optional<std::string> StateManager::get_zone_set_id() const
{
  std::shared_lock lock(this->mutex_);
  if (this->robot_state_.order_id.empty()) return std::nullopt;

  return this->robot_state_.zone_set_id;
}

//=============================================================================
void StateManager::set_agv_position(
  const std::optional<AGVPosition>& agv_position)
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
void StateManager::set_velocity(const std::optional<Velocity>& velocity)
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
bool StateManager::set_driving(bool driving)
{
  std::unique_lock lock(this->mutex_);
  bool changed = this->robot_state_.driving != driving;
  this->robot_state_.driving = driving;
  return changed;
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
bool StateManager::add_load(const Load& load)
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

  return true;
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
const std::vector<Load>& StateManager::get_loads()
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that loads is not being altered at the moment

  static const std::vector<Load> empty;

  // value_or turns empty into a stack object, so this if block is required
  if (this->robot_state_.loads.has_value())
  {
    return *this->robot_state_.loads;
  }
  else
  {
    return empty;
  }
}

//=============================================================================
bool StateManager::set_operating_mode(OperatingMode operating_mode)
{
  std::unique_lock lock(this->mutex_);
  bool changed = this->robot_state_.operating_mode != operating_mode;
  this->robot_state_.operating_mode = operating_mode;
  return changed;
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
const BatteryState& StateManager::get_battery_state() const
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that battery is not being altered at the moment
  return this->robot_state_.battery_state;
}

//=============================================================================
bool StateManager::set_safety_state(const SafetyState& safety_state)
{
  std::unique_lock lock(this->mutex_);
  auto before = this->robot_state_.safety_state;
  this->robot_state_.safety_state = safety_state;
  return before != (safety_state);
}

//=============================================================================
const SafetyState& StateManager::get_safety_state() const
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that safety is not being altered at the moment
  return this->robot_state_.safety_state;
}

void StateManager::request_new_base()
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.new_base_request = true;
}

//=============================================================================
bool StateManager::add_error(const Error& error)
{
  std::unique_lock lock(this->mutex_);
  this->robot_state_.errors.push_back(error);
  return true;
}

//=============================================================================
std::vector<Error> StateManager::get_errors() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.errors;
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
std::string StateManager::get_last_node_id() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.last_node_id;
}

//=============================================================================
uint32_t StateManager::get_last_node_sequence_id() const
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_.last_node_sequence_id;
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

  if (this->robot_state_.action_states.empty()) return true;

  for (const auto& action_state : this->robot_state_.action_states)
  {
    if (
      action_state.action_status != ActionStatus::FINISHED &&
      action_state.action_status != ActionStatus::FAILED)
    {
      return true;
    }
  }
  return false;
}

//=============================================================================
void StateManager::cleanup_previous_order()
{
  std::unique_lock lock(this->mutex_);
  std::string last_node_id = this->robot_state_.last_node_id;
  this->robot_state_ = State();
  this->robot_state_.last_node_id = last_node_id;
}

//=============================================================================
void StateManager::set_new_order(const Order& order)
{
  std::unique_lock lock(this->mutex_);

  // Temp fix to avoid deadlock if cleanup_previous_order is called
  std::string last_node_id = this->robot_state_.last_node_id;
  this->robot_state_ = State();
  this->robot_state_.last_node_id = last_node_id;
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

void StateManager::append_states_for_update(Order& order_update)
{
  this->set_new_order(order_update);
}

const State& StateManager::get_state()
{
  std::shared_lock lock(this->mutex_);
  return this->robot_state_;
}

}  // namespace state_manager
}  // namespace vda5050_core
