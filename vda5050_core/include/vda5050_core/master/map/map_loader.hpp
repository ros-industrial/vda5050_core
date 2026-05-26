/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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

#ifndef VDA5050_CORE__MASTER__MAP__MAP_LOADER_HPP_
#define VDA5050_CORE__MASTER__MAP__MAP_LOADER_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "vda5050_core/master/map/map.hpp"

namespace vda5050_core::master {

// Loads one topology map from a JSON config file (lenient on unknown keys).

enum class MapLoadErrorType : std::uint8_t
{
  FILE_NOT_FOUND,
  FILE_READ_FAILED,
  JSON_PARSE_ERROR,
  MISSING_REQUIRED_FIELD,
  DUPLICATE_NODE_ID,
  DUPLICATE_EDGE_ID,
  DANGLING_EDGE_REF,
  EMPTY_MAP,
};

struct MapLoadError
{
  MapLoadErrorType type;
  std::string description;
};

// Success: `map` set, `errors` empty (bool() true). Failure: the reverse.
struct MapLoadResult
{
  std::optional<Map> map;
  std::vector<MapLoadError> errors;

  explicit operator bool() const
  {
    return errors.empty() && map.has_value();
  }
};

MapLoadResult load_from_file(const std::string& path);

MapLoadResult load_from_json(
  const nlohmann::json& json, const std::string& source_path = "");

}  // namespace vda5050_core::master

#endif  // VDA5050_CORE__MASTER__MAP__MAP_LOADER_HPP_
