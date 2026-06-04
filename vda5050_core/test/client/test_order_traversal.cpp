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

#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <vector>

#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/client/events/navigate_to_node.hpp"
#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/strategies/order_traversal.hpp"
#include "vda5050_core/client/updates/node_reached.hpp"

namespace {

using NavigateToNodeEvent = vda5050_core::client::NavigateToNodeEvent;
using NodeReachedUpdate = vda5050_core::client::NodeReachedUpdate;
using AGVContext = vda5050_core::client::AGVContext;
using OrderExecutionResource = vda5050_core::client::OrderExecutionResource;
using OrderTraversal = vda5050_core::client::OrderTraversal;
namespace types = vda5050_core::types;

std::shared_ptr<AGVContext> make_context()
{
  auto config = std::make_shared<vda5050_core::client::HeaderConfigResource>(
    "uagv", "2.0.0", "ROS-I", "S001");
  auto context = AGVContext::make(config);
  context->init();
  return context;
}

types::NodeState node_state(
  const std::string& node_id, uint32_t sequence_id, bool released)
{
  types::NodeState n;
  n.node_id = node_id;
  n.sequence_id = sequence_id;
  n.released = released;
  return n;
}

types::EdgeState edge_state(const std::string& edge_id, uint32_t sequence_id)
{
  types::EdgeState e;
  e.edge_id = edge_id;
  e.sequence_id = sequence_id;
  e.released = true;
  return e;
}

// Seed the execution state with nodes and edges.
void seed(
  const std::shared_ptr<AGVContext>& context,
  std::vector<types::NodeState> nodes, std::vector<types::EdgeState> edges)
{
  auto execution = context->get_resource<OrderExecutionResource>();
  types::State state = execution->get_state();
  state.order_id = "o1";
  state.node_states = std::move(nodes);
  state.edge_states = std::move(edges);
  execution->set_state(std::move(state));
}

// Test 1: Reaching a node drops its state + incoming edge and sets lastNode.
TEST(OrderTraversalTest, AdvancesStateOnNodeReached)
{
  OrderTraversal strategy;
  auto context = make_context();
  // node_0 already traversed; sitting on edge e1 toward node_2.
  // node_2, node_4 are in the base.
  // e1 is the incoming edge to node_2.
  // e3 is the outgoing edge from node_2.
  seed(
    context, {node_state("node_2", 2, true), node_state("node_4", 4, true)},
    {edge_state("e1", 1), edge_state("e3", 3)});

  // node_2 is reached.
  context->provider()->push<NodeReachedUpdate>("node_2", 2);
  // Step the strategy.
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();

  // Snapshot the execution state to read it.
  const auto state = execution->get_state();
  EXPECT_EQ(state.last_node_id, "node_2");
  EXPECT_EQ(state.last_node_sequence_id, 2u);

  // The reached node is removed from the pending traversal state,
  // leaving only the remaining base node ahead.
  ASSERT_EQ(state.node_states.size(), 1u);
  EXPECT_EQ(state.node_states.front().node_id, "node_4");

  // The incoming edge is removed from the pending traversal state,
  // leaving only the outgoing edge.
  ASSERT_EQ(state.edge_states.size(), 1u);
  EXPECT_EQ(state.edge_states.front().sequence_id, 3u);  // e1 dropped
}

// Test 2: An unknown node-reached signal leaves the state untouched.
TEST(OrderTraversalTest, IgnoresUnknownNode)
{
  OrderTraversal strategy;
  auto context = make_context();
  seed(
    context, {node_state("node_2", 2, true), node_state("node_4", 4, true)},
    {edge_state("e1", 1), edge_state("e3", 3)});

  // Pushed an unknown node-reached signal.
  context->provider()->push<NodeReachedUpdate>("ghost", 99);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  const auto state = execution->get_state();
  EXPECT_EQ(state.node_states.size(), 2u);
  EXPECT_TRUE(state.last_node_id.empty());
}

// Test 3: Reaching the last released node before a horizon stops traversal and
// raises new_base_request.
TEST(OrderTraversalTest, StopsAtDecisionPoint)
{
  OrderTraversal strategy;
  auto context = make_context();
  // node_2 is the last released base node; node_4 is horizon.
  seed(
    context, {node_state("node_2", 2, true), node_state("node_4", 4, false)},
    {edge_state("e3", 3)});

  context->provider()->push<NodeReachedUpdate>("node_2", 2);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  const auto state = execution->get_state();
  EXPECT_EQ(state.last_node_id, "node_2");
  ASSERT_TRUE(state.new_base_request.has_value());
  EXPECT_TRUE(state.new_base_request.value());
  // node_4 (horizon) is kept, still unreleased.
  ASSERT_EQ(state.node_states.size(), 1u);
  EXPECT_FALSE(state.node_states.front().released);
}

// Test 4: Traversing the final node empties the base.
TEST(OrderTraversalTest, ClearsStateWhenOrderComplete)
{
  OrderTraversal strategy;
  auto context = make_context();
  seed(context, {node_state("node_4", 4, true)}, {edge_state("e3", 3)});

  context->provider()->push<NodeReachedUpdate>("node_4", 4);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  const auto state = execution->get_state();
  EXPECT_TRUE(state.node_states.empty());
  EXPECT_EQ(state.last_node_id, "node_4");
  EXPECT_EQ(state.last_node_sequence_id, 4u);
}

// Test 5: The next released node is dispatched via a NavigateToNodeEvent.
TEST(OrderTraversalTest, DispatchesNextNode)
{
  OrderTraversal strategy;
  auto context = make_context();
  seed(
    context, {node_state("node_2", 2, true), node_state("node_4", 4, true)},
    {edge_state("e1", 1), edge_state("e3", 3)});

  std::shared_ptr<NavigateToNodeEvent> dispatched;
  strategy.engine()->on<NavigateToNodeEvent>(
    [&](std::shared_ptr<NavigateToNodeEvent> event) { dispatched = event; });

  context->provider()->push<NodeReachedUpdate>("node_2", 2);
  strategy.step(context);

  ASSERT_NE(dispatched, nullptr);
  EXPECT_EQ(dispatched->target.node_id, "node_4");
  ASSERT_TRUE(dispatched->via_edge.has_value());
  EXPECT_EQ(dispatched->via_edge->sequence_id, 3u);
}

// Test 6: A node reported out of order (ahead of a still-pending node) is
// ignored so the in-between nodes are not orphaned.
TEST(OrderTraversalTest, IgnoresOutOfOrderNode)
{
  OrderTraversal strategy;
  auto context = make_context();
  seed(
    context, {node_state("node_2", 2, true), node_state("node_4", 4, true)},
    {edge_state("e1", 1), edge_state("e3", 3)});

  // Report reaching node_4 while node_2 is still pending.
  context->provider()->push<NodeReachedUpdate>("node_4", 4);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  const auto state = execution->get_state();
  EXPECT_EQ(state.node_states.size(), 2u);
  EXPECT_TRUE(state.last_node_id.empty());
}

// Test 7: new_base_request stays false while the released base is plentiful.
TEST(OrderTraversalTest, DoesNotRequestBaseWhenPlentiful)
{
  OrderTraversal strategy;
  auto context = make_context();
  seed(
    context,
    {node_state("node_2", 2, true), node_state("node_4", 4, true),
     node_state("node_6", 6, true)},
    {edge_state("e1", 1), edge_state("e3", 3), edge_state("e5", 5)});

  context->provider()->push<NodeReachedUpdate>("node_2", 2);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  const auto state = execution->get_state();
  ASSERT_TRUE(state.new_base_request.has_value());
  EXPECT_FALSE(state.new_base_request.value());
}

}  // namespace
