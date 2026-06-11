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

#include "vda5050_core/adapter/reporter.hpp"

#include <mutex>
#include <utility>

#include "vda5050_core/client/updates/node_reached.hpp"

#include "adapter_internal.hpp"

namespace vda5050_core {

namespace adapter {

Reporter::Reporter() = default;

Reporter::~Reporter() = default;

void Reporter::bind_internals(
  std::shared_ptr<RobotIoState> robot_io,
  std::shared_ptr<execution::Provider> provider)
{
  robot_io_ = std::move(robot_io);
  provider_ = std::move(provider);
}

void Reporter::node_reached(const std::string& node_id, uint32_t sequence_id)
{
  if (!provider_) return;

  // Pushing the update fires AGVContext's registered NodeReachedUpdate callback,
  // which caches it and wakes the Handler — OrderTraversal then advances the
  // order. No manual notify needed here.
  provider_->push<client::NodeReachedUpdate>(node_id, sequence_id);
}

void Reporter::set_driving(bool driving)
{
  if (!robot_io_) return;

  std::lock_guard<std::mutex> lock(robot_io_->mutex);
  robot_io_->driving = driving;
}

void Reporter::set_agv_position(const types::AGVPosition& position)
{
  if (!robot_io_) return;

  std::lock_guard<std::mutex> lock(robot_io_->mutex);
  robot_io_->agv_position = position;
}

void Reporter::set_battery_state(const types::BatteryState& battery_state)
{
  if (!robot_io_) return;

  std::lock_guard<std::mutex> lock(robot_io_->mutex);
  robot_io_->battery_state = battery_state;
  robot_io_->battery_set = true;
}

void Reporter::set_action_states(
  const std::vector<types::ActionState>& action_states)
{
  if (!robot_io_) return;

  std::lock_guard<std::mutex> lock(robot_io_->mutex);
  robot_io_->action_states = action_states;
  robot_io_->action_states_set = true;
}

}  // namespace adapter
}  // namespace vda5050_core
