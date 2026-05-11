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

#ifndef VDA5050_CORE__TYPES__AGV_ACTION_HPP_
#define VDA5050_CORE__TYPES__AGV_ACTION_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/action_parameter_factsheet.hpp"
#include "vda5050_core/types/action_scope.hpp"
#include "vda5050_core/types/blocking_type.hpp"

namespace vda5050_core {

namespace types {

/// \brief Actions with parameters supported by the AGV.
/// This includes standard actions specified in VDA5050 and
/// manufacturer-specific actions.
struct AGVAction
{
  /// \brief Unique type of action corresponding to action.actionType
  std::string action_type;

  /// \brief Array of allowed scopes for using this action type.
  std::vector<ActionScope> action_scopes;

  /// \brief Array of parameters an action has. If not defined, the
  /// action has no parameters.
  std::optional<std::vector<ActionParameterFactsheet>> action_parameters;

  /// \brief Free-form text: description of the result.
  std::optional<std::string> result_description;

  /// \brief Free-form text: description of the action.
  std::optional<std::string> action_description;

  /// \brief Array of possible blocking types for defined action.
  std::optional<std::vector<BlockingType>> blocking_types;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const AGVAction& other) const
  {
    if (this->action_type != other.action_type) return false;
    if (this->action_scopes != other.action_scopes) return false;
    if (this->action_parameters != other.action_parameters) return false;
    if (this->result_description != other.result_description) return false;
    if (this->action_description != other.action_description) return false;
    if (this->blocking_types != other.blocking_types) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const AGVAction& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__AGV_ACTION_HPP_
