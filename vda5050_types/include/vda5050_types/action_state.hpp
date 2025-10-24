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

#ifndef VDA5050_TYPES__ACTION_STATE_HPP_
#define VDA5050_TYPES__ACTION_STATE_HPP_

#include <optional>
#include <string>

#include "vda5050_types/action_status.hpp"

namespace vda5050_types {

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
  ActionStatus action_status = ActionStatus::WAITING;

  /// @brief Description of the result, e.g., the
  ///        result of an RFID reading.
  ///        Errors will be transmitted in errors.
  std::optional<std::string> result_description;
  ///
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__ACTION_STATE_HPP_
