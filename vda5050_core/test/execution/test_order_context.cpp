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

#include <atomic>
#include <memory>

#include "vda5050_core/execution/order_context.hpp"
#include "vda5050_core/client/order_types.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"

namespace {

using namespace vda5050_core::execution;  // NOLINT
using namespace vda5050_core::client;     // NOLINT
namespace types = vda5050_core::types;

// Test 1: Initialize OrderContext with a ConfigResource and check if it can be retrieved correctly.
TEST(OrderContextTest, InitializesWithConfigResource)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  auto retrieved = context->get_resource<ConfigResource>();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->interface_name, "uagv");
  EXPECT_EQ(retrieved->version, "v2");
  EXPECT_EQ(retrieved->manufacturer, "ROS-I");
  EXPECT_EQ(retrieved->serial_number, "S001");
}

// Test 2: Push an OrderUpdate into the provider and check if it is cached and retrievable.
TEST(OrderContextTest, CachesOrderUpdate)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  types::Order order;
  order.order_id = "order_42";
  order.order_update_id = 7;

  context->provider()->push<OrderUpdate>(order);

  auto retrieved = context->get_update<OrderUpdate>();  // Retrieve the cached OrderUpdate
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->order.order_id, "order_42");
  EXPECT_EQ(retrieved->order.order_update_id, 7u);
}

// Test 3: Push a StateUpdate and check if it is cached and retrievable.
TEST(OrderContextTest, CachesStateUpdate)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  types::State state;
  state.order_id = "order_42";

  context->provider()->push<StateUpdate>(state);

  auto retrieved = context->get_update<StateUpdate>();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->state.order_id, "order_42");
}

// Test 4: Check that pushing multiple updates of the same type
// replaces the cached update with the latest one.
TEST(OrderContextTest, ReplacesOrderUpdateWithLatest)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

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
  EXPECT_EQ(retrieved->order.order_id, "order_2");  // The latest order should be cached
  EXPECT_EQ(retrieved->order.order_update_id, 2u);
}

// Test 5: Check that pushing multiple updates of the same type
// replaces the cached update with the latest one.
TEST(OrderContextTest, ReplacesStateUpdateWithLatest)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  types::State state1;
  state1.order_id = "order_old";
  context->provider()->push<StateUpdate>(state1);

  types::State state2;
  state2.order_id = "order_new";
  context->provider()->push<StateUpdate>(state2);

  auto retrieved = context->get_update<StateUpdate>();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->state.order_id, "order_new");
}

// Test 6: Check that the on_change callback is invoked when an OrderUpdate is pushed.
TEST(OrderContextTest, InvokesChangeCallbackOnOrderUpdate)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);

  std::atomic_int callback_count = 0;
  context->on_change([&callback_count]() { callback_count++; });

  context->init();

  types::Order order;
  order.order_id = "order_1";
  context->provider()->push<OrderUpdate>(order);

  EXPECT_EQ(callback_count, 1);
}

// Test 7: Check that the on_change callback is invoked when a StateUpdate is pushed.
TEST(OrderContextTest, InvokesChangeCallbackOnStateUpdate)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);

  std::atomic_int callback_count = 0;
  context->on_change([&callback_count]() { callback_count++; });

  context->init();

  types::State state;
  state.order_id = "order_trigger";
  context->provider()->push<StateUpdate>(state);

  EXPECT_EQ(callback_count, 1);
}

// Test 8: Check that multiple updates trigger the on_change callback the correct number of times.
TEST(OrderContextTest, MultipleUpdatesInvokeCallbackMultipleTimes)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);

  std::atomic_int callback_count = 0;
  context->on_change([&callback_count]() { callback_count++; });

  context->init();

  types::Order order1;
  order1.order_id = "order_1";
  context->provider()->push<OrderUpdate>(order1);

  types::State state;
  state.order_id = "order_multiple";
  context->provider()->push<StateUpdate>(state);

  types::Order order2;
  order2.order_id = "order_2";
  context->provider()->push<OrderUpdate>(order2);

  EXPECT_EQ(callback_count, 3);
}

// Test 9: Check that get_update returns nullptr for an update type that was never pushed.
TEST(OrderContextTest, ReturnsNullptrForUnknownUpdateType)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  // OrderUpdate was never pushed, so it should be nullptr
  auto retrieved = context->get_update<OrderUpdate>();
  EXPECT_EQ(retrieved, nullptr);
}

// Test 10: Check that get_resource returns nullptr for a resource type that was never pushed.
TEST(OrderContextTest, ReturnsNullptrForUnknownResourceType)
{
  // Create a resource type that is not stored
  struct UnknownResource : public Initialize<UnknownResource, ResourceBase>
  {};

  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  auto retrieved = context->get_resource<UnknownResource>();
  EXPECT_EQ(retrieved, nullptr);
}

// Test 11: Check that get_resource can retrieve the ConfigResource correctly.
TEST(OrderContextTest, RetrievesCorrectResourceType)
{
  auto config = std::make_shared<ConfigResource>("uagv", "v2", "ROS-I", "S001");
  auto context = std::make_shared<OrderContext>(config);
  context->init();

  auto retrieved = context->get_resource<ConfigResource>();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->serial_number, "S001");
}

}  // namespace
