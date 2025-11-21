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

#ifndef VDA5050_TYPES__WHEEL_DEFINITION_HPP_
#define VDA5050_TYPES__WHEEL_DEFINITION_HPP_

#include <optional>
#include <string>

#include "vda5050_types/position.hpp"
#include "vda5050_types/wheel_definition_type.hpp"

namespace vda5050_types {

/// \brief Struct describing the wheel arrangement and geometry
struct WheelDefinition
{
  /// \brief The type of the wheel
  wheelDefinitionType type;

  /// \brief If set to "true", the wheel is actively driven.
  bool is_active_driven;

  /// \brief If set to "true", the wheel is actively steered.
  bool is_active_steered;

  /// \brief Pose of the AGV according to its coordinate system.
  Position position;

  /// \brief Nominal diameter of the wheel [m].
  double diameter;

  /// \brief Nominal width of the wheel [m].
  double width;

  /// \brief Nominal displacement of the wheel's center to the rotation points.
  /// This field is necessary for caster wheels.
  double center_displacement = 0.0;

  /// \brief Free-form text: can be used by the manufacturer to define
  /// constraints.
  std::optional<std::string> constraints;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const WheelDefinition& other) const
  {
    if (this->type != other.type) return false;
    if (this->is_active_driven != other.is_active_driven) return false;
    if (this->is_active_steered != other.is_active_steered) return false;
    if (this->position != other.position) return false;
    if (this->diameter != other.diameter) return false;
    if (this->width != other.width) return false;
    if (this->center_displacement != other.center_displacement) return false;
    if (this->constraints != other.constraints) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const WheelDefinition& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__WHEEL_DEFINITION_HPP_
