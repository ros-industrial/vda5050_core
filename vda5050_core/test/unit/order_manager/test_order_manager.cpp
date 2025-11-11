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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include "vda5050_core/order_execution/edge.hpp"
#include "vda5050_core/order_execution/node.hpp"
#include "vda5050_core/order_execution/order.hpp"
#include "vda5050_core/order_execution/order_manager.hpp"
#include "vda5050_core/types/state.hpp"
#include "vda5050_core/types/header.hpp"
#include "vda5050_core/types/node_state.hpp"
#include "vda5050_core/state_manager/state_manager.hpp"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::Return;

class OrderManagerTest : public testing::Test
{
protected:
  /// Basic set of nodes and edges to construct a fully released order
  vda5050_core::node::Node n1{1, true, "node1"};
  vda5050_core::edge::Edge e2{2, true, "edge2", "node1", "node5"};
  vda5050_core::node::Node n3{3, true, "node3"};
  vda5050_core::edge::Edge e4{4, true, "edge4", "node1", "node5"};
  vda5050_core::node::Node n5{5, true, "node5"};

  std::vector<vda5050_core::node::Node> fully_released_nodes = {n1, n3, n5};
  std::vector<vda5050_core::edge::Edge> fully_released_edges = {e2, e4};
  vda5050_core::order::Order fully_released_order{"order1", 0, fully_released_nodes, fully_released_edges};

  /// create a valid new order that the vehicle can reach from the fully_released_order
  vda5050_core::edge::Edge e6{6, true, "edge6", "node5", "node7"};
  vda5050_core::node::Node n7{7, true, "node7"};
  std::vector<vda5050_core::node::Node> order2Nodes{n5, n7};
  std::vector<vda5050_core::edge::Edge> order2Edges{e6};
  vda5050_core::order::Order order2{"order2", 0, order2Nodes, order2Edges};

  /// Create a partially released order
  vda5050_core::edge::Edge unreleased_e4{4, false, "edge4", "node1", "node5"};
  vda5050_core::node::Node unreleased_n5{5, false, "node5"};
  std::vector<vda5050_core::node::Node> partially_released_nodes = {
    n1, n3, unreleased_n5};
  std::vector<vda5050_core::edge::Edge> partially_released_edges = {
    e2, unreleased_e4};
  vda5050_core::order::Order partially_released_order{
    "order1", 0, partially_released_nodes, partially_released_edges};

  /// Update the partially released order
  std::vector<vda5050_core::node::Node> order_update_nodes = {n3, n5};
  std::vector<vda5050_core::edge::Edge> order_update_edges = {e4};
  vda5050_core::order::Order order_update{
    "order1", 1, order_update_nodes, order_update_edges};

  /// Instance of an OrderManager
  vda5050_core::order_manager::OrderManager orderManager{};

  /// Snapshot of the AGV's state if it has no existing order 
  vda5050_core::types::State init_state{};

  /// Snapshot of the AGV's state after accepting and completing fully_released_order
  vda5050_core::types::State fully_released_state{};

  /// Snapshot of the AGV's state after accepting and completing partially_released_order
  vda5050_core::types::State partially_released_state{};

  /// Snapshot of the AGV's state after accepting and completing order_update
  vda5050_core::types::State order_update_state{};

  /// Setup function that runs before each test
  void SetUp() override
  {
    /// NOTE: fully_released_state is AFTER the vehicle has completed the order, so node and edge states should be empty!    
    fully_released_state.order_id = "order1";
    fully_released_state.last_node_id = "node5";
    fully_released_state.last_node_sequence_id = 5;
    fully_released_state.order_update_id = 0;

    partially_released_state.order_id = "order1";
    partially_released_state.last_node_id = "node3";
    partially_released_state.last_node_sequence_id = 3;
    partially_released_state.order_update_id = 0;

    order_update_state.order_id = "order1";
    order_update_state.last_node_id = "node5";
    order_update_state.last_node_sequence_id = 5;
    order_update_state.order_update_id = 1;

  }

  /// \brief Helper function to create a vector of node states
  std::vector<vda5050_core::types::NodeState> create_node_states(std::vector<vda5050_core::node::Node>& nodes)
  {
    std::vector<vda5050_core::types::NodeState> node_states_vector;
    for (vda5050_core::node::Node n : nodes)
    {
      vda5050_core::types::NodeState node_state{};
      node_state.node_id = n.node_id();
      node_state.sequence_id = n.sequence_id();
      node_state.released = n.released();

      node_states_vector.push_back(node_state);
    }

    return node_states_vector;
  }

