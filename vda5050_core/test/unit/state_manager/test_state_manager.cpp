/*
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
#include "vda5050_core/state_manager/state_manager.hpp"

using namespace vda5050_core::state_manager;
using namespace vda5050_core::types;

class StateManagerTest : public ::testing::Test
{
protected:
  StateManager sm;
};

TEST_F(StateManagerTest, SetAndGetAgvPosition)
{
  AgvPosition pos;
  pos.x = 1.23;
  pos.y = 4.56;
  pos.theta = 1.57;

  sm.set_agv_position(pos);
  auto result = sm.get_agv_position();

  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->x, 1.23);
  EXPECT_DOUBLE_EQ(result->y, 4.56);
  EXPECT_DOUBLE_EQ(result->theta, 1.57);
}

TEST_F(StateManagerTest, SetAndGetVelocity)
{
  Velocity vel;
  vel.vx = 0.5;
  vel.vy = 0.1;
  vel.omega = 0.05;

  sm.set_velocity(vel);
  auto result = sm.get_velocity();

  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result->vx.value(), 0.5);
  EXPECT_DOUBLE_EQ(result->vy.value(), 0.1);
  EXPECT_DOUBLE_EQ(result->omega.value(), 0.05);
}

TEST_F(StateManagerTest, SetDrivingFlag)
{
  EXPECT_FALSE(sm.set_driving(false));  // initially false
  EXPECT_TRUE(sm.set_driving(true));    // changed to true
  EXPECT_FALSE(sm.set_driving(true));   // no change
}

TEST_F(StateManagerTest, AddAndRemoveLoad)
{
  Load load;
  load.load_id = "test_load";

  EXPECT_TRUE(sm.add_load(load));

  const auto& loads = sm.get_loads();
  ASSERT_EQ(loads.size(), 1);
  EXPECT_EQ(loads[0].load_id, "test_load");

  EXPECT_TRUE(sm.remove_load("test_load"));
  EXPECT_TRUE(sm.get_loads().empty());
}

TEST_F(StateManagerTest, SetAndGetOperatingMode)
{
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::AUTOMATIC);

  EXPECT_FALSE(sm.set_operating_mode(OperatingMode::AUTOMATIC));
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::AUTOMATIC);

  EXPECT_TRUE(sm.set_operating_mode(OperatingMode::MANUAL));
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::MANUAL);

  EXPECT_TRUE(sm.set_operating_mode(OperatingMode::SERVICE));
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::SERVICE);

  EXPECT_FALSE(sm.set_operating_mode(OperatingMode::SERVICE));
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::SERVICE);
}
TEST_F(StateManagerTest, SetBatteryState)
{
  BatteryState battery;
  battery.battery_charge = 0.8;
  battery.battery_voltage = 48.2;

  sm.set_battery_state(battery);
  const auto& b = sm.get_battery_state();
  EXPECT_DOUBLE_EQ(b.battery_charge, 0.8);
  EXPECT_DOUBLE_EQ(b.battery_voltage.value(), 48.2);
}

TEST_F(StateManagerTest, SetSafetyState)
{
  SafetyState s;
  s.e_stop = EStop::AUTOACK;

  EXPECT_TRUE(sm.set_safety_state(s));

  SafetyState current = sm.get_safety_state();
  EXPECT_EQ(current.e_stop, EStop::AUTOACK);

  s.e_stop = EStop::NONE;
  EXPECT_TRUE(sm.set_safety_state(s));
  current = sm.get_safety_state();
  EXPECT_EQ(current.e_stop, EStop::NONE);
}

TEST_F(StateManagerTest, AddErrorAndInfo)
{
  Error e;
  e.error_description = "Test Error";
  EXPECT_TRUE(sm.add_error(e));

  auto errors = sm.get_errors();
  ASSERT_EQ(errors.size(), 1);
  EXPECT_EQ(errors[0].error_description, "Test Error");

  Info i;
  i.info_description = "Test Info";
  sm.add_info(i);

  SUCCEED();  // no getter yet for info, just ensure no crash
}

TEST_F(StateManagerTest, RequestNewBase)
{
  sm.request_new_base();
  SUCCEED();  // ensure no crash
}

TEST_F(StateManagerTest, DumpState)
{
  AgvPosition pos;
  pos.x = 1.0;
  pos.y = 2.0;
  pos.theta = 3.14;

  sm.set_agv_position(pos);

  State state;
  sm.dump_to(state);

  EXPECT_NEAR(state.agv_position.value().x, 1.0, 1e-6);
  EXPECT_NEAR(state.agv_position.value().y, 2.0, 1e-6);
  EXPECT_NEAR(state.agv_position.value().theta, 3.14, 1e-6);
}
TEST_F(StateManagerTest, SetGetAndResetDistanceSinceLastNode)
{
  sm.set_distance_since_last_node(12.34);
  auto d1 = sm.get_distance_since_last_node();
  ASSERT_TRUE(d1.has_value());
  EXPECT_DOUBLE_EQ(*d1, 12.34);

  sm.set_distance_since_last_node(5.0);
  auto d2 = sm.get_distance_since_last_node();
  ASSERT_TRUE(d2.has_value());
  EXPECT_DOUBLE_EQ(*d2, 5.0);

  sm.reset_distance_since_last_node();
  auto d3 = sm.get_distance_since_last_node();
  EXPECT_FALSE(d3.has_value());
}

TEST_F(StateManagerTest, GetLastNodeIdAndSequenceId)
{
  EXPECT_EQ(sm.get_last_node_id(), "");
  EXPECT_EQ(sm.get_last_node_sequence_id(), 0u);
}

TEST_F(StateManagerTest, NodeStatesEmpty)
{
  EXPECT_TRUE(sm.is_node_states_empty());
}

TEST_F(StateManagerTest, AreActionStatesStillExecuting)
{
  EXPECT_FALSE(sm.are_action_states_still_executing());
}

TEST_F(StateManagerTest, CleanupPreviousOrder)
{
  Order order;
  order.order_id = "O1";
  Node node;
  node.node_id = "n1";
  node.sequence_id = 1;
  order.nodes.push_back(node);
  sm.set_new_order(order);
  sm.cleanup_previous_order();

  State state;
  sm.dump_to(state);
  EXPECT_TRUE(state.node_states.empty());
  EXPECT_TRUE(state.edge_states.empty());
  EXPECT_TRUE(state.action_states.empty());
  EXPECT_EQ(state.order_id, "");
  EXPECT_EQ(state.order_update_id, 0u);
}

TEST_F(StateManagerTest, SetNewOrder)
{
  Order order;
  order.order_id = "O1";
  order.order_update_id = 1;
  Node node;
  node.node_id = "n1";
  node.sequence_id = 1;
  Edge edge;
  edge.edge_id = "e1";
  edge.sequence_id = 2;
  order.nodes.push_back(node);
  order.edges.push_back(edge);

  sm.set_new_order(order);
  State state;
  sm.dump_to(state);

  EXPECT_EQ(state.order_id, "O1");
  ASSERT_EQ(state.node_states.size(), 1u);
  EXPECT_EQ(state.node_states[0].node_id, "n1");
  ASSERT_EQ(state.edge_states.size(), 1u);
  EXPECT_EQ(state.edge_states[0].edge_id, "e1");
}

TEST_F(StateManagerTest, ClearHorizon)
{
  Order order;
  order.order_id = "O2";
  Node n1;
  n1.node_id = "n1";
  n1.sequence_id = 1;
  n1.released = true;
  Node n2;
  n2.node_id = "n2";
  n2.sequence_id = 2;
  n2.released = false;
  Edge e1;
  e1.edge_id = "e1";
  e1.sequence_id = 3;
  e1.released = false;
  order.nodes = {n1, n2};
  order.edges = {e1};

  sm.set_new_order(order);
  sm.clear_horizon();

  State state;
  sm.dump_to(state);
  ASSERT_EQ(state.node_states.size(), 1u);
  EXPECT_EQ(state.node_states[0].node_id, "n1");
  EXPECT_TRUE(state.edge_states.empty());
}

TEST_F(StateManagerTest, AppendStatesForUpdate)
{
  Order order1;
  order1.order_id = "O1";
  Node n1;
  n1.node_id = "n1";
  n1.sequence_id = 1;
  order1.nodes.push_back(n1);
  sm.set_new_order(order1);

  Order order2;
  order2.order_id = "O2";
  Node n2;
  n2.node_id = "n2";
  n2.sequence_id = 2;
  Edge e2;
  e2.edge_id = "e2";
  e2.sequence_id = 3;
  order2.nodes.push_back(n2);
  order2.edges.push_back(e2);
  sm.append_states_for_update(order2);

  State state;
  sm.dump_to(state);
  EXPECT_EQ(state.order_id, "O2");
  ASSERT_EQ(state.node_states.size(), 1u);
  EXPECT_EQ(state.node_states[0].node_id, "n2");
  ASSERT_EQ(state.edge_states.size(), 1u);
  EXPECT_EQ(state.edge_states[0].edge_id, "e2");
}