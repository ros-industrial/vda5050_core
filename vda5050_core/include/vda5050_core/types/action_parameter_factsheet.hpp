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

#ifndef VDA5050_CORE__TYPES__ACTION_PARAMETER_FACTSHEET_HPP_
#define VDA5050_CORE__TYPES__ACTION_PARAMETER_FACTSHEET_HPP_

#include <optional>
#include <string>

#include "vda5050_core/types/value_data_type.hpp"

namespace vda5050_core {

namespace types {

/// \brief NOTE: This message is different from ActionParameter.msg used
/// in Order.msg.
/// For more details, refer to page 54 of VDA5050 Recommendation v2.0.0.
/// ActionParameter message specifically used in Factsheet.msg.
/// Describes the array of parameters that an action has.
struct ActionParameterFactsheet
{
  /// \brief Key string for the parameter
  std::string key;

  /// \brief Data type of the value field in Order.ActionParameter
  ValueDataType value_data_type;

  /// \brief Free-form text: description of the parameter
  std::optional<std::string> description;

  /// \brief If set to "true", then this is an optional parameter.
  std::optional<bool> is_optional;

  /// \brief Equality operator
  ///
  /// \param other The oher object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const ActionParameterFactsheet& other) const
  {
    if (this->key != other.key) return false;
    if (this->value_data_type != other.value_data_type) return false;
    if (this->description != other.description) return false;
    if (this->is_optional != other.is_optional) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other objet to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const ActionParameterFactsheet& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ACTION_PARAMETER_FACTSHEET_HPP_
