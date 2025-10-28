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
  // vda5050_core::node::Node n5 {5, true, "node5"};
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

  vda5050_core::order_manager::OrderManager orderManager{};

  /// Snapshot of the AGV's state if it has no existing order 
  vda5050_core::types::State init_state{};

  /// Snapshot of the AGV's state after accepting and completing fully_released_order
  vda5050_core::types::State fully_released_state{};

  /// Snapshot of the AGV's state after accepting and completing partially_released_order
  vda5050_core::types::State partially_released_state{};

  /// Snapshot of the AGV's state after accepting order_update
  vda5050_core::types::State order_update_state{};

  /// Setup function that runs before each test
  void SetUp() override
  {
    fully_released_state.order_id = "order1";
    fully_released_state.last_node_id = "node5";
    fully_released_state.last_node_sequence_id = 5;

    /// NOTE: fully_released_state is AFTER the vehicle has completed the order, so node and edge states should be empty!
    // std::vector<vda5050_core::types::NodeState> fully_released_node_states {create_node_states(fully_released_nodes)};
    // std::vector<vda5050_core::types::EdgeState> fully_released_edge_states {create_edge_states(fully_released_edges)};
    // fully_released_state.node_states = fully_released_node_states;
    // fully_released_state.edge_states = fully_released_edge_states;

    
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

// /// \brief Test if OrderManager rejects order and throws an error if vehicle is still executing an order
// TEST_F(OrderManagerTest, NewOrderNodeStatesNotEmpty)
// {
//   orderManager.make_new_order(fully_released_order);
//   EXPECT_THROW(orderManager.make_new_order(order2), std::runtime_error);
// }

// /// \brief Test if OrderManager rejects order and throws an error if vehicle still has a horizon and is waiting on an update
// TEST_F(OrderManagerTest, NewOrderVehicleWaitingForUpdate)
// {
//   orderManager.make_new_order(fully_released_order);

//   EXPECT_THROW(orderManager.make_new_order(order2), std::runtime_error);
// }

// /// \brief Test if OrderManager rejects order and throws an error if the new order's first node is not trivially reachable by the vehicle
// TEST_F(OrderManagerTest, NewOrderNodeNotTriviallyReachable)
// {
//   /// parse a valid order first
//   orderManager.make_new_order(fully_released_order);

//   /// create an order with a non-trivially reachable node
//   vda5050_core::node::Node n7{7, true, "node7"};
//   vda5050_core::edge::Edge e8{8, true, "edge8", "node7", "node9"};
//   vda5050_core::node::Node n9{9, true, "node9"};

//   std::vector<vda5050_core::node::Node> unreachableOrderNodes{n7, n9};
//   std::vector<vda5050_core::edge::Edge> unreachableOrderEdges{e8};

//   vda5050_core::order::Order unreachableOrder{
//     "unreachableOrder", 0, unreachableOrderNodes, unreachableOrderEdges};

//   EXPECT_THROW(
//     orderManager.make_new_order(unreachableOrder), std::runtime_error);
// }

// /// \brief Test if OrderManager successfully parses a valid order update when the vehicle is ready for one
// TEST_F(OrderManagerTest, NewOrderReadyForOrderUpdate)
// {
//   orderManager.make_new_order(partially_released_order);

//   EXPECT_NO_THROW(orderManager.update_current_order(order_update));
// }

// /// \brief Test if OrderManager rejects order and throws an error if the order update is deprecated
// TEST_F(OrderManagerTest, OrderUpdateDeprecated)
// {
//   orderManager.make_new_order(partially_released_order);

//   orderManager.update_current_order(order_update);

//   std::vector<vda5050_core::node::Node> deprecated_update_nodes{n3, n5, n7};
//   std::vector<vda5050_core::edge::Edge> deprecated_update_edges{e4, e6};

//   vda5050_core::order::Order deprecated_update_order{
//     "order1", 0, deprecated_update_nodes, deprecated_update_edges};

//   EXPECT_THROW(
//     orderManager.update_current_order(deprecated_update_order),
//     std::runtime_error);
// }

// /// \brief Test if OrderManager discards the order update if it is already on the vehicle
// TEST_F(OrderManagerTest, OrderUpdateOnVehicle)
// {
//   orderManager.make_new_order(partially_released_order);

//   orderManager.update_current_order(order_update);

//   ::testing::internal::CaptureStderr();

//   orderManager.update_current_order(order_update);

//   std::string err = ::testing::internal::GetCapturedStderr();
//   EXPECT_THAT(
//     err, HasSubstr("OrderManager warning: Received duplicate order update"));
// }

// /// \brief Test if OrderManager rejects order and throws an error if the update order is not a valid continuation of a previous order
// TEST_F(OrderManagerTest, OrderUpdateInvalidContinuationOfCurrentOrder)
// {
//   orderManager.make_new_order(partially_released_order);

//   std::vector<vda5050_core::node::Node> invalid_continuation_nodes{n5, n7};
//   std::vector<vda5050_core::edge::Edge> invalid_continuation_edges{e6};

//   vda5050_core::order::Order invalid_continuation{
//     "order1", 1, invalid_continuation_nodes, invalid_continuation_edges};

//   EXPECT_THROW(
//     orderManager.update_current_order(invalid_continuation),
//     std::runtime_error);
// }

// /// \brief Test if OrderManager returns the graph elements from the base of an order correctly
// TEST_F(OrderManagerTest, GetNextGraphElement)
// {
//   orderManager.make_new_order(partially_released_order);

//   std::optional<vda5050_core::order_graph_element::OrderGraphElement> ge1 =
//     orderManager.next_graph_element();
//   EXPECT_EQ(ge1->sequence_id(), n1.sequence_id());

//   std::optional<vda5050_core::order_graph_element::OrderGraphElement> ge2 =
//     orderManager.next_graph_element();
//   EXPECT_EQ(ge2->sequence_id(), e2.sequence_id());

//   std::optional<vda5050_core::order_graph_element::OrderGraphElement> ge3 =
//     orderManager.next_graph_element();
//   EXPECT_EQ(ge3->sequence_id(), n3.sequence_id());

//   std::optional<vda5050_core::order_graph_element::OrderGraphElement>
//     nullEelment = orderManager.next_graph_element();
//   EXPECT_EQ(nullEelment, std::nullopt);
// }
