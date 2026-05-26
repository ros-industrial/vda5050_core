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

#include "vda5050_core/master/map/map_loader.hpp"

#include <chrono>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>

namespace vda5050_core::master {

namespace {

void add_error(
  std::vector<MapLoadError>& errors, MapLoadErrorType type, std::string desc)
{
  errors.push_back({type, std::move(desc)});
}

// Absent map_status defaults to ENABLED; a present value must be a recognized
// string ("ENABLED" / "DISABLED") so a config typo surfaces instead of silently
// running enabled.
bool parse_map_status(
  const nlohmann::json& info, MapStatus& out, std::string& reason)
{
  if (!info.contains("map_status"))
  {
    out = MapStatus::ENABLED;
    return true;
  }
  if (!info.at("map_status").is_string())
  {
    reason = "map_info.map_status is present but not a string.";
    return false;
  }
  const auto s = info.at("map_status").get<std::string>();
  if (s == "ENABLED")
  {
    out = MapStatus::ENABLED;
    return true;
  }
  if (s == "DISABLED")
  {
    out = MapStatus::DISABLED;
    return true;
  }
  reason = "map_info.map_status has unrecognized value '" + s +
           "' (expected 'ENABLED' or 'DISABLED').";
  return false;
}

bool parse_map_info(
  const nlohmann::json& json, MapInfo& info, std::vector<MapLoadError>& errors)
{
  if (!json.contains("map_info") || !json.at("map_info").is_object())
  {
    add_error(
      errors, MapLoadErrorType::MISSING_REQUIRED_FIELD,
      "Top-level field 'map_info' is missing or not an object.");
    return false;
  }
  const auto& mi = json.at("map_info");
  if (!mi.contains("map_id") || !mi.at("map_id").is_string())
  {
    add_error(
      errors, MapLoadErrorType::MISSING_REQUIRED_FIELD,
      "map_info.map_id is missing or not a string.");
    return false;
  }
  if (!mi.contains("map_version") || !mi.at("map_version").is_string())
  {
    add_error(
      errors, MapLoadErrorType::MISSING_REQUIRED_FIELD,
      "map_info.map_version is missing or not a string.");
    return false;
  }
  info.map_id = mi.at("map_id").get<std::string>();
  info.map_version = mi.at("map_version").get<std::string>();
  std::string status_reason;
  if (!parse_map_status(mi, info.map_status, status_reason))
  {
    add_error(errors, MapLoadErrorType::INVALID_FIELD_VALUE, status_reason);
    return false;
  }
  if (mi.contains("map_descriptor") && mi.at("map_descriptor").is_string())
  {
    info.map_descriptor = mi.at("map_descriptor").get<std::string>();
  }
  return true;
}

bool parse_node(const nlohmann::json& jn, MapNode& out, std::string& reason)
{
  if (!jn.contains("node_id") || !jn.at("node_id").is_string())
  {
    reason = "node missing required string field 'node_id'";
    return false;
  }
  if (
    !jn.contains("x") || !jn.at("x").is_number() || !jn.contains("y") ||
    !jn.at("y").is_number())
  {
    reason = "node '" + jn.value("node_id", std::string{}) +
             "' missing required numeric fields 'x' / 'y'";
    return false;
  }
  out.node_id = jn.at("node_id").get<std::string>();
  out.x = jn.at("x").get<double>();
  out.y = jn.at("y").get<double>();
  if (jn.contains("theta") && jn.at("theta").is_number())
  {
    out.theta = jn.at("theta").get<double>();
  }
  if (
    jn.contains("allowed_deviation_xy") &&
    jn.at("allowed_deviation_xy").is_number())
  {
    out.allowed_deviation_xy = jn.at("allowed_deviation_xy").get<double>();
  }
  if (
    jn.contains("allowed_deviation_theta") &&
    jn.at("allowed_deviation_theta").is_number())
  {
    out.allowed_deviation_theta =
      jn.at("allowed_deviation_theta").get<double>();
  }
  if (jn.contains("map_description") && jn.at("map_description").is_string())
  {
    out.map_description = jn.at("map_description").get<std::string>();
  }
  return true;
}

bool parse_edge(const nlohmann::json& je, MapEdge& out, std::string& reason)
{
  if (!je.contains("edge_id") || !je.at("edge_id").is_string())
  {
    reason = "edge missing required string field 'edge_id'";
    return false;
  }
  if (
    !je.contains("start_node_id") || !je.at("start_node_id").is_string() ||
    !je.contains("end_node_id") || !je.at("end_node_id").is_string())
  {
    reason = "edge '" + je.value("edge_id", std::string{}) +
             "' missing required string fields 'start_node_id' / "
             "'end_node_id'";
    return false;
  }
  out.edge_id = je.at("edge_id").get<std::string>();
  out.start_node_id = je.at("start_node_id").get<std::string>();
  out.end_node_id = je.at("end_node_id").get<std::string>();
  if (je.contains("bidirectional") && je.at("bidirectional").is_boolean())
  {
    out.bidirectional = je.at("bidirectional").get<bool>();
  }
  if (je.contains("max_speed") && je.at("max_speed").is_number())
  {
    out.max_speed = je.at("max_speed").get<double>();
  }
  if (je.contains("length") && je.at("length").is_number())
  {
    out.length = je.at("length").get<double>();
  }
  return true;
}

}  // namespace

MapLoadResult load_from_file(const std::string& path)
{
  MapLoadResult result;
  std::ifstream in(path);
  if (!in.is_open())
  {
    add_error(
      result.errors, MapLoadErrorType::FILE_NOT_FOUND,
      "Cannot open map config file '" + path + "'.");
    return result;
  }
  std::stringstream buf;
  buf << in.rdbuf();
  if (in.bad())
  {
    add_error(
      result.errors, MapLoadErrorType::FILE_READ_FAILED,
      "I/O error while reading map config file '" + path + "'.");
    return result;
  }
  nlohmann::json parsed;
  try
  {
    parsed = nlohmann::json::parse(buf.str());
  }
  catch (const nlohmann::json::parse_error& e)
  {
    add_error(
      result.errors, MapLoadErrorType::JSON_PARSE_ERROR,
      std::string("JSON parse error in '") + path + "': " + e.what());
    return result;
  }
  return load_from_json(parsed, path);
}

namespace {

MapLoadResult parse_map_document(
  const nlohmann::json& json, const std::string& source_path)
{
  MapLoadResult result;
  if (!json.is_object())
  {
    add_error(
      result.errors, MapLoadErrorType::JSON_PARSE_ERROR,
      "Top-level JSON value is not an object.");
    return result;
  }

  Map map;
  map.source_path = source_path;
  map.loaded_at = std::chrono::system_clock::now();

  if (!parse_map_info(json, map.info, result.errors)) return result;

  if (!json.contains("nodes") || !json.at("nodes").is_array())
  {
    add_error(
      result.errors, MapLoadErrorType::MISSING_REQUIRED_FIELD,
      "Top-level field 'nodes' is missing or not an array.");
    return result;
  }
  const auto& jnodes = json.at("nodes");
  if (jnodes.empty())
  {
    add_error(
      result.errors, MapLoadErrorType::EMPTY_MAP,
      "Map config has no nodes — at least one is required.");
    return result;
  }
  std::unordered_set<std::string> seen_node_ids;
  for (std::size_t i = 0; i < jnodes.size(); ++i)
  {
    MapNode n;
    std::string reason;
    if (!parse_node(jnodes[i], n, reason))
    {
      add_error(
        result.errors, MapLoadErrorType::MISSING_REQUIRED_FIELD,
        "nodes[" + std::to_string(i) + "]: " + reason);
      continue;
    }
    if (!seen_node_ids.insert(n.node_id).second)
    {
      add_error(
        result.errors, MapLoadErrorType::DUPLICATE_NODE_ID,
        "Duplicate node_id '" + n.node_id + "' (nodes[" + std::to_string(i) +
          "]).");
      continue;
    }
    map.node_index.emplace(n.node_id, map.nodes.size());
    map.nodes.push_back(std::move(n));
  }

  // Edges (optional — a map with only nodes is legal).
  if (json.contains("edges") && json.at("edges").is_array())
  {
    const auto& jedges = json.at("edges");
    std::unordered_set<std::string> seen_edge_ids;
    for (std::size_t i = 0; i < jedges.size(); ++i)
    {
      MapEdge e;
      std::string reason;
      if (!parse_edge(jedges[i], e, reason))
      {
        add_error(
          result.errors, MapLoadErrorType::MISSING_REQUIRED_FIELD,
          "edges[" + std::to_string(i) + "]: " + reason);
        continue;
      }
      if (!seen_edge_ids.insert(e.edge_id).second)
      {
        add_error(
          result.errors, MapLoadErrorType::DUPLICATE_EDGE_ID,
          "Duplicate edge_id '" + e.edge_id + "' (edges[" + std::to_string(i) +
            "]).");
        continue;
      }
      if (
        seen_node_ids.find(e.start_node_id) == seen_node_ids.end() ||
        seen_node_ids.find(e.end_node_id) == seen_node_ids.end())
      {
        add_error(
          result.errors, MapLoadErrorType::DANGLING_EDGE_REF,
          "Edge '" + e.edge_id + "' references unknown node id '" +
            e.start_node_id + "' or '" + e.end_node_id + "'.");
        continue;
      }
      map.edge_index.emplace(e.edge_id, map.edges.size());
      map.edges.push_back(std::move(e));
    }
  }

  if (!result.errors.empty()) return result;

  result.map = std::move(map);
  return result;
}

}  // namespace

MapLoadResult load_from_json(
  const nlohmann::json& json, const std::string& source_path)
{
  try
  {
    return parse_map_document(json, source_path);
  }
  catch (const nlohmann::json::exception& e)
  {
    MapLoadResult result;
    add_error(
      result.errors, MapLoadErrorType::JSON_PARSE_ERROR,
      std::string("Unexpected JSON error while parsing map config: ") +
        e.what());
    return result;
  }
}

}  // namespace vda5050_core::master
