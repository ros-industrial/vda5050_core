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

#include <memory>
#include <string>

#include "vda5050_core/errors/error_codes.hpp"
#include "vda5050_core/master/map/map.hpp"
#include "vda5050_core/master/validation/traversability_validator.hpp"

namespace {

::testing::AssertionResult AllErrorsHaveTraversabilityType(
  const vda5050_core::order_utils::ValidationResult& res)
{
  for (const auto& e : res.errors)
  {
    if (e.error_type != vda5050_core::errors::TraversabilityValidationError)
    {
      return ::testing::AssertionFailure()
             << "found error with type '" << e.error_type
             << "' (expected TraversabilityValidationError)";
    }
  }
  return ::testing::AssertionSuccess();
}

::testing::AssertionResult AnyErrorMentions(
  const vda5050_core::order_utils::ValidationResult& res,
  const std::string& needle)
{
  for (const auto& e : res.errors)
  {
    if (
      e.error_description &&
      e.error_description->find(needle) != std::string::npos)
    {
      return ::testing::AssertionSuccess();
    }
  }
  return ::testing::AssertionFailure()
         << "no error description mentions '" << needle << "'";
}

}  // namespace

namespace vda5050_core::master::test {

namespace {

// Build a Factsheet that supports the typical happy-path Order: generous
// limits, one supported action_type "pick" available on NODE/EDGE/INSTANT
// scopes with a "loadId" parameter key.
vda5050_core::types::Factsheet make_factsheet()
{
  vda5050_core::types::Factsheet fs;
  fs.protocol_limits.max_array_lens.order_nodes = 100;
  fs.protocol_limits.max_array_lens.order_edges = 100;
  fs.protocol_limits.max_array_lens.node_actions = 10;
  fs.protocol_limits.max_array_lens.edge_actions = 10;
  fs.protocol_limits.max_array_lens.actions_actions_parameters = 10;
  fs.protocol_limits.max_array_lens.instant_actions = 10;

  vda5050_core::types::AGVAction supported;
  supported.action_type = "pick";
  supported.action_scopes = {
    vda5050_core::types::ActionScope::NODE,
    vda5050_core::types::ActionScope::EDGE,
    vda5050_core::types::ActionScope::INSTANT};
  vda5050_core::types::ActionParameterFactsheet param;
  param.key = "loadId";
  supported.action_parameters =
    std::vector<vda5050_core::types::ActionParameterFactsheet>{param};
  fs.protocol_features.agv_actions = {supported};
  return fs;
}

// State where AGV is on node "N0" — trivial reachability for an order whose
// first node is "N0".
vda5050_core::types::State make_state_on_node(const std::string& node_id)
{
  vda5050_core::types::State s;
  s.last_node_id = node_id;
  vda5050_core::types::AGVPosition pos;
  pos.position_initialized = true;
  pos.x = 0.0;
  pos.y = 0.0;
  s.agv_position = pos;
  return s;
}

PreSendContext make_ctx(
  const vda5050_core::types::State& state,
  const vda5050_core::types::Factsheet& fs)
{
  return PreSendContext{
    vda5050_core::types::ConnectionState::ONLINE,
    state,
    fs,
    AGVState::AVAILABLE,
    std::nullopt,
    nullptr};
}

vda5050_core::types::Order make_minimal_order()
{
  vda5050_core::types::Order o;
  o.order_id = "ORDER_T1";
  o.order_update_id = 0;
  vda5050_core::types::Node n;
  n.node_id = "N0";
  n.sequence_id = 0;
  n.released = true;
  o.nodes.push_back(n);
  return o;
}

vda5050_core::types::Action make_pick_action()
{
  vda5050_core::types::Action a;
  a.action_id = "A1";
  a.action_type = "pick";
  a.blocking_type = vda5050_core::types::BlockingType::NONE;
  return a;
}

}  // namespace

// ============================================================================
// Order — happy paths
// ============================================================================

TEST(TraversabilityValidatorTest, OrderHappyPathOnNode)
{
  auto ctx = make_ctx(make_state_on_node("N0"), make_factsheet());
  auto order = make_minimal_order();
  auto res = validate_traversability(ctx, order);
  EXPECT_TRUE(static_cast<bool>(res));
  EXPECT_TRUE(res.errors.empty());
}

TEST(TraversabilityValidatorTest, OrderHappyPathByPositionWithinDeviation)
{
  auto state = make_state_on_node("OTHER");  // AGV not on first node
  state.agv_position->x = 0.05;              // 5cm offset
  state.agv_position->y = 0.0;
  auto ctx = make_ctx(state, make_factsheet());

  auto order = make_minimal_order();
  vda5050_core::types::NodePosition np;
  np.x = 0.0;
  np.y = 0.0;
  np.allowed_deviation_x_y = 0.10;  // 10cm tolerance
  order.nodes.front().node_position = np;

  auto res = validate_traversability(ctx, order);
  EXPECT_TRUE(static_cast<bool>(res));
}

// ============================================================================
// Order — reachability rejections
// ============================================================================

TEST(TraversabilityValidatorTest, OrderRejectFirstNodeUnreachable)
{
  auto state = make_state_on_node("OTHER");
  state.agv_position->x = 5.0;  // 5m away
  state.agv_position->y = 0.0;
  auto ctx = make_ctx(state, make_factsheet());

  auto order = make_minimal_order();
  vda5050_core::types::NodePosition np;
  np.x = 0.0;
  np.y = 0.0;
  np.allowed_deviation_x_y = 0.10;  // 10cm tolerance
  order.nodes.front().node_position = np;

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "deviation"));
}

