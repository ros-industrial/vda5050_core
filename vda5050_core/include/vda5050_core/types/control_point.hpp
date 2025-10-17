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

#ifndef VDA5050_CORE__TYPES__CONTROL_POINT_HPP_
#define VDA5050_CORE__TYPES__CONTROL_POINT_HPP_

#include <nlohmann/json.hpp>
#include <optional>

namespace vda5050_core {

namespace types {

/// \struct ControlPoint
/// \brief  ControlPoint describing a trajectory (NURBS)
struct ControlPoint
{
  /// \brief X-coordinate described in the world
  ///        coordinate system.
  double x = 0.0;

  /// \brief Y-coordinate described in the world
  ///        coordinate system.
  double y = 0.0;

  /// \brief Degree of the NURBS curve defining the trajectory.
  ///        If not defined, the default value is 1.
  ///        Valid range: [1, 10]
  double weight = 1.0;
};

using json = nlohmann::json;

inline void to_json(json& j, const ControlPoint& point)
{
  j = json{{"x", point.x}, {"y", point.y}, {"weight", point.weight}};
}

inline void from_json(const json& j, ControlPoint& point)
{
  j.at("x").get_to(point.x);
  j.at("y").get_to(point.y);

  if (j.contains("weight") && !j.at("weight").is_null())
  {
    point.weight = j.at("weight").get<double>();
  }
  else
  {
    point.weight = 1.0;  // Default value if not provided
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__CONTROL_POINT_HPP_
