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

#ifndef VDA5050_TYPES__AGV_POSITION_HPP_
#define VDA5050_TYPES__AGV_POSITION_HPP_

#include <optional>
#include <string>

namespace vda5050_types {

/// \brief Defines the position on the map in world coordinate system
/// Part of the state message
struct AGVPosition
{
  /// \brief Indicates if the AGV's position is initialized
  ///   - True if position is initialized
  ///   - False if position is not initialized
  bool position_initialized = false;

  /// \brief Desribes the quality of localization and can be
  /// used to describe the accuracy of the current position information
  /// Primarily for logging and visualization purposes
  ///   - 0.0 denotes position unknown
  ///   - 1.0 denotes position known
  std::optional<double> localization_score;

  /// \brief Deviation from position in meters. Optional for vehicles
  /// that cannot estimate their deviation
  std::optional<double> deviation_range;

  /// \brief X-position on the map in reference to
  /// the map coordinate system.
  double x = 0.0;

  /// \brief Y-position on the map in reference to
  /// the map coordinate system.
  double y = 0.0;

  /// \brief Orientation of the AGV in radians
  /// Valid range: [-PI, PI]
  double theta = 0.0;

  /// \brief Unique identification of the map in
  /// which the position is referenced
  std::string map_id;

  /// \brief Additional information on the map.
  std::optional<std::string> map_description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const AGVPosition& other) const
  {
    if (position_initialized != other.position_initialized) return false;
    if (localization_score != other.localization_score) return false;
    if (deviation_range != other.deviation_range) return false;
    if (x != other.x) return false;
    if (y != other.y) return false;
    if (theta != other.theta) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const AGVPosition& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__AGV_POSITION_HPP_
