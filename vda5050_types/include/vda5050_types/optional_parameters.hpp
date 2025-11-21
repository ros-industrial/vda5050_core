/**
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

#ifndef VDA5050_TYPES__OPTIONAL_PARAMETERS_HPP_
#define VDA5050_TYPES__OPTIONAL_PARAMETERS_HPP_

#include <optional>
#include <string>
#include "vda5050_types/support_type.hpp"

namespace vda5050_types {

/// \brief Supported or required optional parameters.
struct OptionalParameters
{
  /// \brief Full name of optional parameter.
  /// eg: "order.nodes.nodePosition.allowedDeviationTheta".
  std::string parameter;

  /// \brief Type of support for the optional parameter.
  SupportType support;

  /// \brief Free-form text: description of optional parameter.
  /// eg:
  ///   - Reason why the optional parameter direction is necessary for this AGV type.
  ///   - The parameter nodeMarker shall contain unsigned integers only.
  ///   - NURBS support is limited to straight lines and circular segments.
  std::optional<std::string> description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const OptionalParameters& other) const
  {
    if (this->parameter != other.parameter) return false;
    if (this->support != other.support) return false;
    if (this->description != other.description) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const OptionalParameters& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__OPTIONAL_PARAMETERS_HPP_
