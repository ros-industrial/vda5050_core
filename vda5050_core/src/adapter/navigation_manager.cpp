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

#include "vda5050_core/adapter/navigation_manager.hpp"

#include <mutex>
#include <utility>

#include "vda5050_core/client/updates/node_reached.hpp"

#include "adapter_internal.hpp"

namespace vda5050_core {

namespace adapter {

class NavigationManager::Implementation
{
public:
  std::shared_ptr<RobotIoState> robot_io;
  std::shared_ptr<execution::Provider> provider;
};

NavigationManager::NavigationManager()
: pimpl_(std::make_unique<Implementation>())
{
  // Nothing to do here ...
}

NavigationManager::~NavigationManager() = default;

void NavigationManager::bind_internals(
  std::shared_ptr<RobotIoState> robot_io,
  std::shared_ptr<execution::Provider> provider)
{
  pimpl_->robot_io = std::move(robot_io);
  pimpl_->provider = std::move(provider);
}

void NavigationManager::node_reached(const types::Node& node)
{
  if (!pimpl_->provider) return;

  // Pushing the update fires AGVContext's registered NodeReachedUpdate callback,
  // which caches it and wakes the Handler — OrderTraversal then advances the
  // order. No manual notify needed here.
  pimpl_->provider->push<client::NodeReachedUpdate>(
    node.node_id, node.sequence_id);
}

void NavigationManager::set_driving(bool driving)
{
  if (!pimpl_->robot_io) return;

  std::lock_guard<std::mutex> lock(pimpl_->robot_io->mutex);
  pimpl_->robot_io->driving = driving;
}

void NavigationManager::set_agv_position(const types::AGVPosition& position)
{
  if (!pimpl_->robot_io) return;

  std::lock_guard<std::mutex> lock(pimpl_->robot_io->mutex);
  pimpl_->robot_io->agv_position = position;
}

void NavigationManager::set_battery_state(
  const types::BatteryState& battery_state)
{
  if (!pimpl_->robot_io) return;

  std::lock_guard<std::mutex> lock(pimpl_->robot_io->mutex);
  pimpl_->robot_io->battery_state = battery_state;
  pimpl_->robot_io->battery_set = true;
}

void NavigationManager::set_action_states(
  const std::vector<types::ActionState>& action_states)
{
  if (!pimpl_->robot_io) return;

  std::lock_guard<std::mutex> lock(pimpl_->robot_io->mutex);
  pimpl_->robot_io->action_states = action_states;
  pimpl_->robot_io->action_states_set = true;
}

}  // namespace adapter
}  // namespace vda5050_core
