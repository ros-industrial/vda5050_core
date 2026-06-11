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

#ifndef VDA5050_CORE__ADAPTER__NAVIGATION_MANAGER_HPP_
#define VDA5050_CORE__ADAPTER__NAVIGATION_MANAGER_HPP_

#include <memory>
#include <vector>

#include "vda5050_core/execution/provider.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/battery_state.hpp"
#include "vda5050_core/types/node.hpp"

namespace vda5050_core {

namespace adapter {

// Robot-owned State fields, shared with the Adapter for merge-at-publish.
struct RobotIoState;

/// \brief Robot-side handle for reporting navigation completion and live state.
///
/// Obtained from `Adapter::navigation_manager()`. The robot integration calls
/// these as hardware state changes:
///   - `node_reached` pushes a `NodeReachedUpdate` into the execution provider,
///     which `OrderTraversal` consumes to advance the order (drop the reached
///     node/edge and dispatch the next `NavigateToNodeEvent`).
///   - the setters update `RobotIoState`; the Adapter overlays these onto the
///     strategy-produced State just before publishing, so they never collide
///     with the strategies' full-state writes to `OrderExecutionResource`.
class NavigationManager
{
public:
  ~NavigationManager();

  /// \brief Signal that the robot reached `node` (uses node_id + sequence_id).
  void node_reached(const types::Node& node);

  /// \brief Update the published State's `driving` flag.
  void set_driving(bool driving);

  /// \brief Update the published State's `agvPosition`.
  void set_agv_position(const types::AGVPosition& position);

  /// \brief Update the published State's `batteryState`.
  void set_battery_state(const types::BatteryState& battery_state);

  /// \brief Update the published State's `actionStates` array.
  void set_action_states(const std::vector<types::ActionState>& action_states);

private:
  friend class Adapter;

  NavigationManager();

  /// \brief Wire the shared robot-state buffer and the execution provider.
  void bind_internals(
    std::shared_ptr<RobotIoState> robot_io,
    std::shared_ptr<execution::Provider> provider);

  class Implementation;
  std::unique_ptr<Implementation> pimpl_;
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__NAVIGATION_MANAGER_HPP_
