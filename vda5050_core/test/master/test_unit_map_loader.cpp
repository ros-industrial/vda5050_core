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

#include <gtest/gtest.h>

#include <algorithm>
#include <string>

#include <nlohmann/json.hpp>

#include "vda5050_core/master/map/map.hpp"
#include "vda5050_core/master/map/map_loader.hpp"

namespace vda5050_core::master::test {

namespace {

// Minimal valid map JSON used as the baseline; tests mutate copies for
// individual error paths.
nlohmann::json minimal_valid_json()
{
  return nlohmann::json::parse(R"({
    "map_info": { "map_id": "M1", "map_version": "1.0" },
    "nodes": [
      {"node_id": "N0", "x": 0.0, "y": 0.0},
      {"node_id": "N1", "x": 5.0, "y": 0.0}
    ],
    "edges": [
      {"edge_id": "E1", "start_node_id": "N0", "end_node_id": "N1"}
    ]
  })");
}

bool has_error_of_type(const MapLoadResult& r, MapLoadErrorType t)
{
  return std::any_of(r.errors.begin(), r.errors.end(), [&](const auto& e) {
    return e.type == t;
  });
}

}  // namespace

// ============================================================================
// File-level errors
// ============================================================================

TEST(MapLoaderTest, LoadFromFile_NotFound_ReturnsFileNotFound)
{
  auto r = load_from_file("/nonexistent/path/to/map.json");
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::FILE_NOT_FOUND));
}

TEST(MapLoaderTest, LoadFromJson_TopLevelNotObject_ReturnsParseError)
{
  nlohmann::json arr = nlohmann::json::array();
  auto r = load_from_json(arr);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::JSON_PARSE_ERROR));
}

// ============================================================================
// map_info validation
// ============================================================================

TEST(MapLoaderTest, LoadFromJson_MissingMapInfo_ReturnsMissingField)
{
  auto j = minimal_valid_json();
  j.erase("map_info");
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::MISSING_REQUIRED_FIELD));
}

TEST(MapLoaderTest, LoadFromJson_MissingMapId_ReturnsMissingField)
{
  auto j = minimal_valid_json();
  j["map_info"].erase("map_id");
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::MISSING_REQUIRED_FIELD));
}

TEST(MapLoaderTest, LoadFromJson_MissingMapVersion_ReturnsMissingField)
{
  auto j = minimal_valid_json();
  j["map_info"].erase("map_version");
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::MISSING_REQUIRED_FIELD));
}

// ============================================================================
// Nodes validation
// ============================================================================

TEST(MapLoaderTest, LoadFromJson_EmptyNodes_ReturnsEmptyMap)
{
  auto j = minimal_valid_json();
  j["nodes"] = nlohmann::json::array();
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::EMPTY_MAP));
}

TEST(MapLoaderTest, LoadFromJson_NodeMissingNodeId_ReturnsMissingField)
{
  auto j = minimal_valid_json();
  j["nodes"][0].erase("node_id");
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::MISSING_REQUIRED_FIELD));
}

TEST(MapLoaderTest, LoadFromJson_NodeMissingX_ReturnsMissingField)
{
  auto j = minimal_valid_json();
  j["nodes"][0].erase("x");
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::MISSING_REQUIRED_FIELD));
}

TEST(MapLoaderTest, LoadFromJson_DuplicateNodeId_ReturnsDuplicateNodeId)
{
  auto j = minimal_valid_json();
  j["nodes"].push_back(
    nlohmann::json::parse(R"({"node_id": "N0", "x": 1.0, "y": 1.0})"));
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::DUPLICATE_NODE_ID));
}

// ============================================================================
// Edges validation
// ============================================================================

TEST(MapLoaderTest, LoadFromJson_DuplicateEdgeId_ReturnsDuplicateEdgeId)
{
  auto j = minimal_valid_json();
  j["edges"].push_back(nlohmann::json::parse(
    R"({"edge_id": "E1", "start_node_id": "N0", "end_node_id": "N1"})"));
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::DUPLICATE_EDGE_ID));
}

TEST(MapLoaderTest, LoadFromJson_EdgeRefersUnknownNode_ReturnsDanglingEdgeRef)
{
  auto j = minimal_valid_json();
  j["edges"][0]["end_node_id"] = "DOES_NOT_EXIST";
  auto r = load_from_json(j);
  EXPECT_FALSE(static_cast<bool>(r));
  EXPECT_TRUE(has_error_of_type(r, MapLoadErrorType::DANGLING_EDGE_REF));
}

// ============================================================================
// Happy path + parsing details
// ============================================================================

