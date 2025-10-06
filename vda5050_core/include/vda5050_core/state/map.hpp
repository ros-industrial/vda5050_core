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

#ifndef VDA5050_CORE__STATE__MAP_HPP_
#define VDA5050_CORE__STATE__MAP_HPP_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "vda5050_core/state/map_status.hpp"

namespace vda5050_core {

namespace msg {

/// @struct Map
/// @brief Map objects that are currently stored on the vehicle.
struct Map
{
  /// @brief ID of the map describing a defined area of the vehicle's workspace.
  std::string map_id;

  /// @brief Version of the map.
  std::string map_version;

  /// @brief Additional information on the map.
  std::optional<std::string> map_description;

  /// @brief Enum {'ENABLED', 'DISABLED'}
  ///        'ENABLED': Indicates this map is currently active / used on the AGV.
  ///        At most one map with the same mapId can have its status set to 'ENABLED'.
  ///        'DISABLED': Indicates this map version is currently not enabled on the AGV
  ///        and thus could be enabled or deleted by request.
  MapStatus map_status = MapStatus::DISABLED;
};

using json = nlohmann::json;

void to_json(json& j, const Map& map)
{
  j["mapId"] = map.map_id;
  j["mapVersion"] = map.map_version;
  if (map.map_description.has_value())
  {
    j["mapDescription"] = *map.map_description;
  }
  j["mapStatus"] = map.map_status;
}
void from_json(const json& j, Map& map)
{
  map.map_id = j.at("mapId");
  map.map_version = j.at("mapVersion");
  if (j.contains("mapDescription"))
  {
    map.map_description = j.at("mapDescription");
  }
  map.map_status = j.at("mapStatus");
}

}  // namespace msg
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__MAP_HPP_