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

#ifndef VDA5050_TYPES__PHYSICAL_PARAMETERS_HPP_
#define VDA5050_TYPES__PHYSICAL_PARAMETERS_HPP_

#include <optional>

namespace vda5050_types {

/// \brief Describes physical properties of the AGV.
struct PhysicalParameters
{
  /// \brief Minimal controlled continuous speed of the AGV [m/s]
  double speed_min;

  /// \brief Maximum speed of the AGV [m/s]
  double speed_max;

  /// \brief Maximum acceleration with maximum load [m/s²]
  double acceleration_max;

  /// \brief Maximum deceleration with maximum load [m/s²]
  double deceleration_max;

  /// \brief Minimum height of AGV [m]
  double height_min;

  /// \brief Maximum height of AGV [m]
  double height_max;

  /// \brief Width of AGV [m]
  double width;

  /// \brief Length of AGV [m]
  double length;

  /// \brief Minimal controlled continuous rotation speed of the AGV [rad/s]
  std::optional<double> angular_speed_min;

  /// \brief Maximum rotation speed of the AGV [rad/s]
  std::optional<double> angular_speed_max;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const PhysicalParameters& other) const
  {
    if (this->speed_min != other.speed_min) return false;
    if (this->speed_max != other.speed_max) return false;
    if (this->acceleration_max != other.acceleration_max) return false;
    if (this->deceleration_max != other.deceleration_max) return false;
    if (this->height_min != other.height_min) return false;
    if (this->height_max != other.height_max) return false;
    if (this->width != other.width) return false;
    if (this->length != other.length) return false;
    if (this->angular_speed_min != other.angular_speed_min) return false;
    if (this->angular_speed_max != other.angular_speed_max) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const PhysicalParameters& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__PHYSICAL_PARAMETERS_HPP_
