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

#include "vda5050_core/master/map/map.hpp"
#include "vda5050_core/master/validation/factsheet_alignment.hpp"
#include "vda5050_core/types/factsheet.hpp"

namespace vda5050_core::master::test {

namespace {

// Build a small map with a single edge whose max_speed is 1.0 m/s.
Map make_map_with_edge_max_speed(double edge_max_speed)
{
  Map m;
  m.info.map_id = "M1";
  m.info.map_version = "1.0";
  MapNode n0;
  n0.node_id = "N0";
  MapNode n1;
  n1.node_id = "N1";
  MapEdge e;
  e.edge_id = "E1";
  e.start_node_id = "N0";
  e.end_node_id = "N1";
  e.max_speed = edge_max_speed;
  m.nodes = {n0, n1};
  m.edges = {e};
  return m;
}

vda5050_core::types::Factsheet make_factsheet_with_speed(double speed_max)
{
  vda5050_core::types::Factsheet fs;
  fs.physical_parameters.speed_max = speed_max;
  return fs;
}

bool has_finding_with_code(
  const FactsheetAlignmentResult& r, const std::string& code)
{
  return std::any_of(r.findings.begin(), r.findings.end(), [&](const auto& f) {
    return f.code == code;
  });
}

}  // namespace

// ============================================================================
// Speed capability checks
// ============================================================================

TEST(FactsheetAlignmentTest, AgvSpeedExceedsAllEdges_NoFindings)
{
  auto m = make_map_with_edge_max_speed(1.0);
  auto fs = make_factsheet_with_speed(2.0);  // headroom

  auto r = check_factsheet_alignment(m, fs);
  EXPECT_TRUE(r.findings.empty());
  EXPECT_FALSE(r.has_error());
}

TEST(FactsheetAlignmentTest, AgvSpeedEqualsEdgeMax_NoFindings)
{
  auto m = make_map_with_edge_max_speed(1.0);
  auto fs = make_factsheet_with_speed(1.0);  // exact match accepted

  auto r = check_factsheet_alignment(m, fs);
  EXPECT_TRUE(r.findings.empty());
  EXPECT_FALSE(r.has_error());
}

TEST(FactsheetAlignmentTest, AgvSpeedBelowEdgeMax_ReturnsWarning)
{
  auto m = make_map_with_edge_max_speed(2.0);
  auto fs = make_factsheet_with_speed(1.5);

  auto r = check_factsheet_alignment(m, fs);
  EXPECT_FALSE(r.has_error());
  ASSERT_EQ(r.findings.size(), 1u);
  EXPECT_EQ(r.findings[0].severity, AlignmentSeverity::WARNING);
  EXPECT_EQ(r.findings[0].code, "SpeedExceedsCapability");
}

TEST(FactsheetAlignmentTest, EdgeWithoutMaxSpeed_SkippedFromCheck)
{
  // Edge declares no max_speed → no finding regardless of factsheet.
  Map m;
  m.info.map_id = "M1";
  m.info.map_version = "1.0";
  MapNode n0;
  n0.node_id = "N0";
  MapNode n1;
  n1.node_id = "N1";
  MapEdge e;
  e.edge_id = "E1";
  e.start_node_id = "N0";
  e.end_node_id = "N1";
  // e.max_speed left unset.
  m.nodes = {n0, n1};
  m.edges = {e};

  auto fs = make_factsheet_with_speed(0.1);  // very slow AGV — but no rule
  auto r = check_factsheet_alignment(m, fs);
  EXPECT_TRUE(r.findings.empty());
  EXPECT_FALSE(r.has_error());
}

TEST(FactsheetAlignmentTest, MultipleEdgesExceeded_AllReported)
{
  Map m;
  m.info.map_id = "M1";
  m.info.map_version = "1.0";
  MapNode n0;
  n0.node_id = "N0";
  MapNode n1;
  n1.node_id = "N1";
  MapEdge e1;
  e1.edge_id = "E1";
  e1.start_node_id = "N0";
  e1.end_node_id = "N1";
  e1.max_speed = 2.0;
  MapEdge e2;
  e2.edge_id = "E2";
  e2.start_node_id = "N1";
  e2.end_node_id = "N0";
  e2.max_speed = 3.0;
  m.nodes = {n0, n1};
  m.edges = {e1, e2};

  auto fs = make_factsheet_with_speed(1.0);
  auto r = check_factsheet_alignment(m, fs);
  EXPECT_FALSE(r.has_error());
  EXPECT_EQ(r.findings.size(), 2u);
}

TEST(FactsheetAlignmentTest, EmptyMap_NoFindings)
{
  // Map with nodes but no edges (legal config).
  Map m;
  m.info.map_id = "M1";
  m.info.map_version = "1.0";
  MapNode n;
  n.node_id = "N0";
  m.nodes = {n};
  // No edges — nothing to align.

  auto fs = make_factsheet_with_speed(1.0);
  auto r = check_factsheet_alignment(m, fs);
  EXPECT_TRUE(r.findings.empty());
  EXPECT_FALSE(r.has_error());
}

TEST(FactsheetAlignmentTest, UnsetSpeedMax_ReturnsNonFatalWarning)
{
  auto m = make_map_with_edge_max_speed(1.0);
  auto fs = make_factsheet_with_speed(0.0);  // factsheet not yet received

  auto r = check_factsheet_alignment(m, fs);
  EXPECT_FALSE(r.has_error());
  ASSERT_EQ(r.findings.size(), 1u);
  EXPECT_EQ(r.findings[0].severity, AlignmentSeverity::WARNING);
  EXPECT_TRUE(has_finding_with_code(r, "SpeedCapabilityUnknown"));
}

TEST(FactsheetAlignmentTest, EdgeMaxBelowSpeedMin_ReturnsWarning)
{
  auto m = make_map_with_edge_max_speed(0.05);
  auto fs = make_factsheet_with_speed(2.0);
  fs.physical_parameters.speed_min = 0.2;

  auto r = check_factsheet_alignment(m, fs);
  EXPECT_FALSE(r.has_error());
  EXPECT_TRUE(has_finding_with_code(r, "SpeedBelowMinimum"));
}

}  // namespace vda5050_core::master::test
