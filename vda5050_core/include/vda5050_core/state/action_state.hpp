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

#ifndef VDA5050_CORE__STATE__ACTION_STATE_HPP_
#define VDA5050_CORE__STATE__ACTION_STATE_HPP_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "vda5050_core/state/action_status.hpp"

namespace vda5050_core {

namespace msg {

/// @struct ActionState
/// @brief Contains an array of all actions from
///        the current order and all received
///        instantActions since the last order.
///        The action states are kept until a new
///        order is received. Action states,
///        except for running instant actions,
///        are removed upon receiving a new
///        order.
///        This may include actions from
///        previous nodes, that are still in
///        progress.
///
///        When an action is completed, an
///        updated state message is published
///        with actionStatus set to 'FINISHED'
///        and if applicable with the
///        corresponding
///        resultDescription.
struct ActionState
{
  /// @brief Unique identifier of the action.
  std::string action_id;

  /// @brief Unique identifier of the action.
  ///        Type of the action.
  ///        Optional: Only for informational or
  ///        visualization purposes. MC is aware
  ///        of action type as dispatched in the order.
  std::optional<std::string> action_type;

  /// @brief Additional information on the current action.
  std::optional<std::string> action_description;

  /// @brief The current state of the action
  ///
  /// @note Enum {'WAITING', 'INITIALIZING',
  ///       'RUNNING', 'PAUSED', 'FINISHED',
  ///       'FAILED'}
  ActionStatus action_status = vda5050_core::msg::ActionStatus::WAITING;

  /// @brief Description of the result, e.g., the
  ///        result of an RFID reading.
  ///        Errors will be transmitted in errors.
  std::optional<std::string> result_description;
  ///
};

using json = nlohmann::json;

inline void to_json(json& j, const ActionState& state)
{
  j =
    json{{"actionId", state.action_id}, {"actionStatus", state.action_status}};

  if (state.action_type.has_value())
  {
    j["actionType"] = state.action_type.value();
  }

  if (state.action_description.has_value())
  {
    j["actionDescription"] = state.action_description.value();
  }

  if (state.result_description.has_value())
  {
    j["resultDescription"] = state.result_description.value();
  }
}

inline void from_json(const json& j, ActionState& state)
{
  j.at("actionId").get_to(state.action_id);
  j.at("actionStatus").get_to(state.action_status);

  if (j.contains("actionType") && !j.at("actionType").is_null())
  {
    state.action_type = j.at("actionType").get<std::string>();
  }
  else
  {
    state.action_type.reset();
  }

  if (j.contains("actionDescription") && !j.at("actionDescription").is_null())
  {
    state.action_description = j.at("actionDescription").get<std::string>();
  }
  else
  {
    state.action_description.reset();
  }

  if (j.contains("resultDescription") && !j.at("resultDescription").is_null())
  {
    state.result_description = j.at("resultDescription").get<std::string>();
  }
  else
  {
    state.result_description.reset();
  }
}

}  // namespace msg

}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__ACTION_STATE_HPP_