TEST(MapLoaderTest, LoadFromJson_HappyPath_PopulatesAllFields)
{
  auto j = nlohmann::json::parse(R"({
    "map_info": {
      "map_id": "warehouse_floor1",
      "map_version": "2.3",
      "map_status": "ENABLED",
      "map_descriptor": "Floor 1"
    },
    "nodes": [
      {"node_id": "N0", "x": 1.5, "y": 2.5, "theta": 0.7,
       "allowed_deviation_xy": 0.4, "allowed_deviation_theta": 0.05,
       "map_description": "Pickup"}
    ],
    "edges": []
  })");
  auto r = load_from_json(j, "/configs/floor1.json");
  ASSERT_TRUE(static_cast<bool>(r));
  ASSERT_TRUE(r.map.has_value());
  EXPECT_EQ(r.map->info.map_id, "warehouse_floor1");
  EXPECT_EQ(r.map->info.map_version, "2.3");
  EXPECT_EQ(r.map->info.map_status, MapStatus::ENABLED);
  EXPECT_EQ(r.map->info.map_descriptor, "Floor 1");
  ASSERT_EQ(r.map->nodes.size(), 1u);
  EXPECT_EQ(r.map->nodes[0].node_id, "N0");
  EXPECT_DOUBLE_EQ(r.map->nodes[0].x, 1.5);
  EXPECT_DOUBLE_EQ(r.map->nodes[0].y, 2.5);
  ASSERT_TRUE(r.map->nodes[0].theta.has_value());
  EXPECT_DOUBLE_EQ(*r.map->nodes[0].theta, 0.7);
  ASSERT_TRUE(r.map->nodes[0].allowed_deviation_xy.has_value());
  EXPECT_DOUBLE_EQ(*r.map->nodes[0].allowed_deviation_xy, 0.4);
  EXPECT_EQ(r.map->nodes[0].map_description, "Pickup");
  EXPECT_EQ(r.map->source_path, "/configs/floor1.json");
}

TEST(MapLoaderTest, LoadFromJson_OptionalFieldsAbsent_DefaultsApplied)
{
  // No theta / allowed_deviation_* / map_description / map_descriptor.
  auto j = minimal_valid_json();
  auto r = load_from_json(j);
  ASSERT_TRUE(static_cast<bool>(r));
  ASSERT_TRUE(r.map.has_value());
  EXPECT_EQ(r.map->info.map_status, MapStatus::ENABLED);  // default
  EXPECT_TRUE(r.map->info.map_descriptor.empty());
  EXPECT_FALSE(r.map->nodes[0].theta.has_value());
  EXPECT_FALSE(r.map->nodes[0].allowed_deviation_xy.has_value());
  EXPECT_FALSE(r.map->edges[0].max_speed.has_value());
  EXPECT_FALSE(r.map->edges[0].length.has_value());
}

TEST(MapLoaderTest, LoadFromJson_BidirectionalDefaultFalse)
{
  auto j = minimal_valid_json();
  // Edge does not declare bidirectional.
  auto r = load_from_json(j);
  ASSERT_TRUE(static_cast<bool>(r));
  ASSERT_EQ(r.map->edges.size(), 1u);
  EXPECT_FALSE(r.map->edges[0].bidirectional);
}

TEST(MapLoaderTest, LoadFromJson_PopulatesNodeAndEdgeIndex)
{
  auto j = minimal_valid_json();
  auto r = load_from_json(j);
  ASSERT_TRUE(static_cast<bool>(r));
  EXPECT_NE(r.map->find_node("N0"), nullptr);
  EXPECT_NE(r.map->find_node("N1"), nullptr);
  EXPECT_EQ(r.map->find_node("MISSING"), nullptr);
  EXPECT_NE(r.map->find_edge("E1"), nullptr);
  EXPECT_EQ(r.map->find_edge("MISSING"), nullptr);
}

TEST(MapLoaderTest, LoadFromJson_LenientOnUnknownFields)
{
  auto j = minimal_valid_json();
  // Add a v2.1.0-shaped field the loader doesn't know about.
  j["map_info"]["map_download_link"] = "https://maps.example/floor1.bin";
  j["nodes"][0]["custom_extension_key"] = 42;
  auto r = load_from_json(j);
  EXPECT_TRUE(static_cast<bool>(r));
}

TEST(MapLoaderTest, MapStatusDisabledParsedCorrectly)
{
  auto j = minimal_valid_json();
  j["map_info"]["map_status"] = "DISABLED";
  auto r = load_from_json(j);
  ASSERT_TRUE(static_cast<bool>(r));
  EXPECT_EQ(r.map->info.map_status, MapStatus::DISABLED);
}

// ============================================================================
// Round-trip with sample data
// ============================================================================

TEST(MapLoaderTest, LoadFromFile_RoundTripSampleData)
{
#ifdef VDA5050_CORE__MASTER__SAMPLE_MAP_PATH
  auto r = load_from_file(VDA5050_CORE__MASTER__SAMPLE_MAP_PATH);
  ASSERT_TRUE(static_cast<bool>(r))
    << "Sample map at " << VDA5050_CORE__MASTER__SAMPLE_MAP_PATH
    << " did not load";
  ASSERT_TRUE(r.map.has_value());
  EXPECT_EQ(r.map->info.map_id, "warehouse_floor1");
  EXPECT_GE(r.map->nodes.size(), 1u);
  EXPECT_GE(r.map->edges.size(), 1u);
#else
  GTEST_SKIP()
    << "VDA5050_CORE__MASTER__SAMPLE_MAP_PATH not defined at compile time";
#endif
}

}  // namespace vda5050_core::master::test
