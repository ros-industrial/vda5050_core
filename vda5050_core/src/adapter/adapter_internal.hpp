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

#ifndef ADAPTER__ADAPTER_INTERNAL_HPP_
#define ADAPTER__ADAPTER_INTERNAL_HPP_

#include <mutex>
#include <optional>
#include <vector>

#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/battery_state.hpp"
#include "vda5050_core/types/state.hpp"

namespace vda5050_core {

namespace adapter {

/// \brief Robot-owned State fields, written by NavigationManager and overlaid
///        onto the strategy-produced State at publish time.
///
/// These fields have no strategy owner (no strategy sets driving, position, or
/// battery), so keeping them here — rather than in `OrderExecutionResource` —
/// avoids being clobbered by the strategies' full-state `set_state()` writes.
struct RobotIoState
{
  mutable std::mutex mutex;

  bool driving = false;
  std::optional<types::AGVPosition> agv_position;

  types::BatteryState battery_state;
  bool battery_set = false;

  std::vector<types::ActionState> action_states;
  bool action_states_set = false;
};

/// \brief Overlay the robot-owned fields of `io` onto a copy of `order_state`.
inline types::State merge_robot_io(
  const types::State& order_state, const RobotIoState& io)
{
  types::State merged = order_state;

  std::lock_guard<std::mutex> lock(io.mutex);
  merged.driving = io.driving;
  if (io.agv_position) merged.agv_position = io.agv_position;
  if (io.battery_set) merged.battery_state = io.battery_state;
  if (io.action_states_set) merged.action_states = io.action_states;

  return merged;
}

}  // namespace adapter
}  // namespace vda5050_core

#endif  // ADAPTER__ADAPTER_INTERNAL_HPP_
