/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
 * Advanced Remanufacturing and Technology Centre
 * A*STAR Research Entities (Co. Registration No. 199702110H)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>

#include <atomic>
#include <memory>
#include <stdexcept>

#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/client/updates/order_context.hpp"
#include "vda5050_core/types/order.hpp"

namespace {

using HeaderConfigResource = vda5050_core::client::HeaderConfigResource;
using OrderContext = vda5050_core::client::OrderContext;
using OrderExecutionResource = vda5050_core::client::OrderExecutionResource;
using OrderUpdate = vda5050_core::client::OrderUpdate;

namespace execution = vda5050_core::execution;
namespace types = vda5050_core::types;

std::shared_ptr<OrderContext> make_context()
{
  auto config =
    std::make_shared<HeaderConfigResource>("uagv", "2.0.0", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();
  return context;
}

// Test 1: Make sure cannot create context without config.
TEST(OrderContextTest, ThrowsForNullConfig)
{
  EXPECT_THROW(std::make_shared<OrderContext>(nullptr), std::invalid_argument);
}

// Test 2: Make sure robot identity metadata matched what has passed in.
TEST(OrderContextTest, SeedsHeaderConfigResource)
{
  auto context = make_context();

  auto config = context->get_resource<HeaderConfigResource>();
  ASSERT_NE(config, nullptr);
  EXPECT_EQ(config->interface_name, "uagv");
  EXPECT_EQ(config->version, "2.0.0");
  EXPECT_EQ(config->manufacturer, "ROS-I");
  EXPECT_EQ(config->serial_number, "S001");
}

// Test 3: Ensure the robot starts in a clean state.
TEST(OrderContextTest, SeedsOrderExecutionResource)
{
  auto context = make_context();

  auto execution = context->get_resource<OrderExecutionResource>();
  ASSERT_NE(execution, nullptr);
  EXPECT_FALSE(execution->executing_order);
  EXPECT_FALSE(execution->awaiting_order_update);
  EXPECT_TRUE(execution->state.order_id.empty());
  EXPECT_EQ(execution->state.order_update_id, 0u);
  EXPECT_TRUE(execution->state.last_node_id.empty());
  EXPECT_EQ(execution->state.last_node_sequence_id, 0u);
  EXPECT_TRUE(execution->state.node_states.empty());
  EXPECT_TRUE(execution->state.edge_states.empty());
  EXPECT_TRUE(execution->state.action_states.empty());
}

// Test 4: Make sure get_resource() always return same object.
TEST(OrderContextTest, OrderExecutionResourceIsSharedAcrossLookups)
{
  auto context = make_context();

  auto a = context->get_resource<OrderExecutionResource>();
  a->executing_order = true;
  a->state.order_id = "order_1";
  a->state.order_update_id = 2;
  a->state.last_node_id = "node_a";
  a->state.last_node_sequence_id = 4;

  auto b = context->get_resource<OrderExecutionResource>();
  ASSERT_EQ(a.get(), b.get());
  EXPECT_TRUE(b->executing_order);
  EXPECT_EQ(b->state.order_id, "order_1");
  EXPECT_EQ(b->state.order_update_id, 2u);
  EXPECT_EQ(b->state.last_node_id, "node_a");
  EXPECT_EQ(b->state.last_node_sequence_id, 4u);
}

// Test 5: Make sure the incoming order is received and stored in context.
TEST(OrderContextTest, CachesOrderUpdate)
{
  auto context = make_context();

  types::Order order;
  order.order_id = "order_42";
  order.order_update_id = 7;
  context->provider()->push<OrderUpdate>(order);

  auto retrieved = context->get_update<OrderUpdate>();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->order.order_id, "order_42");
  EXPECT_EQ(retrieved->order.order_update_id, 7u);
}

// Test 6: Check that the latest OrderUpdate will overwrite the previous one.
TEST(OrderContextTest, ReplacesOrderUpdateWithLatest)
{
  auto context = make_context();

  types::Order order1;
  order1.order_id = "order_1";
  order1.order_update_id = 1;
  context->provider()->push<OrderUpdate>(order1);

  types::Order order2;
  order2.order_id = "order_2";
  order2.order_update_id = 2;
  context->provider()->push<OrderUpdate>(order2);

  auto retrieved = context->get_update<OrderUpdate>();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->order.order_id, "order_2");
  EXPECT_EQ(retrieved->order.order_update_id, 2u);
}

// Test 7: Make sure no crash before any update arrives.
TEST(OrderContextTest, ReturnsNullptrForUnpushedUpdate)
{
  auto context = make_context();
  EXPECT_EQ(context->get_update<OrderUpdate>(), nullptr);
}

