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

#ifndef VDA5050_CORE__TYPES__TRAJECTORY_HPP_
#define VDA5050_CORE__TYPES__TRAJECTORY_HPP_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "vda5050_core/types/control_point.hpp"

namespace vda5050_core {

namespace types {

/// \struct EdgeState
/// \brief Trajectory JSON object for this edge as NURBS.
//         Defines the path, on which the AGV should
//         move between the start node and the end
//         node of the edge.
struct Trajectory
{
  /// \brief Degree of the NURBS curve defining the trajectory.
  ///        If not defined, the default value is 1.
  ///        Valid range: [1, 10]
  double degree = 1.0;

  /// \brief Array of knot values of the NURBS
  ///        knotVector has size of number of control
  ///        points + degree + 1.
  ///        Valid range: [0.0 … 1.0]
  std::vector<double> knot_vector;

  /// \brief Array of controlPoint objects defining the
  ///        control points of the NURBS, explicitly
  ///        including the start and end point.
  std::vector<ControlPoint> control_points;

  /// \brief Compares two Trajectory objects for equality.
  /// \param other The Trajectory instance to compare with.
  /// \return True if degree, knotVector, and controlPoints are equal, otherwise false.
  inline bool operator==(const Trajectory& other) const
  {
    if (this->degree != other.degree) return false;
    if (this->knot_vector != other.knot_vector) return false;
    if (this->control_points != other.control_points) return false;

    return true;
  }

  /// \brief Compares two Trajectory objects for inequality.
  /// \param other The Trajectory instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const Trajectory& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Trajectory& t)
{
  j = json{
    {"degree", t.degree},
    {"knot_vector", t.knot_vector},
    {"control_points", t.control_points}};
}

inline void from_json(const json& j, Trajectory& t)
{
  j.at("degree").get_to(t.degree);
  j.at("knot_vector").get_to(t.knot_vector);
  j.at("control_points").get_to(t.control_points);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__TRAJECTORY_HPP_
