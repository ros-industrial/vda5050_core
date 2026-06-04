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

#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/strategies/order_acceptance.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/client/updates/order_context.hpp"
#include "vda5050_core/types/order.hpp"

namespace {

using OrderAcceptance = vda5050_core::client::OrderAcceptance;
using OrderContext = vda5050_core::client::OrderContext;
using OrderExecutionResource = vda5050_core::client::OrderExecutionResource;
using OrderUpdate = vda5050_core::client::OrderUpdate;
namespace execution = vda5050_core::execution;
namespace types = vda5050_core::types;

std::shared_ptr<OrderContext> make_context()
{
  auto config = std::make_shared<vda5050_core::client::HeaderConfigResource>(
    "uagv", "2.0.0", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();
  return context;
}

types::Node make_node(const std::string& node_id, uint32_t sequence_id)
{
  types::Node node;
  node.node_id = node_id;
  node.sequence_id = sequence_id;
  node.released = true;
  return node;
}

types::Edge make_edge(
  const std::string& edge_id, uint32_t sequence_id,
  const std::string& start_node_id, const std::string& end_node_id)
{
  types::Edge edge;
  edge.edge_id = edge_id;
  edge.sequence_id = sequence_id;
  edge.start_node_id = start_node_id;
  edge.end_node_id = end_node_id;
  edge.released = true;
  return edge;
}

types::Action make_action(const std::string& action_id)
{
  types::Action action;
  action.action_id = action_id;
  action.action_type = "pick";
  return action;
}

// Test 1: A valid new order populates the execution state and marks executing.
TEST(OrderAcceptanceTest, AcceptsAndPopulatesNewOrder)
{
  OrderAcceptance strategy;
  auto context = make_context();

  types::Order order;
  order.order_id = "o1";
  order.order_update_id = 0;
  auto node0 = make_node("node_0", 0);
  node0.actions.push_back(make_action("act_0"));
  order.nodes.push_back(node0);
  order.nodes.push_back(make_node("node_2", 2));
  order.edges.push_back(make_edge("edge_1", 1, "node_0", "node_2"));

  context->provider()->push<OrderUpdate>(order);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  std::lock_guard<std::mutex> lock(execution->mutex_);
  EXPECT_TRUE(execution->executing_order);
  EXPECT_EQ(execution->state.order_id, "o1");
  EXPECT_EQ(execution->state.order_update_id, 0u);
  EXPECT_EQ(execution->state.node_states.size(), 2u);
  EXPECT_EQ(execution->state.edge_states.size(), 1u);
  EXPECT_EQ(execution->state.action_states.size(), 1u);
  EXPECT_TRUE(execution->state.errors.empty());
}

// Test 2: A structurally invalid order is rejected; errors are recorded and the
// execution state is left untouched.
TEST(OrderAcceptanceTest, RejectsInvalidOrderAndRecordsErrors)
{
  OrderAcceptance strategy;
  auto context = make_context();

  types::Order order;
  order.order_id = "bad";
  order.order_update_id = 0;
  order.nodes.push_back(make_node("node_0", 0));
  order.nodes.push_back(make_node("node_2", 2));  // 2 nodes, 0 edges -> invalid

  context->provider()->push<OrderUpdate>(order);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  std::lock_guard<std::mutex> lock(execution->mutex_);
  EXPECT_FALSE(execution->executing_order);
  EXPECT_TRUE(execution->state.order_id.empty());
  EXPECT_TRUE(execution->state.node_states.empty());
  EXPECT_FALSE(execution->state.errors.empty());
}

// Test 3: A valid update appends the new node onto the running base.
TEST(OrderAcceptanceTest, AppliesUpdateByAppending)
{
  OrderAcceptance strategy;
  auto context = make_context();

  // Seed a running base: order "o1" update 1, sitting at node_a (seq 10).
  {
    auto execution = context->get_resource<OrderExecutionResource>();
    std::lock_guard<std::mutex> lock(execution->mutex_);
    execution->executing_order = true;
    execution->state.order_id = "o1";
    execution->state.order_update_id = 1;
    execution->state.last_node_id = "node_a";
    execution->state.last_node_sequence_id = 10;
    execution->state.node_states.push_back([] {
      types::NodeState n;
      n.node_id = "node_a";
      n.sequence_id = 10;
      n.released = true;
      return n;
    }());
  }

  types::Order update;
  update.order_id = "o1";
  update.order_update_id = 2;
  update.nodes.push_back(make_node("node_a", 10));  // re-sent stitch node
  update.nodes.push_back(make_node("node_b", 12));
  update.edges.push_back(make_edge("edge_ab", 11, "node_a", "node_b"));

  context->provider()->push<OrderUpdate>(update);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  std::lock_guard<std::mutex> lock(execution->mutex_);
  EXPECT_EQ(execution->state.order_update_id, 2u);
  ASSERT_EQ(execution->state.node_states.size(), 2u);
  EXPECT_EQ(execution->state.node_states[0].node_id, "node_a");
  EXPECT_EQ(execution->state.node_states[1].node_id, "node_b");
  EXPECT_EQ(execution->state.edge_states.size(), 1u);
}

// Test 4: A duplicate update (same orderUpdateId) leaves the state untouched.
TEST(OrderAcceptanceTest, IgnoresDuplicateUpdate)
{
  OrderAcceptance strategy;
  auto context = make_context();

  {
    auto execution = context->get_resource<OrderExecutionResource>();
    std::lock_guard<std::mutex> lock(execution->mutex_);
    execution->state.order_id = "o1";
    execution->state.order_update_id = 3;
    execution->state.last_node_id = "node_a";
    execution->state.last_node_sequence_id = 10;
  }

  types::Order duplicate;
  duplicate.order_id = "o1";
  duplicate.order_update_id = 3;
  duplicate.nodes.push_back(make_node("node_a", 10));

  context->provider()->push<OrderUpdate>(duplicate);
  strategy.step(context);

  auto execution = context->get_resource<OrderExecutionResource>();
  std::lock_guard<std::mutex> lock(execution->mutex_);
  EXPECT_EQ(execution->state.order_update_id, 3u);
  EXPECT_TRUE(execution->state.node_states.empty());
  EXPECT_TRUE(execution->state.errors.empty());
}

}  // namespace
