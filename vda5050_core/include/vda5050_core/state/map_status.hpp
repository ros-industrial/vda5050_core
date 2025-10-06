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

#ifndef VDA5050_CORE__STATE__MAP_STATUS_HPP_
#define VDA5050_CORE__STATE__MAP_STATUS_HPP_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "vda5050_core/state/map_status.hpp"

namespace vda5050_core {

namespace msg {

/// @enum MapStatus
/// @brief Map objects that are currently stored on the vehicle.
enum class MapStatus
{
  /// @brief Indicates this map version is currently not enabled on the AGV and thus
  ///        could be enabled or deleted by request.
  DISABLED = 0,
  /// @brief Indicates this map is currently active / used on the AGV. At most one map
  ///        with the same mapId can have its status set to 'ENABLED'.
  ENABLED = 1,
};

using json = nlohmann::json;

inline void to_json(json& j, const MapStatus& map_status)
{
  switch (map_status)
  {
    case MapStatus::DISABLED:
      j = "DISABLED";
      break;
    case MapStatus::ENABLED:
      j = "ENABLED";
      break;
    default:
      j = "UNKNOWN";
      break;
  }
}

inline void from_json(const json& j, MapStatus& map_status)
{
  auto str = j.get<std::string>();
  if (str == "DISABLED")
  {
    map_status = MapStatus::DISABLED;
  }
  else if (str == "ENABLED")
  {
    map_status = MapStatus::ENABLED;
  }
}

}  // namespace msg
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__MAP_STATUS_HPP_