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

#ifndef VDA5050_TYPES__CONTROL_POINT_HPP_
#define VDA5050_TYPES__CONTROL_POINT_HPP_

namespace vda5050_types {

/// \brief  ControlPoint describing a trajectory (NURBS)
/// Part of the state message
struct ControlPoint
{
  /// \brief X-coordinate described in the world coordinate system
  double x = 0.0;

  /// \brief Y-coordinate described in the world coordinate system
  double y = 0.0;

  /// \brief Degree of the NURBS curve defining the trajectory. If not
  /// defined, the default value is 1.
  /// Valid range: [1, 10]
  double weight = 1.0;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const ControlPoint& other) const
  {
    if (this->x != other.x) return false;
    if (this->y != other.y) return false;
    if (this->weight != other.weight) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const ControlPoint& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__CONTROL_POINT_HPP_