  /// \brief Helper function to create a vector of edge states
  std::vector<vda5050_core::types::EdgeState> create_edge_states(std::vector<vda5050_core::edge::Edge>& edges)
  {
    std::vector<vda5050_core::types::EdgeState> edge_states_vector;

    for (vda5050_core::edge::Edge e : edges)
    {
      vda5050_core::types::EdgeState edge_state{};
      edge_state.edge_id = e.edge_id();
      edge_state.sequence_id = e.sequence_id();
      edge_state.released = e.released();

      edge_states_vector.push_back(edge_state);
    }

    return edge_states_vector;
  }
  
};

/// \brief Test if OrderManager successfully parses a new, valid order while no current order exists on the vehicle. Assumes that vehicle is ready to receive a new order.
TEST_F(OrderManagerTest, NewOrderNoCurrentOrder)
{
  bool is_new_order_accepted { orderManager.make_new_order(fully_released_order, init_state)};

  EXPECT_EQ(is_new_order_accepted, true);
}

/// \brief Test if OrderManager successfully parses a new, valid order while a current order exists on the vehicle. Assumes that vehicle is ready to receive a new order.
TEST_F(OrderManagerTest, NewOrderWithCurrentOrder)
{
  bool is_first_order_accepted { orderManager.make_new_order(fully_released_order, init_state) };

  EXPECT_EQ(is_first_order_accepted, true);

  bool is_second_order_accepted { orderManager.make_new_order(order2, fully_released_state) };

  EXPECT_EQ(is_second_order_accepted, true);
}

/// \brief Test if OrderManager rejects an order if vehicle is still executing an order
TEST_F(OrderManagerTest, NewOrderNodeStatesNotEmpty)
{
  bool is_first_order_accepted { orderManager.make_new_order(fully_released_order, init_state) };

  EXPECT_EQ(is_first_order_accepted, true);

  /// create State with non-empty NodeStates
  std::vector<vda5050_core::node::Node> unexecuted_nodes {n3, n5};
  std::vector<vda5050_core::edge::Edge> unexecuted_edges {e4};
  std::vector<vda5050_core::types::NodeState> fully_released_node_states {create_node_states(unexecuted_nodes)};
  std::vector<vda5050_core::types::EdgeState> fully_released_edge_states {create_edge_states(unexecuted_edges)};
  fully_released_state.node_states = fully_released_node_states;
  fully_released_state.edge_states = fully_released_edge_states;

  ::testing::internal::CaptureStderr();

  bool is_second_order_accepted { orderManager.make_new_order(order2, fully_released_state) };

  std::string err = ::testing::internal::GetCapturedStderr();

  EXPECT_THAT(err, HasSubstr("OrderManager error: Vehicle is not ready to accept a new order. Vehicle is either still executing or waiting for an order update."));

  EXPECT_EQ(is_second_order_accepted, false);
}

/// \brief Test if OrderManager rejects order and throws an error if vehicle still has a horizon and is waiting on an update
TEST_F(OrderManagerTest, NewOrderVehicleWaitingForUpdate)
{
  bool is_first_order_accepted { orderManager.make_new_order(partially_released_order, init_state) };

  EXPECT_EQ(is_first_order_accepted, true);

  ::testing::internal::CaptureStderr();

  bool is_second_order_accepted { orderManager.make_new_order(order2, partially_released_state)};

  std::string err = ::testing::internal::GetCapturedStderr();

  EXPECT_THAT(err, HasSubstr("OrderManager error: Vehicle is not ready to accept a new order and received order's start node is not trivially reachable."));

  EXPECT_EQ(is_second_order_accepted, false);
}

/// \brief Test if OrderManager rejects order and throws an error if the new order's first node is not trivially reachable by the vehicle
TEST_F(OrderManagerTest, NewOrderNodeNotTriviallyReachable)
{
  bool is_first_order_accepted { orderManager.make_new_order(partially_released_order, init_state) };

  EXPECT_EQ(is_first_order_accepted, true);

  /// create an order with a non-trivially reachable node
  vda5050_core::node::Node n7{7, true, "node7"};
  vda5050_core::edge::Edge e8{8, true, "edge8", "node7", "node9"};
  vda5050_core::node::Node n9{9, true, "node9"};

  std::vector<vda5050_core::node::Node> unreachableOrderNodes{n7, n9};
  std::vector<vda5050_core::edge::Edge> unreachableOrderEdges{e8};
  vda5050_core::order::Order unreachableOrder{
    "unreachableOrder", 0, unreachableOrderNodes, unreachableOrderEdges};

  ::testing::internal::CaptureStderr();

  bool is_unreachable_order_accepted { orderManager.make_new_order(unreachableOrder, fully_released_state) };

  std::string err = ::testing::internal::GetCapturedStderr();

  EXPECT_THAT(err, HasSubstr("OrderManager error: Vehicle is not ready to accept a new order and received order's start node is not trivially reachable."));

  EXPECT_EQ(is_unreachable_order_accepted, false);
}

