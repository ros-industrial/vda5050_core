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

/// @struct AgvPosition
/// @brief  Current position of the AGV on the
///         map.
struct AgvPosition
{
  /// @brief “true”: position is initialized.
  ///        “false”: position is not initialized.
  bool position_initialized = false;

  /// @brief Describes the quality of the
  ///        localization and therefore, can be
  ///        used
  ///        Valid range: [0.0, 1.0]
  ///        0.0: position unknown
  ///        1.0: position known
  std::optional<double> localization_score;

  /// @brief Value for the deviation range of the
  ///        position in meters.
  std::optional<double> deviation_range;

  /// @brief X-position on the map in reference to
  ///        the map coordinate system.
  double x = 0.0;

  /// @brief Y-position on the map in reference to
  ///        the map coordinate system.
  double y = 0.0;

  /// @brief Orientation of the AGV.
  ///        Valid range: [-Pi, Pi]
  ///        0.0: position unknown
  ///        1.0: position known
  double theta = 0.0;

  /// @brief Unique identification of the map in
  ///        which the position is referenced
  std::string map_id;

  /// @brief Additional information on the map.
  std::optional<std::string> map_description;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__AGV_POSITION_HPP_
