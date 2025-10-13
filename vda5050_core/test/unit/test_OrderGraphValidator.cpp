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

#include "vda5050_core/order_execution/Edge.hpp"
#include "vda5050_core/order_execution/Node.hpp"
#include "vda5050_core/order_execution/OrderGraphValidator.hpp"

class OrderGraphValidatorTest : public testing::Test
{
protected:
  /// TODO change released state for future tests
  vda5050_core::node::Node n1_{1, true, "node1"};
  vda5050_core::edge::Edge e2_{2, true, "edge2", "node1", "node5"};
  vda5050_core::node::Node n3_{3, true, "node3"};
  vda5050_core::edge::Edge e4_{4, true, "edge4", "node1", "node5"};
  vda5050_core::node::Node n5_{5, true, "node5"};
};

/// \brief Tests that graph validator returns true on a valid graph
TEST_F(OrderGraphValidatorTest, ValidGraphTest)
{
  std::vector<vda5050_core::node::Node> nodes;
  std::vector<vda5050_core::edge::Edge> edges;

  nodes.push_back(n1_);
  edges.push_back(e2_);
  nodes.push_back(n3_);
  edges.push_back(e4_);
  nodes.push_back(n5_);

  vda5050_core::order_graph_validator::OrderGraphValidator graph_validator{};

  EXPECT_TRUE(graph_validator.is_valid_graph(nodes, edges));
}

/// \brief Tests that graph validator returns false when nodes and edges are not in traversal order
TEST_F(OrderGraphValidatorTest, NotInTraversalOrderTest)
{
  std::vector<vda5050_core::node::Node> nodes;
  std::vector<vda5050_core::edge::Edge> edges;

  nodes.push_back(n1_);
  edges.push_back(e4_);
  nodes.push_back(n3_);
  edges.push_back(e2_);
  nodes.push_back(n5_);

  vda5050_core::order_graph_validator::OrderGraphValidator graph_validator{};

  EXPECT_FALSE(graph_validator.is_valid_graph(nodes, edges));
}

///\brief Tests that graph validator throws an error if zero nodes are present
TEST_F(OrderGraphValidatorTest, ZeroNodesTest)
{
  std::vector<vda5050_core::node::Node> nodes;
  std::vector<vda5050_core::edge::Edge> edges;

  EXPECT_EQ(nodes.size(), 0);
  EXPECT_EQ(edges.size(), 0);

  vda5050_core::order_graph_validator::OrderGraphValidator graph_validator{};

  EXPECT_FALSE(graph_validator.is_valid_graph(nodes, edges));
}

///\brief Tests that graph validator returns false if the difference in number of nodes and edges is greater than one
TEST_F(OrderGraphValidatorTest, IncorrectNumberOfNodesAndEdgesTest)
{
  std::vector<vda5050_core::node::Node> nodes;
  std::vector<vda5050_core::edge::Edge> edges;

  nodes.push_back(n1_);
  edges.push_back(e4_);
  nodes.push_back(n3_);
  edges.push_back(e2_);

  vda5050_core::node::Node n6 = vda5050_core::node::Node{6, true, "node6"};
  nodes.push_back(n6);

  nodes.push_back(n5_);

  vda5050_core::order_graph_validator::OrderGraphValidator graph_validator{};

  EXPECT_FALSE(graph_validator.is_valid_graph(nodes, edges));
}
