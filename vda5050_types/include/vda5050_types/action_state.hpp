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

/// \brief Describes the state of the action carried out by the AGV
/// Part of the state message
struct ActionState
{
  /// \brief Unique identifier of the action
  std::string action_id;

  /// \brief The type of this action. Only for visualization
  /// purposes as the order already knows the type of the action
  std::optional<std::string> action_type;

  /// \brief Additional information on the current action.
  std::optional<std::string> action_description;

  /// \brief The current progress of the action
  ActionStatus action_status = ActionStatus::WAITING;

  /// \brief Description of the result. Errors will be transmitted
  /// in the errors field
  std::optional<std::string> result_description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const ActionState& other) const
  {
    if (this->action_id != other.action_id) return false;
    if (this->action_type != other.action_type) return false;
    if (this->action_description != other.action_description) return false;
    if (this->action_status != other.action_status) return false;
    if (this->result_description != other.result_description) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const ActionState& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__ACTION_STATE_HPP_