TEST(TraversabilityValidatorTest, OrderRejectInsufficientReachabilityData)
{
  // AGV not on node, AND first node has no node_position.
  auto state = make_state_on_node("OTHER");
  auto ctx = make_ctx(state, make_factsheet());
  auto order = make_minimal_order();
  // node_position omitted
  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "Cannot determine reachability"));
}

// ============================================================================
// Order — action capability rejections
// ============================================================================

TEST(TraversabilityValidatorTest, OrderRejectActionTypeNotInFactsheet)
{
  auto ctx = make_ctx(make_state_on_node("N0"), make_factsheet());
  auto order = make_minimal_order();
  vda5050_core::types::Action unsupported;
  unsupported.action_id = "A1";
  unsupported.action_type = "fly";  // not in factsheet
  unsupported.blocking_type = vda5050_core::types::BlockingType::NONE;
  order.nodes.front().actions = {unsupported};

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "fly"));
}

TEST(TraversabilityValidatorTest, OrderRejectActionScopeNotNode)
{
  // Build factsheet where "pick" is INSTANT-only — not allowed on Nodes.
  auto fs = make_factsheet();
  fs.protocol_features.agv_actions[0].action_scopes = {
    vda5050_core::types::ActionScope::INSTANT};
  auto ctx = make_ctx(make_state_on_node("N0"), fs);

  auto order = make_minimal_order();
  order.nodes.front().actions = {make_pick_action()};

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "scope"));
}

TEST(TraversabilityValidatorTest, OrderRejectActionParameterKeyNotInFactsheet)
{
  auto ctx = make_ctx(make_state_on_node("N0"), make_factsheet());
  auto order = make_minimal_order();
  auto a = make_pick_action();
  vda5050_core::types::ActionParameter p;
  p.key = "weirdKey";  // not declared by factsheet
  p.value = "x";
  a.action_parameters = std::vector<vda5050_core::types::ActionParameter>{p};
  order.nodes.front().actions = {a};

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "weirdKey"));
}

// ============================================================================
// Order — array-size limit rejections
// ============================================================================

TEST(TraversabilityValidatorTest, OrderRejectExceedsMaxNodes)
{
  auto fs = make_factsheet();
  fs.protocol_limits.max_array_lens.order_nodes = 1;  // tight limit
  auto ctx = make_ctx(make_state_on_node("N0"), fs);

  auto order = make_minimal_order();
  vda5050_core::types::Node n2;
  n2.node_id = "N1";
  n2.sequence_id = 2;
  n2.released = true;
  order.nodes.push_back(n2);

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "order_nodes"));
}

TEST(TraversabilityValidatorTest, OrderRejectExceedsMaxNodeActions)
{
  auto fs = make_factsheet();
  fs.protocol_limits.max_array_lens.node_actions = 1;
  auto ctx = make_ctx(make_state_on_node("N0"), fs);

  auto order = make_minimal_order();
  order.nodes.front().actions = {make_pick_action(), make_pick_action()};

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "node_actions"));
}

// ============================================================================
// InstantActions — capability + limit
// ============================================================================

TEST(TraversabilityValidatorTest, InstantActionsHappyPath)
{
  auto ctx = make_ctx(make_state_on_node("N0"), make_factsheet());
  vda5050_core::types::InstantActions ia;
  ia.actions = {make_pick_action()};
  auto res = validate_traversability(ctx, ia);
  EXPECT_TRUE(static_cast<bool>(res));
}

TEST(TraversabilityValidatorTest, InstantActionsRejectActionTypeNotInFactsheet)
{
  auto ctx = make_ctx(make_state_on_node("N0"), make_factsheet());
  vda5050_core::types::InstantActions ia;
  vda5050_core::types::Action unsupported;
  unsupported.action_id = "A1";
  unsupported.action_type = "teleport";
  unsupported.blocking_type = vda5050_core::types::BlockingType::NONE;
  ia.actions = {unsupported};
  auto res = validate_traversability(ctx, ia);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "teleport"));
}

