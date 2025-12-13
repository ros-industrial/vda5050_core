/**
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

#include <gtest/gtest.h>

#include <optional>
#include <vector>

#include <vda5050_types/edge.hpp>
#include <vda5050_types/node.hpp>
#include <vda5050_types/order.hpp>
#include "vda5050_core/client/order/order_graph_validator.hpp"
#include "vda5050_core/client/order/validation_result.hpp"

class OrderGraphValidatorTest : public testing::Test
{
protected:
  vda5050_types::Node n0_{"node0", 0, true, {}, std::nullopt, std::nullopt};
  vda5050_types::Edge e1_{"edge1",      1,
                          "node0",      "node2",
                          true,         {},
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt};
  vda5050_types::Node n2_{"node2", 2, true, {}, std::nullopt, std::nullopt};
  vda5050_types::Edge e3_{"edge3",      3,
                          "node2",      "node4",
                          true,         {},
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt, std::nullopt,
                          std::nullopt};
  vda5050_types::Node n4_{"node4", 4, true, {}, std::nullopt, std::nullopt};

  vda5050_types::Order order_{};
  std::vector<vda5050_types::Node> nodes;
  std::vector<vda5050_types::Edge> edges;
};

//=============================================================================
/// \brief Tests that graph validator returns true on a valid graph
TEST_F(OrderGraphValidatorTest, ValidGraphTest)
{
  nodes.push_back(n0_);
  edges.push_back(e1_);
  nodes.push_back(n2_);
  edges.push_back(e3_);
  nodes.push_back(n4_);

  order_.order_id = "ValidGraphTest";
  order_.order_update_id = 0;
  order_.edges = edges;
  order_.nodes = nodes;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_TRUE(res);
}

//=============================================================================
// /// \brief Tests that graph validator returns false when nodes and edges are
// not in traversal order
TEST_F(OrderGraphValidatorTest, NotInTraversalOrderTest)
{
  nodes.push_back(n0_);
  edges.push_back(e3_);
  nodes.push_back(n2_);
  edges.push_back(e1_);
  nodes.push_back(n4_);

  order_.order_id = "NotInTraversalOrderTest";
  order_.order_update_id = 0;
  order_.edges = edges;
  order_.nodes = nodes;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that graph validator returns false if there are more nodes
/// than edges
TEST_F(OrderGraphValidatorTest, MoreNodesThanEdgesTest)
{
  nodes.push_back(n0_);
  edges.push_back(e1_);
  nodes.push_back(n2_);
  edges.push_back(e3_);
  nodes.push_back(n4_);

  vda5050_types::Node n6{"node6", 6, true, {}, std::nullopt, std::nullopt};
  nodes.push_back(n6);

  order_.order_id = "MoreNodesThanEdgesTest";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that validation fails if there are more edges
/// than nodes
TEST_F(OrderGraphValidatorTest, MoreEdgesThanNodesTest)
{
  nodes.push_back(n0_);
  edges.push_back(e1_);
  nodes.push_back(n2_);
  edges.push_back(e3_);

  vda5050_types::Edge e5{"edge5",      5,
                         "node4",      "node6",
                         true,         {},
                         std::nullopt, std::nullopt,
                         std::nullopt, std::nullopt,
                         std::nullopt, std::nullopt,
                         std::nullopt, std::nullopt,
                         std::nullopt, std::nullopt,
                         std::nullopt};
  edges.push_back(e5);

  order_.order_id = "MoreEdgesThanNodesTest";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that an edge in the right sequenceId order causes validation
/// to fail if its startNodeId and endNodeId do not match the nodeIds of
/// its neighbouring nodes
TEST_F(OrderGraphValidatorTest, ValidEdgesTest)
{
  nodes.push_back(n0_);

  e1_.start_node_id = "foo";
  e1_.end_node_id = "bar";
  edges.push_back(e1_);

  nodes.push_back(n2_);

  order_.order_id = "ValidEdgesTest";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that an order with odd node sequenceIds and even edge
/// sequenceIds causes validation to fail
TEST_F(OrderGraphValidatorTest, NodeWithOddSequenceIdTest)
{
  vda5050_types::Node odd_node1{"oddNode1",   1,           true, {},
                                std::nullopt, std::nullopt};
  nodes.push_back(odd_node1);
  vda5050_types::Node odd_node2{"oddNode2",   3,           true, {},
                                std::nullopt, std::nullopt};
  nodes.push_back(odd_node2);

  vda5050_types::Edge even_edge{"evenEdge",   2,
                                "node4",      "node6",
                                true,         {},
                                std::nullopt, std::nullopt,
                                std::nullopt, std::nullopt,
                                std::nullopt, std::nullopt,
                                std::nullopt, std::nullopt,
                                std::nullopt, std::nullopt,
                                std::nullopt};
  edges.push_back(even_edge);

  order_.order_id = "NodeWithOddSequenceIdTest";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that no two nodes share the same sequenceId
TEST_F(OrderGraphValidatorTest, DuplicateNodeSequenceIdTest)
{
  n2_.sequence_id = 0;

  nodes.push_back(n0_);
  nodes.push_back(n2_);

  edges.push_back(e1_);

  order_.order_id = "DuplicateNodeSequenceIdTest";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that there is only one base in the order
TEST_F(OrderGraphValidatorTest, MultipleBaseTest)
{
  /// n0_, e1_, n2_, and n4_ all released. Create a gap by setting e3_ to
  /// unreleased.
  e3_.released = false;

  nodes.push_back(n0_);
  nodes.push_back(n2_);
  nodes.push_back(n4_);

  edges.push_back(e1_);
  edges.push_back(e3_);

  order_.order_id = "MultipleBaseTest";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_graph(order_);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief Tests that there is only one base in the order
TEST_F(OrderGraphValidatorTest, ValidOrderUpdate)
{
  // Base order: node0 -> node2
  nodes = {n0_, n2_};
  edges = {e1_};

  order_.order_id = "OrderA";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto base_order = order_;

  // Next order continues from node2 -> node4
  nodes = {n2_, n4_};
  edges = {e3_};

  order_.order_update_id = 1;
  order_.nodes = nodes;
  order_.edges = edges;

  auto next_order = order_;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    base_order, next_order);

  EXPECT_TRUE(res);
}

//=============================================================================
/// \brief test if base order is invalid
TEST_F(OrderGraphValidatorTest, InvalidBaseOrder)
{
  // Invalid base order: missing edge
  nodes = {n0_, n2_};
  edges = {};

  order_.order_id = "OrderA";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto base_order = order_;

  // Valid next order
  nodes = {n2_, n4_};
  edges = {e3_};

  order_.order_update_id = 1;
  order_.nodes = nodes;
  order_.edges = edges;

  auto next_order = order_;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    base_order, next_order);

  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief test if next order is invalid
TEST_F(OrderGraphValidatorTest, InvalidNextOrder)
{
  // Valid base order
  nodes = {n0_, n2_};
  edges = {e1_};

  order_.order_id = "OrderA";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto base_order = order_;

  // Invalid next order: wrong edge count
  nodes = {n2_, n4_};
  edges = {};

  order_.order_update_id = 1;
  order_.nodes = nodes;
  order_.edges = edges;

  auto next_order = order_;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    base_order, next_order);

  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief test for order id mismatch between base order and next order
TEST_F(OrderGraphValidatorTest, OrderIdMismatch)
{
  // Base order
  nodes = {n0_, n2_};
  edges = {e1_};

  order_.order_id = "OrderA";
  order_.order_update_id = 0;
  order_.nodes = nodes;
  order_.edges = edges;

  auto base_order = order_;

  // Next order with different order_id
  nodes = {n2_, n4_};
  edges = {e3_};

  order_.order_id = "OrderB";  // mismatch
  order_.order_update_id = 1;
  order_.nodes = nodes;
  order_.edges = edges;

  auto next_order = order_;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    base_order, next_order);

  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief test if order update id of next order is greater than base order
TEST_F(OrderGraphValidatorTest, OrderUpdateIdRegression)
{
  // Base order update_id = 2
  nodes = {n0_, n2_};
  edges = {e1_};

  order_.order_id = "OrderA";
  order_.order_update_id = 2;
  order_.nodes = nodes;
  order_.edges = edges;

  auto base_order = order_;

  // Next order update_id = 1 (invalid)
  nodes = {n2_, n4_};
  edges = {e3_};

  order_.order_update_id = 1;
  order_.nodes = nodes;
  order_.edges = edges;

  auto next_order = order_;

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    base_order, next_order);

  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief test if base order and next order is valid for stitching
TEST_F(OrderGraphValidatorTest, ValidOrderStitching)
{
  // Base Order: Node0(Rel) -> Edge1(Rel) -> Node2(Rel)
  // Next Order:                             Node2(Rel) -> Edge3(Rel) -> Node4(Rel)

  // Path: [Node0] -> [Edge1] -> [Node2] (All Released)
  order_.order_id = "StitchTest";
  order_.order_update_id = 0;
  order_.nodes = {n0_, n2_};
  order_.edges = {e1_};

  // Path: [Node2] -> [Edge3] -> [Node4]
  vda5050_types::Order next_order;
  next_order.order_id = "StitchTest";
  next_order.order_update_id = 1;
  next_order.nodes = {n2_, n4_};  // Starts at n2_
  next_order.edges = {e3_};

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    order_, next_order);
  EXPECT_TRUE(res);
}

//=============================================================================
/// \brief test for invalid stitch
TEST_F(OrderGraphValidatorTest, InvalidOrderStitching)
{
  // CASE 1: Gap in Graph
  // Base Order: Node0(Rel) -> Edge1(Rel) -> Node2(Rel)
  // Next Order:                                            Node4(Rel) -> ...
  order_.order_id = "StitchFailNode";
  order_.order_update_id = 0;
  order_.nodes = {n0_, n2_};
  order_.edges = {e1_};

  // Starts at Node4 (The robot is at Node2, cannot jump to Node4)
  vda5050_types::Order next_order;
  next_order.order_id = "StitchFailNode";
  next_order.order_update_id = 1;
  next_order.nodes = {n4_};  // Error: Should start at n2_
  next_order.edges = {e3_};

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    order_, next_order);
  EXPECT_FALSE(res);

  // CASE 2: Backtracking / Discontinuity
  // Base Order: Node0(Rel) -> Edge1(Rel) -> Node2(Rel)
  // Next Order: Node0(Rel) -> ...
  order_.order_id = "StitchFailNode";
  order_.order_update_id = 0;
  order_.nodes = {n0_, n2_};
  order_.edges = {e1_};

  // Starts at Node4 (The robot is at Node2, next order cant jump to Node0)
  next_order.order_id = "StitchFailNode";
  next_order.order_update_id = 1;
  next_order.nodes = {n0_};  // Error: Should start at n2_
  next_order.edges = {e3_};

  res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    order_, next_order);
  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief test for sequence mismatch
TEST_F(OrderGraphValidatorTest, SequenceMismatch)
{
  // Base Order: Node0(Rel) -> Edge1(Rel) -> Node2(Rel, Seq=2)
  // Next Order:                             Node2(Rel, Seq=0) -> Edge3 -> ...
  // Ends at Node2 (Sequence ID 2)

  order_.order_id = "StitchFailSeq";
  order_.order_update_id = 0;
  order_.nodes = {n0_, n2_};
  order_.edges = {e1_};

  // Starts at Node2, BUT with Sequence ID 0
  vda5050_types::Node n2_wrong_seq = n2_;
  n2_wrong_seq.sequence_id = 0;

  vda5050_types::Order next_order;
  next_order.order_id = "StitchFailSeq";
  next_order.order_update_id = 1;
  next_order.nodes = {n2_wrong_seq, n4_};
  next_order.edges = {e3_};

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    order_, next_order);

  EXPECT_FALSE(res);
}

//=============================================================================
/// \brief test for the "Last Released Node" logic.
TEST_F(OrderGraphValidatorTest, StitchingReplacesHorizon)
{
  // Base Order: Node0(Rel) -> Edge1(Rel) -> Node2(Rel) -> Edge3(Unreleased) -> Node4(Unreleased)
  // Next Order:                             Node2(Rel) -> Edge3(Rel)        -> Node4(Rel)

  vda5050_types::Edge e3_horizon = e3_;
  e3_horizon.released = false;
  vda5050_types::Node n4_horizon = n4_;
  n4_horizon.released = false;

  order_.order_id = "StitchingReplacesHorizon";
  order_.order_update_id = 0;
  order_.nodes = {n0_, n2_, n4_horizon};
  order_.edges = {e1_, e3_horizon};

  vda5050_types::Order next_order;
  next_order.order_id = "StitchingReplacesHorizon";
  next_order.order_update_id = 1;
  next_order.nodes = {
    n2_, n4_};  // Stitching at n2 (Last released), IGNORING n4_horizon
  next_order.edges = {e3_};

  auto res = vda5050_core::order::OrderGraphValidator::is_valid_order_update(
    order_, next_order);
  EXPECT_TRUE(res);
}
