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

void StateManager::set_agv_position(
  const std::optional<AgvPosition>& agv_position)
{
  std::unique_lock lock(this->mutex_);
  this->agv_position_ = agv_position;
}

std::optional<AgvPosition> StateManager::get_agv_position()
{
  std::shared_lock lock(this->mutex_);
  return this->agv_position_;
}

void StateManager::set_velocity(const std::optional<Velocity>& velocity)
{
  std::unique_lock lock(this->mutex_);
  this->velocity_ = velocity;
}

std::optional<Velocity> StateManager::get_velocity() const
{
  std::shared_lock lock(this->mutex_);
  return this->velocity_;
}

bool StateManager::set_driving(bool driving)
{
  std::unique_lock lock(this->mutex_);
  bool changed = this->driving_ != driving;
  this->driving_ = driving;
  return changed;
}

void StateManager::set_distance_since_last_node(double distance_since_last_node)
{
  std::unique_lock lock(this->mutex_);
  this->distance_since_last_node_ = distance_since_last_node;
}

void StateManager::reset_distance_since_last_node()
{
  std::unique_lock lock(this->mutex_);
  this->distance_since_last_node_.reset();
}

bool StateManager::add_load(const Load& load)
{
  std::unique_lock lock(this->mutex_);

  if (!this->loads_.has_value())
  {
    this->loads_ = {load};
  }
  else
  {
    this->loads_->push_back(load);
  }

  return true;
}

bool StateManager::remove_load(std::string_view load_id)
{
  std::unique_lock lock(this->mutex_);

  if (!this->loads_.has_value())
  {
    return false;
  }

  auto match_id = [load_id](const Load& load) {
    return load.load_id == load_id;
  };

  auto before_size = this->loads_->size();

  this->loads_->erase(
    std::remove_if(this->loads_->begin(), this->loads_->end(), match_id),
    this->loads_->end());

  return this->loads_->size() != before_size;
}

const std::vector<Load>& StateManager::get_loads()
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that loads is not being altered at the moment

  const static std::vector<Load> empty;

  // value_or turns empty into a stack object, so this if block is required
  if (this->loads_.has_value())
  {
    return *this->loads_;
  }
  else
  {
    return empty;
  }
}

bool StateManager::set_operating_mode(OperatingMode operating_mode)
{
  std::unique_lock lock(this->mutex_);
  bool changed = this->operating_mode_ != operating_mode;
  this->operating_mode_ = operating_mode;
  return changed;
}

OperatingMode StateManager::get_operating_mode()
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that mode is not being altered at the moment
  return this->operating_mode_;
}

void StateManager::set_battery_state(const BatteryState& battery_state)
{
  std::unique_lock lock(this->mutex_);
  this->battery_state_ = battery_state;
}

const BatteryState& StateManager::get_battery_state()
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that battery is not being altered at the moment
  return this->battery_state_;
}

bool StateManager::set_safety_state(const SafetyState& safety_state)
{
  std::unique_lock lock(this->mutex_);
  auto before = this->safety_state_;
  this->safety_state_ = safety_state;
  return before != (safety_state);
}

const SafetyState& StateManager::get_safety_state()
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that safety is not being altered at the moment
  return this->safety_state_;
}

void StateManager::request_new_base()
{
  std::unique_lock lock(this->mutex_);
  this->new_base_request_ = true;
}

bool StateManager::add_error(const Error& error)
{
  std::unique_lock lock(this->mutex_);
  this->errors_.push_back(error);
  return true;
}

std::vector<Error> StateManager::get_errors() const
{
  std::shared_lock lock(this->mutex_);
  return this->errors_;
}

void StateManager::add_info(const Info& info)
{
  std::unique_lock lock(this->mutex_);
  this->information_.push_back(info);
}

void StateManager::dump_to(State& state)
{
  std::shared_lock lock(this->mutex_);
  state.agv_position = this->agv_position_;
  state.battery_state = this->battery_state_;
  state.distance_since_last_node = this->distance_since_last_node_;
  state.driving = this->driving_;
  state.errors = this->errors_;
  state.information = this->information_;
  state.loads = this->loads_;
  state.new_base_request = this->new_base_request_;
  state.operating_mode = this->operating_mode_;
  state.safety_state = this->safety_state_;
  state.velocity = this->velocity_;
}

}  // namespace state_manager
}  // namespace vda5050_core
