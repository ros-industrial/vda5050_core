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

/// @struct EdgeState
/// @brief Trajectory JSON object for this edge as NURBS.
//         Defines the path, on which the AGV should
//         move between the start node and the end
//         node of the edge.
struct Trajectory
{
  /// @brief Degree of the NURBS curve defining the trajectory.
  ///        If not defined, the default value is 1.
  ///        Valid range: [1, 10]
  double degree = 1.0;

  /// @brief Array of knot values of the NURBS
  ///        knotVector has size of number of control
  ///        points + degree + 1.
  ///        Valid range: [0.0 … 1.0]
  std::vector<double> knot_vector;

  /// @brief Array of controlPoint objects defining the
  ///        control points of the NURBS, explicitly
  ///        including the start and end point.
  std::vector<ControlPoint> control_points;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__TRAJECTORY_HPP_
