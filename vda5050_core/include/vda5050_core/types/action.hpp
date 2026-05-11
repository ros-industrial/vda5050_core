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

#ifndef VDA5050_CORE__TYPES__ACTION_HPP_
#define VDA5050_CORE__TYPES__ACTION_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/action_parameter.hpp"
#include "vda5050_core/types/blocking_type.hpp"

namespace vda5050_core {

namespace types {

struct Action
{
  /// \brief Name of action as described in the first column of Actions and
  /// Parameters. Identifies the function of the action
  std::string action_type;

  /// \brief Unique ID to identify the action and map it to the actionState in
  /// the State message
  std::string action_id;

  /// \brief  Regulates if the action is allowed to be executed during movement
  /// and/or parallel to other actions.
  BlockingType blocking_type;

  /// \brief Additional information on the action
  std::optional<std::string> action_description;

  /// \brief Array of actionParameter objects for the indicated action. If not
  /// defined, the action has no parameters
  /// eg: deviceId, loadId, external Triggers
  std::optional<std::vector<ActionParameter>> action_parameters;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Action& other) const
  {
    if (this->action_type != other.action_type) return false;
    if (this->action_id != other.action_id) return false;

    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator!=(const Action& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ACTION_HPP_
