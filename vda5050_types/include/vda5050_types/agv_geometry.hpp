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

#ifndef VDA5050_TYPES__AGV_GEOMETRY_HPP_
#define VDA5050_TYPES__AGV_GEOMETRY_HPP_

#include <optional>
#include <vector>

#include "vda5050_types/envelope2d.hpp"
#include "vda5050_types/envelope3d.hpp"
#include "vda5050_types/wheel_definition.hpp"

namespace vda5050_types {

/// @brief Message defining the geometry properties of the AGV. eg: outlines and wheel positions.
struct AGVGeometry
{
  /// @brief Array of wheels, containing wheel arrangement and geometry.
  std::optional<std::vector<WheelDefinition>> wheel_definitions;

  /// @brief Array of AGV envelope curves in 2D. eg: the mechanical envelopes for unloaded and loaded state, the safety fields for different speed cases.
  std::optional<std::vector<Envelope2d>> envelopes_2d;

  /// @brief Array of AGV envelope curves in 3D.
  std::optional<std::vector<Envelope3d>> envelopes_3d;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const AGVGeometry& other) const
  {
    if (this->wheel_definitions != other.wheel_definitions) return false;
    if (this->envelopes_2d != other.envelopes_2d) return false;
    if (this->envelopes_3d != other.envelopes_3d) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const AGVGeometry& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__AGV_GEOMETRY_HPP_
