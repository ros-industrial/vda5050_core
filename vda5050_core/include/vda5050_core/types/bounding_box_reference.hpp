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

#ifndef VDA5050_CORE__TYPES__BOUNDING_BOX_REFERENCE_HPP_
#define VDA5050_CORE__TYPES__BOUNDING_BOX_REFERENCE_HPP_

#include <optional>

namespace vda5050_core {

namespace types {

/// \brief Point of reference for the location of the bounding box. The point of
/// reference is always the center of the bounding box bottom surface
/// (at height = 0) and is described in coordinates of the AGV’s coordinate
/// system
/// Part of the state message
struct BoundingBoxReference
{
  /// \brief X-coordinate of the point of reference
  double x = 0.0;

  /// \brief Y-coordinate of the point of reference
  double y = 0.0;

  /// \brief Z-coordinate of the point of reference
  double z = 0.0;

  /// \brief Orientation of the bounding box
  std::optional<double> theta;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const BoundingBoxReference& other) const
  {
    if (this->x != other.x) return false;
    if (this->y != other.y) return false;
    if (this->z != other.z) return false;
    if (this->theta != other.theta) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const BoundingBoxReference& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__BOUNDING_BOX_REFERENCE_HPP_
