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

#ifndef VDA5050_TYPES__VELOCITY_HPP_
#define VDA5050_TYPES__VELOCITY_HPP_

#include <optional>

namespace vda5050_types {

/// @struct Velocity
/// @brief The AGVs velocity in vehicle coordinate
struct Velocity
{
  /// @brief The AVGs velocity in its x directio
  std::optional<double> vx;

  /// @brief The AVGs velocity in its y direction
  std::optional<double> vy;

  /// @brief The AVGs turning speed around its z axis
  std::optional<double> omega;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__VELOCITY_HPP_
