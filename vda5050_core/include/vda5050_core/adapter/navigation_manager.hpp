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

#include <cstdint>
#include <memory>
#include <string>

#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/provider.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/error.hpp"
#include "vda5050_core/types/operating_mode.hpp"
#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/battery_state.hpp"
#include "vda5050_core/types/node.hpp"
#include <vector>

namespace vda5050_core {

namespace adapter {

struct AgvState;

class NavigationManager
{
public:
  ~NavigationManager();

  /// \brief Robot reports it reached a node — unblocks order execution
  ///
  /// \param node The reached order node (uses node_id and sequence_id)
  void node_reached(const types::Node& node);

  /// \brief State field setters — robot calls these as hardware state changes
  void set_driving(bool driving);

  /// \brief Update AGV position in the published state message
  void set_agv_position(const types::AGVPosition& position);

  /// \brief Update battery information in the published state message
  void set_battery_state(const types::BatteryState& battery_state);

  /// \brief Update action states in the published state message
  void set_action_states(const std::vector<types::ActionState>& action_states);

  /// \brief Update operating mode in the published state message
  void set_operating_mode(types::OperatingMode mode);


  /// \brief Append an error to the published state message
  void add_error(types::Error error);

  /// \brief Clear all errors from the published state message
  void clear_errors();
private:
  friend class Adapter;

  NavigationManager();

  void bind_internals(
    std::shared_ptr<AgvState> agv_state,
    std::shared_ptr<execution::Provider> provider,
    std::weak_ptr<execution::ContextInterface> context);

  class Implementation;
  std::unique_ptr<Implementation> pimpl_;
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__NAVIGATION_MANAGER_HPP_
