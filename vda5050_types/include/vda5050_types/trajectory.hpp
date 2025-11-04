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

#ifndef VDA5050_TYPES__TRAJECTORY_HPP_
#define VDA5050_TYPES__TRAJECTORY_HPP_

#include <vector>

#include "vda5050_types/control_point.hpp"

namespace vda5050_types {

/// \brief Trajectory described as a NURBS (points defining a spline)
/// Part of the state message
struct Trajectory
{
  /// \brief The number of control points that influence a point on the
  /// curve. Increasing this value increases the continuity of the curve
  /// Valid range: [1, infinity)
  double degree = 1.0;

  /// \brief Sequence of parameter values determining how the control point
  /// affects the NURBS curve
  /// Valid range: [0.0, 1.0]
  std::vector<double> knot_vector;

  /// \brief Array of control points from beginning to end, defining the
  /// NURBS, explicitly including the start and end point.
  std::vector<ControlPoint> control_points;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Trajectory& other) const
  {
    if (this->degree != other.degree) return false;
    if (this->knot_vector != other.knot_vector) return false;
    if (this->control_points != other.control_points) return false;

    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Trajectory& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__TRAJECTORY_HPP_