// Test 8: Make sure on_change fires once per OrderUpdate push.
TEST(OrderContextTest, InvokesChangeCallbackOnOrderUpdate)
{
  auto config =
    std::make_shared<HeaderConfigResource>("uagv", "2.0.0", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);

  std::atomic_int count = 0;
  context->on_change([&count]() { count++; });
  context->init();

  types::Order order;
  order.order_id = "order_1";
  context->provider()->push<OrderUpdate>(order);

  EXPECT_EQ(count, 1);
}

// Test 9: Make sure each update triggers the event separately.
TEST(OrderContextTest, MultipleUpdatesInvokeCallbackMultipleTimes)
{
  auto config =
    std::make_shared<HeaderConfigResource>("uagv", "2.0.0", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);

  std::atomic_int count = 0;
  context->on_change([&count]() { count++; });
  context->init();

  types::Order order1;
  order1.order_id = "order_1";
  context->provider()->push<OrderUpdate>(order1);

  types::Order order2;
  order2.order_id = "order_2";
  context->provider()->push<OrderUpdate>(order2);

  EXPECT_EQ(count, 2);
}

// Test 10: Make sure get_resource returns nullptr for an unknown type.
TEST(OrderContextTest, ReturnsNullptrForUnknownResourceType)
{
  struct UnknownResource
  : public execution::Initialize<UnknownResource, execution::ResourceBase>
  {};

  auto context = make_context();
  EXPECT_EQ(context->get_resource<UnknownResource>(), nullptr);
}

// Test 11: Ensure get_active_order_id and update_id fetch from OrderExecutionResource.
TEST(OrderContextTest, BridgeGettersFetchFromExecutionResource)
{
  auto context = make_context();
  auto execution = context->get_resource<OrderExecutionResource>();

  // Simulate a strategy updating active order details
  {
    std::lock_guard<std::mutex> lock(execution->mutex_);
    execution->state.order_id = "order_abc";
    execution->state.order_update_id = 99;
  }

  // Verify context methods extract them directly
  EXPECT_EQ(context->get_active_order_id(), "order_abc");
  EXPECT_EQ(context->get_active_order_update_id(), 99u);
}

// Test 12: Ensure get_node_states returns correct vector.
TEST(OrderContextTest, BridgeGettersExtractNodeStates)
{
  auto context = make_context();
  auto execution = context->get_resource<OrderExecutionResource>();

  // Simulate an updated node state track array
  types::NodeState node_a;
  node_a.node_id = "node_a";
  node_a.sequence_id = 0;
  node_a.released = true;

  {
    std::lock_guard<std::mutex> lock(execution->mutex_);
    execution->state.node_states.push_back(node_a);
  }

  auto retrieved_nodes = context->get_node_states();
  ASSERT_EQ(retrieved_nodes.size(), 1u);
  EXPECT_EQ(retrieved_nodes[0].node_id, "node_a");
  EXPECT_TRUE(retrieved_nodes[0].released);
}

// Test 13: Ensure get_edge_states successfully bridges vectors from the state object.
TEST(OrderContextTest, BridgeGettersExtractEdgeStates)
{
  auto context = make_context();
  auto execution = context->get_resource<OrderExecutionResource>();

  // Simulate an updated edge state track array
  types::EdgeState edge_ab;
  edge_ab.edge_id = "edge_ab";
  edge_ab.sequence_id = 1;
  edge_ab.released = false;

  {
    std::lock_guard<std::mutex> lock(execution->mutex_);
    execution->state.edge_states.push_back(edge_ab);
  }

  auto retrieved_edges = context->get_edge_states();
  ASSERT_EQ(retrieved_edges.size(), 1u);
  EXPECT_EQ(retrieved_edges[0].edge_id, "edge_ab");
  EXPECT_FALSE(retrieved_edges[0].released);
}

// Test 14: Check whether shortcut getters return empty/default values
// if execution resource isn't found
TEST(OrderContextTest, BridgeGettersReturnDefaultsWhenMissing)
{
  // A context created without running init() won't have execution cached
  auto config =
    std::make_shared<HeaderConfigResource>("uagv", "2.0.0", "ROS-I", "S001");
  OrderContext raw_context(config);

  EXPECT_EQ(raw_context.get_active_order_id(), "");
  EXPECT_EQ(raw_context.get_active_order_update_id(), 0u);
  EXPECT_TRUE(raw_context.get_node_states().empty());
}

}  // namespace
