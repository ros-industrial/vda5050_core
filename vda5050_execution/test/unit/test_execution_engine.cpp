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

#include <vda5050_types/edge.hpp>
#include <vda5050_types/node.hpp>

#include "vda5050_execution/event.hpp"
#include "vda5050_execution/execution_engine.hpp"

TEST(ExecutionEngineTest, DispatchUpdate)
{
  auto engine = std::make_shared<vda5050_execution::ExecutionEngine>();

  int call_count_1 = 0;
  int call_count_2 = 0;

  vda5050_types::Node node;
  node.node_id = "N1";
  vda5050_types::Edge edge;
  edge.edge_id = "E1";

  engine->on<vda5050_execution::NavigationNodeReady>(
    [&](std::shared_ptr<vda5050_execution::NavigationNodeReady> event) {
      call_count_1++;
      EXPECT_EQ(*event->target_node, node);
      EXPECT_EQ(*event->traversal_edge, edge);
    });
  engine->on<vda5050_execution::NavigationStatusChange>(
    [&](std::shared_ptr<vda5050_execution::NavigationStatusChange> event) {
      call_count_2++;
      EXPECT_TRUE(event->paused);
    });

  engine->emit<vda5050_execution::NavigationNodeReady>(
    std::make_shared<vda5050_types::Node>(node),
    std::make_shared<vda5050_types::Edge>(edge));
  engine->emit<vda5050_execution::NavigationStatusChange>(true);

  engine->step();

  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 0);

  engine->step();

  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 1);
}

TEST(ExecutionEngineTest, EmptyQueueTest)
{
  auto engine = std::make_shared<vda5050_execution::ExecutionEngine>();

  EXPECT_NO_THROW(engine->step());
}
