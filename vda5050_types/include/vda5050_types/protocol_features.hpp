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

#ifndef VDA5050_TYPES__PROTOCOL_FEATURES_HPP_
#define VDA5050_TYPES__PROTOCOL_FEATURES_HPP_

#include <vector>
#include "vda5050_types/agv_action.hpp"
#include "vda5050_types/optional_parameter.hpp"

namespace vda5050_types {

/// \brief Message defining the actions and parameters supported by the AGV.
struct ProtocolFeatures
{
  /// \brief Array of supported and/or required optional parameters.
  /// Optional parameters that are not listed here are assumed to be not supported
  /// by the AGV
  std::vector<OptionalParameter> optional_parameters;

  /// \brief Array of actions with parameters supported by this AGV. This includes
  /// standard actions specified in VDA5050 and manufacturer-specific actions.
  std::vector<AGVAction> agv_actions;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const ProtocolFeatures& other) const
  {
    if (this->optional_parameters != other.optional_parameters) return false;
    if (this->agv_actions != other.agv_actions) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const ProtocolFeatures& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__PROTOCOL_FEATURES_HPP_