TEST(TraversabilityValidatorTest, InstantActionsRejectScopeNotInstant)
{
  auto fs = make_factsheet();
  fs.protocol_features.agv_actions[0].action_scopes = {
    vda5050_core::types::ActionScope::NODE};  // NODE only, NOT INSTANT
  auto ctx = make_ctx(make_state_on_node("N0"), fs);
  vda5050_core::types::InstantActions ia;
  ia.actions = {make_pick_action()};
  auto res = validate_traversability(ctx, ia);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "scope"));
}

TEST(TraversabilityValidatorTest, InstantActionsRejectExceedsMaxLength)
{
  auto fs = make_factsheet();
  fs.protocol_limits.max_array_lens.instant_actions = 1;
  auto ctx = make_ctx(make_state_on_node("N0"), fs);
  vda5050_core::types::InstantActions ia;
  ia.actions = {make_pick_action(), make_pick_action()};
  auto res = validate_traversability(ctx, ia);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "instant_actions"));
}

// ============================================================================
// Skip-when-factsheet-missing
// ============================================================================

TEST(TraversabilityValidatorTest, OrderSkipsCapabilityChecksWhenNoFactsheet)
{
  // No factsheet → skip capability + limit checks. Reachability still runs;
  // AGV is on first node so it passes.
  PreSendContext ctx{
    vda5050_core::types::ConnectionState::ONLINE,
    make_state_on_node("N0"),
    std::nullopt,
    AGVState::AVAILABLE,
    std::nullopt,
    nullptr};
  auto order = make_minimal_order();
  auto res = validate_traversability(ctx, order);
  EXPECT_TRUE(static_cast<bool>(res));
}

TEST(
  TraversabilityValidatorTest,
  InstantActionsSkipsCapabilityChecksWhenNoFactsheet)
{
  // No factsheet → no capability check on InstantActions; passes silently.
  PreSendContext ctx{
    vda5050_core::types::ConnectionState::ONLINE,
    make_state_on_node("N0"),
    std::nullopt,
    AGVState::AVAILABLE,
    std::nullopt,
    nullptr};
  vda5050_core::types::InstantActions ia;
  vda5050_core::types::Action a;
  a.action_id = "A1";
  a.action_type = "anything_goes";
  a.blocking_type = vda5050_core::types::BlockingType::NONE;
  ia.actions = {a};
  auto res = validate_traversability(ctx, ia);
  EXPECT_TRUE(static_cast<bool>(res));
}

// ============================================================================
// Multi-error accumulation
// ============================================================================

TEST(TraversabilityValidatorTest, MultipleErrorsAccumulate)
{
  auto fs = make_factsheet();
  fs.protocol_limits.max_array_lens.order_nodes = 1;  // tight limit
  auto ctx = make_ctx(make_state_on_node("N0"), fs);

  auto order = make_minimal_order();
  // Add a second node to exceed limit
  vda5050_core::types::Node n2;
  n2.node_id = "N1";
  n2.sequence_id = 2;
  n2.released = true;
  order.nodes.push_back(n2);
  // Add an unsupported action type
  vda5050_core::types::Action unsupported;
  unsupported.action_id = "A1";
  unsupported.action_type = "fly";
  unsupported.blocking_type = vda5050_core::types::BlockingType::NONE;
  order.nodes.front().actions = {unsupported};

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_GE(res.errors.size(), 2u);
  EXPECT_TRUE(AllErrorsHaveTraversabilityType(res));
  EXPECT_TRUE(AnyErrorMentions(res, "order_nodes"));
  EXPECT_TRUE(AnyErrorMentions(res, "fly"));
}

// ============================================================================
// Map integrity — cross-checks Order graph against loaded map.
// ============================================================================
//
// These tests exercise the new validate_map_integrity sub-check, which
// runs only when ctx.loaded_map is non-null. The pre_send_validator
// no-map gate covers the nullptr case separately.

namespace {

std::shared_ptr<const Map> make_two_node_map()
{
  Map m;
  m.info.map_id = "M1";
  m.info.map_version = "1.0";
  MapNode n0;
  n0.node_id = "N0";
  n0.x = 0.0;
  n0.y = 0.0;
  n0.allowed_deviation_xy = 0.5;
  MapNode n1;
  n1.node_id = "N1";
  n1.x = 5.0;
  n1.y = 0.0;
  n1.allowed_deviation_xy = 0.5;
  m.nodes = {n0, n1};
  m.node_index = {{"N0", 0}, {"N1", 1}};
  MapEdge e1;
  e1.edge_id = "E1";
  e1.start_node_id = "N0";
  e1.end_node_id = "N1";
  e1.bidirectional = false;
  m.edges = {e1};
  m.edge_index = {{"E1", 0}};
  return std::make_shared<const Map>(std::move(m));
}

PreSendContext make_ctx_with_map(
  const vda5050_core::types::State& state,
  const vda5050_core::types::Factsheet& fs, std::shared_ptr<const Map> mp)
{
  return PreSendContext{
    vda5050_core::types::ConnectionState::ONLINE,
    state,
    fs,
    AGVState::AVAILABLE,
    std::nullopt,
    std::move(mp)};
}

}  // namespace

