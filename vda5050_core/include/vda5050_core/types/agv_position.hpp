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

#ifndef VDA5050_CORE__TYPES__AGV_POSITION_HPP_
#define VDA5050_CORE__TYPES__AGV_POSITION_HPP_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace vda5050_core {

namespace types {

/// \struct AgvPosition
/// \brief  Current position of the AGV on the
///         map.
struct AgvPosition
{
  /// \brief “true”: position is initialized.
  ///        “false”: position is not initialized.
  bool position_initialized = false;

  /// \brief Describes the quality of the
  ///        localization and therefore, can be
  ///        used
  ///        Valid range: [0.0, 1.0]
  ///        0.0: position unknown
  ///        1.0: position known
  std::optional<double> localization_score;

  /// \brief Value for the deviation range of the
  ///        position in meters.
  std::optional<double> deviation_range;

  /// \brief X-position on the map in reference to
  ///        the map coordinate system.
  double x = 0.0;

  /// \brief Y-position on the map in reference to
  ///        the map coordinate system.
  double y = 0.0;

  /// \brief Orientation of the AGV.
  ///        Valid range: [-Pi, Pi]
  ///        0.0: position unknown
  ///        1.0: position known
  double theta = 0.0;

  /// \brief Unique identification of the map in
  ///        which the position is referenced
  std::string map_id;

  /// \brief Additional information on the map.
  std::optional<std::string> map_description;

  /// \brief Compare two AgvPosition objects for equality.
  /// \param other The AgvPosition instance to compare with.
  /// \return True if all compared fields are equal, otherwise false.
  inline bool operator==(const AgvPosition& other) const
  {
    if (position_initialized != other.position_initialized) return false;
    if (localization_score != other.localization_score) return false;
    if (deviation_range != other.deviation_range) return false;
    if (x != other.x) return false;
    if (y != other.y) return false;
    if (theta != other.theta) return false;
    return true;
  }

  /// \brief Compare two AgvPosition objects for inequality.
  /// \param other The AgvPosition instance to compare with.
  /// \return True if all compared fields are not equal, otherwise true.
  inline bool operator!=(const AgvPosition& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const AgvPosition& pos)
{
  j = json{
    {"position_initialized", pos.position_initialized},
    {"x", pos.x},
    {"y", pos.y},
    {"theta", pos.theta},
    {"map_id", pos.map_id}};

  if (pos.localization_score.has_value())
  {
    j["localization_score"] = pos.localization_score.value();
  }

  if (pos.deviation_range.has_value())
  {
    j["deviation_range"] = pos.deviation_range.value();
  }

  if (pos.map_description.has_value())
  {
    j["map_description"] = pos.map_description.value();
  }
}

inline void from_json(const json& j, AgvPosition& pos)
{
  j.at("position_initialized").get_to(pos.position_initialized);
  j.at("x").get_to(pos.x);
  j.at("y").get_to(pos.y);
  j.at("theta").get_to(pos.theta);
  j.at("map_id").get_to(pos.map_id);

  if (j.contains("localization_score") && !j.at("localization_score").is_null())
    pos.localization_score = j.at("localization_score").get<double>();
  else
    pos.localization_score = std::nullopt;

  if (j.contains("deviation_range") && !j.at("deviation_range").is_null())
    pos.deviation_range = j.at("deviation_range").get<double>();
  else
    pos.deviation_range = std::nullopt;

  if (j.contains("map_description") && !j.at("map_description").is_null())
    pos.map_description = j.at("map_description").get<std::string>();
  else
    pos.map_description = std::nullopt;
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__AGV_POSITION_HPP_
