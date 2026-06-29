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

#include "vda5050_core/client/adapter/action_execution.hpp"

namespace vda5050_core {

namespace client {

namespace adapter {

//=============================================================================
std::shared_ptr<ActionExecution> ActionExecution::make(
  const std::string& action_id, const std::string& action_type,
  std::function<void()> finish_callback,
  std::function<void(std::string)> fail_callback,
  std::optional<std::string> order_id)
{
  auto execution = std::shared_ptr<ActionExecution>(new ActionExecution(
    action_id, action_type, std::move(finish_callback),
    std::move(fail_callback), order_id));
  return execution;
}

//=============================================================================
const std::string& ActionExecution::action_id() const
{
  return action_id_;
}

//=============================================================================
const std::string& ActionExecution::action_type() const
{
  return action_id_;
}

//=============================================================================
std::optional<std::string> ActionExecution::order_id()
{
  return order_id_;
}

//=============================================================================
ActionExecution::ActionExecution(
  const std::string& action_id, const std::string& action_type,
  std::function<void()> finish_callback,
  std::function<void(std::string)> fail_callback,
  std::optional<std::string> order_id)
: Execution(std::move(finish_callback), std::move(fail_callback)),
  action_id_(action_id),
  action_type_(action_type),
  order_id_(order_id)
{
  // Nothing to do here ...
}

}  // namespace adapter
}  // namespace client
}  // namespace vda5050_core
