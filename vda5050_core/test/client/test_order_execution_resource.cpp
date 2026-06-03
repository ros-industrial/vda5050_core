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
#include <typeindex>

#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/execution/base.hpp"

namespace {

using namespace vda5050_core::execution;  // NOLINT
using namespace vda5050_core::client;     // NOLINT

// Test 1: Ensure OrderExecutionResource correctly stores runtime execution state.
TEST(OrderExecutionResourceTest, CarriesExecutionSnapshot)
{
  OrderExecutionResource resource;
  resource.executing_order = true;
  resource.awaiting_order_update = true;
  resource.state.order_id = "order_1";
  resource.state.order_update_id = 1;
  resource.state.last_node_id = "node_0";
  resource.state.last_node_sequence_id = 0;

  EXPECT_TRUE(resource.executing_order);
  EXPECT_TRUE(resource.awaiting_order_update);
  EXPECT_EQ(resource.state.order_id, "order_1");
  EXPECT_EQ(resource.state.order_update_id, 1u);
  EXPECT_EQ(resource.state.last_node_id, "node_0");
}

// Test 2: Check OrderExecutionResource reports its type through ResourceBase.
TEST(OrderExecutionResourceTest, ReportsTypeThroughBase)
{
  std::shared_ptr<ResourceBase> base =
    std::make_shared<OrderExecutionResource>();

  EXPECT_EQ(base->get_type(), std::type_index(typeid(OrderExecutionResource)));
}

// Test 3: Make sure each resource/update type must have unique runtime type identity.
TEST(OrderExecutionResourceTest, HasDistinctTypeIndex)
{
  EXPECT_NE(
    std::type_index(typeid(OrderExecutionResource)),
    std::type_index(typeid(OrderUpdate)));

  EXPECT_NE(
    std::type_index(typeid(OrderExecutionResource)),
    std::type_index(typeid(HeaderConfigResource)));
}

}  // namespace
