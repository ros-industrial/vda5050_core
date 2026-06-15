/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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

#include "vda5050_core/adapter/state_manager.hpp"

namespace vda5050_core {

namespace adapter {

//=============================================================================
std::shared_ptr<StateManager> StateManager::make()
{
  auto manager = std::shared_ptr<StateManager>(new StateManager());
  return manager;
}

//=============================================================================
void StateManager::set_driving(bool driving)
{
  std::lock_guard<std::mutex> lock(mutex_);
  state_.driving = driving;
  publish_requested_ = true;
}

//=============================================================================
void StateManager::set_agv_position(const types::AGVPosition& position)
{
  std::lock_guard<std::mutex> lock(mutex_);
  state_.agv_position = position;
}

//=============================================================================
void StateManager::set_battery_state(const types::BatteryState& battery)
{
  std::lock_guard<std::mutex> lock(mutex_);
  state_.battery_state = battery;
}

//=============================================================================
void StateManager::set_action_states(
  const std::vector<types::ActionState>& action_states)
{
}

//=============================================================================
void StateManager::set_operating_mode(types::OperatingMode mode)
{
  std::lock_guard<std::mutex> lock(mutex_);
  state_.operating_mode = mode;
}

//=============================================================================
void StateManager::add_error(const types::Error& error)
{
  std::lock_guard<std::mutex> lock(mutex_);
  state_.errors.push_back(error);
  publish_requested_ = true;
}

//=============================================================================
void StateManager::clear_errors()
{
  std::lock_guard<std::mutex> lock(mutex_);
  state_.errors.clear();
}

//=============================================================================
types::State state() {}

//=============================================================================
void StateManager::mark_publish_requested()
{
  publish_requested_ = true;
}

//=============================================================================
bool StateManager::consume_publish_requested()
{
  publish_requested_.exchange(false);
}

//=============================================================================
StateManager::StateManager() : publish_requested_(false)
{
  // Nothing to do here ...
}

}  // namespace adapter
}  // namespace vda5050_core
