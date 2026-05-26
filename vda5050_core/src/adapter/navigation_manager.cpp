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

#include <utility>

#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/provider.hpp"

#include "adapter_internal.hpp"

namespace vda5050_core {

namespace adapter {

class NavigationManager::Implementation
{
public:
  std::shared_ptr<AgvState> agv_state;
  std::shared_ptr<execution::Provider> provider;
  std::weak_ptr<execution::ContextInterface> context;
};

NavigationManager::NavigationManager() : pimpl_(std::make_unique<Implementation>())
{
  // Nothing to do here ...
}

NavigationManager::~NavigationManager() = default;

void NavigationManager::bind_internals(
  std::shared_ptr<AgvState> agv_state,
  std::shared_ptr<execution::Provider> provider,
  std::weak_ptr<execution::ContextInterface> context)
{
  pimpl_->agv_state = std::move(agv_state);
  pimpl_->provider = std::move(provider);
  pimpl_->context = std::move(context);
}

void NavigationManager::node_reached(const types::Node& node)
{
  if (!pimpl_->agv_state || !pimpl_->provider) return;

  // Keep a local copy before updating shared state. Reading last_node_id after
  // the lock is released would race with any concurrent node_reached() call.
  const std::string node_id_copy = node.node_id;
  const uint32_t sequence_id = node.sequence_id;

  {
    std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
    pimpl_->agv_state->state.last_node_id = node_id_copy;
    pimpl_->agv_state->state.last_node_sequence_id = sequence_id;
    pimpl_->agv_state->state.driving = false;
  }

  pimpl_->provider->push<NodeAckUpdate>(node_id_copy, sequence_id);

  if (auto context = pimpl_->context.lock())
  {
    context->notify_on_change();
  }
}

void NavigationManager::set_driving(bool driving)
{
  if (!pimpl_->agv_state) return;

  std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
  pimpl_->agv_state->state.driving = driving;
}

void NavigationManager::set_agv_position(const types::AGVPosition& position)
{
  if (!pimpl_->agv_state) return;

  std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
  pimpl_->agv_state->state.agv_position = position;
}

void NavigationManager::set_battery_state(const types::BatteryState& battery_state)
{
  if (!pimpl_->agv_state) return;

  std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
  pimpl_->agv_state->state.battery_state = battery_state;
}

void NavigationManager::set_operating_mode(types::OperatingMode mode)
{
  if (!pimpl_->agv_state) return;

  std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
  pimpl_->agv_state->state.operating_mode = mode;
}

void NavigationManager::add_error(types::Error error)
{
  if (!pimpl_->agv_state) return;

  std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
  pimpl_->agv_state->state.errors.push_back(std::move(error));
}

void NavigationManager::clear_errors()
{
  if (!pimpl_->agv_state) return;

  std::lock_guard<std::mutex> lock(pimpl_->agv_state->mutex);
  pimpl_->agv_state->state.errors.clear();
}

}  // namespace adapter
}  // namespace vda5050_core
