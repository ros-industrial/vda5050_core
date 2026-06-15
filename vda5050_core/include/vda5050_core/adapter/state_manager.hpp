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

#ifndef VDA5050_CORE__ADAPTER__STATE_MANAGER_HPP_
#define VDA5050_CORE__ADAPTER__STATE_MANAGER_HPP_

#include <memory>
#include <mutex>

#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/state.hpp"

namespace vda5050_core {

namespace adapter {

class StateManager : public std::enable_shared_from_this<StateManager>
{
public:
  static std::shared_ptr<StateManager> make();

  void set_driving(bool driving);

  void set_agv_position(const types::AGVPosition& position);

  void set_action_states(const std::vector<types::ActionState>& action_states);

  void set_operating_mode(types::OperatingMode mode);

  void add_error(const types::Error& error);

  void clear_errors();

  types::State state() const;

private:
  StateManager();

  mutable std::mutex mutex_;
  types::State state_;
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__STATE_MANAGER_HPP_
