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

#ifndef VDA5050_CORE__MASTER__MAP__MAP_HPP_
#define VDA5050_CORE__MASTER__MAP__MAP_HPP_

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vda5050_core::master {

// Master-side static topology map (loaded once from JSON at startup).

enum class MapStatus : std::uint8_t
{
  ENABLED = 0,
  DISABLED = 1,
};

struct MapInfo
{
  std::string map_id;
  std::string map_version;
  MapStatus map_status = MapStatus::ENABLED;
  std::string map_descriptor;
};

struct MapNode
{
  std::string node_id;
  double x = 0.0;                                 ///< [m]
  double y = 0.0;                                 ///< [m]
  std::optional<double> theta;                    ///< [rad]
  std::optional<double> allowed_deviation_xy;     ///< [m]
  std::optional<double> allowed_deviation_theta;  ///< [rad]
  std::string map_description;
};

struct MapEdge
{
  std::string edge_id;
  std::string start_node_id;
  std::string end_node_id;
  bool bidirectional = false;
  std::optional<double> max_speed;  ///< [m/s]
  std::optional<double> length;     ///< [m]
};

struct Map
{
  MapInfo info;
  std::vector<MapNode> nodes;
  std::vector<MapEdge> edges;
  std::unordered_map<std::string, std::size_t> node_index;
  std::unordered_map<std::string, std::size_t> edge_index;
  std::chrono::system_clock::time_point loaded_at;
  std::string source_path;

  const MapNode* find_node(const std::string& id) const;
  const MapEdge* find_edge(const std::string& id) const;
};

}  // namespace vda5050_core::master

#endif  // VDA5050_CORE__MASTER__MAP__MAP_HPP_