TEST(TraversabilityValidatorTest, MapIntegrity_OrderNodeNotInMap_Rejects)
{
  auto ctx = make_ctx_with_map(
    make_state_on_node("N0"), make_factsheet(), make_two_node_map());
  auto order = make_minimal_order();
  order.nodes.front().node_id = "GHOST";  // not in the map

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AnyErrorMentions(res, "GHOST"));
  EXPECT_TRUE(AnyErrorMentions(res, "not present in the master's loaded map"));
}

TEST(TraversabilityValidatorTest, MapIntegrity_OrderEdgeNotInMap_Rejects)
{
  auto ctx = make_ctx_with_map(
    make_state_on_node("N0"), make_factsheet(), make_two_node_map());
  auto order = make_minimal_order();
  vda5050_core::types::Edge e;
  e.edge_id = "GHOST_EDGE";
  e.sequence_id = 1;
  e.released = true;
  e.start_node_id = "N0";
  e.end_node_id = "N1";
  order.edges.push_back(e);

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AnyErrorMentions(res, "GHOST_EDGE"));
}

TEST(TraversabilityValidatorTest, MapIntegrity_EdgeEndpointMismatch_Rejects)
{
  auto ctx = make_ctx_with_map(
    make_state_on_node("N0"), make_factsheet(), make_two_node_map());
  auto order = make_minimal_order();
  vda5050_core::types::Edge e;
  e.edge_id = "E1";
  e.sequence_id = 1;
  e.released = true;
  e.start_node_id = "N1";  // map has N0->N1, not N1->N0 (one-way)
  e.end_node_id = "N0";
  order.edges.push_back(e);

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AnyErrorMentions(res, "endpoints"));
}

TEST(TraversabilityValidatorTest, MapIntegrity_BidirectionalAllowsReverse)
{
  // Mark E1 bidirectional and request reverse traversal.
  Map m;
  m.info.map_id = "M1";
  m.info.map_version = "1.0";
  MapNode n0;
  n0.node_id = "N0";
  MapNode n1;
  n1.node_id = "N1";
  m.nodes = {n0, n1};
  m.node_index = {{"N0", 0}, {"N1", 1}};
  MapEdge e1;
  e1.edge_id = "E1";
  e1.start_node_id = "N0";
  e1.end_node_id = "N1";
  e1.bidirectional = true;
  m.edges = {e1};
  m.edge_index = {{"E1", 0}};
  auto mp = std::make_shared<const Map>(std::move(m));

  auto ctx = make_ctx_with_map(make_state_on_node("N0"), make_factsheet(), mp);
  auto order = make_minimal_order();
  vda5050_core::types::Edge e;
  e.edge_id = "E1";
  e.sequence_id = 1;
  e.released = true;
  e.start_node_id = "N1";
  e.end_node_id = "N0";
  order.edges.push_back(e);

  auto res = validate_traversability(ctx, order);
  EXPECT_TRUE(static_cast<bool>(res)) << "bidirectional edge should accept "
                                         "reverse-traversal endpoints";
}

TEST(TraversabilityValidatorTest, MapIntegrity_PositionWithinDeviation_Accepts)
{
  auto ctx = make_ctx_with_map(
    make_state_on_node("N0"), make_factsheet(), make_two_node_map());
  auto order = make_minimal_order();
  vda5050_core::types::NodePosition np;
  np.x = 0.3;  // map N0 is at (0,0), tolerance 0.5 → ok
  np.y = 0.0;
  order.nodes.front().node_position = np;

  auto res = validate_traversability(ctx, order);
  EXPECT_TRUE(static_cast<bool>(res));
}

TEST(TraversabilityValidatorTest, MapIntegrity_PositionExceedsDeviation_Rejects)
{
  auto ctx = make_ctx_with_map(
    make_state_on_node("N0"), make_factsheet(), make_two_node_map());
  auto order = make_minimal_order();
  vda5050_core::types::NodePosition np;
  np.x = 2.0;  // map N0 is at (0,0), tolerance 0.5 → way out
  np.y = 0.0;
  order.nodes.front().node_position = np;

  auto res = validate_traversability(ctx, order);
  EXPECT_FALSE(static_cast<bool>(res));
  EXPECT_TRUE(AnyErrorMentions(res, "outside the map's allowed_deviation_xy"));
}

}  // namespace vda5050_core::master::test