/// \brief Test if OrderManager successfully parses a valid order update when the vehicle is ready for one
TEST_F(OrderManagerTest, BasicOrderUpdate)
{
  bool is_first_order_accepted = orderManager.make_new_order(partially_released_order, init_state);

  EXPECT_EQ(is_first_order_accepted, true);

  bool is_order_update_accepted = orderManager.update_current_order(order_update, partially_released_state);

  EXPECT_EQ(is_order_update_accepted, true);
}

/// \brief Test if OrderManager rejects order and throws an error if the order update is deprecated
TEST_F(OrderManagerTest, OrderUpdateDeprecated)
{
  bool is_first_order_accepted = orderManager.make_new_order(partially_released_order, init_state);

  EXPECT_EQ(is_first_order_accepted, true);

  bool is_order_update_accepted = orderManager.update_current_order(order_update, partially_released_state);

  EXPECT_EQ(is_order_update_accepted, true);

  std::vector<vda5050_core::node::Node> deprecated_update_nodes{n3, n5, n7};
  std::vector<vda5050_core::edge::Edge> deprecated_update_edges{e4, e6};
  vda5050_core::order::Order deprecated_update_order{
    "order1", 0, deprecated_update_nodes, deprecated_update_edges};

  ::testing::internal::CaptureStderr();

  bool is_deprecated_update_accepted = orderManager.update_current_order(deprecated_update_order, order_update_state);

  std::string err = ::testing::internal::GetCapturedStderr();

  EXPECT_THAT(err, HasSubstr("OrderManager error: Order update is deprecated."));

  EXPECT_EQ(is_deprecated_update_accepted, false);
}

/// \brief Test if OrderManager discards the order update if it is already on the vehicle
TEST_F(OrderManagerTest, OrderUpdateOnVehicle)
{
  bool is_first_order_accepted = orderManager.make_new_order(partially_released_order, init_state);

  EXPECT_EQ(is_first_order_accepted, true);

  bool is_order_update_accepted = orderManager.update_current_order(order_update, partially_released_state);

  EXPECT_EQ(is_order_update_accepted, true);

  ::testing::internal::CaptureStderr();

  bool is_duplicate_order_update_accepted = orderManager.update_current_order(order_update, order_update_state);

  std::string err = ::testing::internal::GetCapturedStderr();

  EXPECT_THAT(
    err, HasSubstr("OrderManager warning: Received duplicate order update"));

  EXPECT_EQ(is_duplicate_order_update_accepted, false);
}

/// \brief Test if OrderManager rejects order and throws an error if the update order is not a valid continuation of a previous order
TEST_F(OrderManagerTest, OrderUpdateInvalidContinuationOfCurrentOrder)
{
  bool is_first_order_accepted = orderManager.make_new_order(partially_released_order, init_state);

  EXPECT_EQ(is_first_order_accepted, true);

  /// Create an orderUpdate that is not a valid continuation from partially_released_order
  std::vector<vda5050_core::node::Node> invalid_continuation_nodes{n5, n7};
  std::vector<vda5050_core::edge::Edge> invalid_continuation_edges{e6};
  vda5050_core::order::Order invalid_continuation{
    "order1", 1, invalid_continuation_nodes, invalid_continuation_edges};

  ::testing::internal::CaptureStderr();

  bool is_invalid_order_update_accepted = orderManager.update_current_order(invalid_continuation, partially_released_state);

  std::string err = ::testing::internal::GetCapturedStderr();

  EXPECT_THAT(err, HasSubstr("OrderManager error: Order update rejected as it is not a valid continuation of the previously completed order."));

  EXPECT_EQ(is_invalid_order_update_accepted, false);
}

/// \brief Test if OrderManager returns the graph elements from the base of an order correctly
TEST_F(OrderManagerTest, GetNextGraphElement)
{
  orderManager.make_new_order(partially_released_order, init_state);

  std::optional<vda5050_core::order_graph_element::OrderGraphElement> ge1 =
    orderManager.next_graph_element();
  EXPECT_EQ(ge1->sequence_id(), n1.sequence_id());

  std::optional<vda5050_core::order_graph_element::OrderGraphElement> ge2 =
    orderManager.next_graph_element();
  EXPECT_EQ(ge2->sequence_id(), e2.sequence_id());

  std::optional<vda5050_core::order_graph_element::OrderGraphElement> ge3 =
    orderManager.next_graph_element();
  EXPECT_EQ(ge3->sequence_id(), n3.sequence_id());

  std::optional<vda5050_core::order_graph_element::OrderGraphElement>
    nullEelment = orderManager.next_graph_element();
  EXPECT_EQ(nullEelment, std::nullopt);
}
