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
#include "vda5050_core/client/state/state_manager.hpp"

using vda5050_types::ActionState;
using vda5050_types::ActionStatus;
using vda5050_types::AGVPosition;
using vda5050_types::BatteryState;
using vda5050_types::Edge;
using vda5050_types::EdgeState;
using vda5050_types::Error;
using vda5050_types::ErrorLevel;
using vda5050_types::EStop;
using vda5050_types::Header;
using vda5050_types::Info;
using vda5050_types::Load;
using vda5050_types::Node;
using vda5050_types::NodeState;
using vda5050_types::OperatingMode;
using vda5050_types::Order;
using vda5050_types::SafetyState;
using vda5050_types::State;
using vda5050_types::Velocity;

class StateManagerTest : public ::testing::Test
{
protected:
  vda5050_core::state::StateManager sm;
};

//=============================================================================
// 1. Header and Order Identification Tests
//=============================================================================

TEST_F(StateManagerTest, HeaderAndIds)
{
  // Test Header
  Header h;
  h.header_id = 999;
  sm.set_header(h);
  EXPECT_EQ(sm.get_header().header_id, 999);

  // Test Order ID
  sm.set_order_id("order_alpha");
  EXPECT_EQ(sm.get_order_id(), "order_alpha");

  // Test Order Update ID
  sm.set_order_update_id(12);
  EXPECT_EQ(sm.get_order_update_id(), 12);

  // Test Zone Set ID
  sm.set_zone_set_id("zone_beta");
  ASSERT_TRUE(sm.get_zone_set_id());
  EXPECT_EQ(sm.get_zone_set_id(), "zone_beta");
}

TEST_F(StateManagerTest, OrderIdEmpty)
{
  // Ensure order_id is empty
  sm.set_order_id("");

  sm.set_order_update_id(50);
  sm.set_zone_set_id("zone_gamma");

  EXPECT_EQ(sm.get_order_update_id(), 50);
  EXPECT_EQ(sm.get_zone_set_id(), "zone_gamma");
}

//=============================================================================
// 2. Navigation Tests
//=============================================================================

TEST_F(StateManagerTest, LastNodeData)
{
  sm.set_last_node_id("node_start");
  EXPECT_EQ(sm.get_last_node_id(), "node_start");

  sm.set_last_node_sequence_id(5);
  EXPECT_EQ(sm.get_last_node_sequence_id(), 5);
}

TEST_F(StateManagerTest, PositionAndVelocity)
{
  // Position
  AGVPosition pos;
  pos.x = 10.0;
  pos.y = 20.0;
  pos.theta = 1.57;
  pos.map_id = "map_1";

  sm.set_agv_position(pos);
  ASSERT_TRUE(sm.get_agv_position().has_value());
  EXPECT_DOUBLE_EQ(sm.get_agv_position()->x, 10.0);
  EXPECT_DOUBLE_EQ(sm.get_agv_position()->y, 20.0);
  EXPECT_DOUBLE_EQ(sm.get_agv_position()->theta, 1.57);
  EXPECT_EQ(sm.get_agv_position()->map_id, "map_1");

  // Velocity
  Velocity vel;
  vel.vx = 2.5;
  vel.omega = 0.5;

  sm.set_velocity(vel);
  ASSERT_TRUE(sm.get_velocity().has_value());
  EXPECT_DOUBLE_EQ(sm.get_velocity()->vx.value(), 2.5);
  EXPECT_DOUBLE_EQ(sm.get_velocity()->omega.value(), 0.5);
}

TEST_F(StateManagerTest, DrivingStatus)
{
  sm.set_driving_status(true);
  EXPECT_TRUE(sm.get_driving_status());

  sm.set_driving_status(false);
  EXPECT_FALSE(sm.get_driving_status());
}

TEST_F(StateManagerTest, DistanceSinceLastNode)
{
  sm.set_distance_since_last_node(12.5);
  ASSERT_TRUE(sm.get_distance_since_last_node().has_value());
  EXPECT_DOUBLE_EQ(sm.get_distance_since_last_node().value(), 12.5);

  sm.reset_distance_since_last_node();
  EXPECT_FALSE(sm.get_distance_since_last_node().has_value());
}

//=============================================================================
// 3. Load Management Tests
//=============================================================================

TEST_F(StateManagerTest, LoadOperations)
{
  Load load1;
  load1.load_id = "L1";
  load1.load_type = "pallet";

  // Test Add
  sm.add_load(load1);
  auto loads = sm.get_loads();
  ASSERT_TRUE(loads.has_value());
  EXPECT_EQ(loads->size(), 1);
  EXPECT_EQ(loads->at(0).load_id, "L1");

  // Test Add Multiple
  Load load2;
  load2.load_id = "L2";
  sm.add_load(load2);
  EXPECT_EQ(sm.get_loads()->size(), 2);

  // Test Set (Overwrite)
  Load load3;
  load3.load_id = "L3";
  sm.set_load({load3});
  EXPECT_EQ(sm.get_loads()->size(), 1);
  EXPECT_EQ(sm.get_loads()->at(0).load_id, "L3");

  // Test Remove
  EXPECT_TRUE(sm.remove_load("L3"));
  EXPECT_TRUE(sm.get_loads()->empty());

  // Test Remove non-existent
  EXPECT_FALSE(sm.remove_load("ghost_load"));
}

//=============================================================================
// 4. System Status Tests
//=============================================================================

TEST_F(StateManagerTest, OperatingModeAndBattery)
{
  sm.set_operating_mode(OperatingMode::SEMIAUTOMATIC);
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::SEMIAUTOMATIC);

  BatteryState bat;
  bat.battery_charge = 0.95;
  bat.charging = true;
  sm.set_battery_state(bat);

  EXPECT_DOUBLE_EQ(sm.get_battery_state().battery_charge, 0.95);
  EXPECT_TRUE(sm.get_battery_state().charging);
}

TEST_F(StateManagerTest, SafetyAndNewBase)
{
  SafetyState safe;
  safe.e_stop = EStop::AUTOACK;
  safe.field_violation = true;
  sm.set_safety_state(safe);

  EXPECT_EQ(sm.get_safety_state().e_stop, EStop::AUTOACK);
  EXPECT_TRUE(sm.get_safety_state().field_violation);

  // New Base Request
  sm.set_request_new_base(true);
  State s;
  sm.dump_to(s);
  EXPECT_TRUE(s.new_base_request.value());

  sm.set_request_new_base(false);
  sm.dump_to(s);
  EXPECT_FALSE(s.new_base_request.value());
}

//=============================================================================
// 5. Error and Info Tests
//=============================================================================

TEST_F(StateManagerTest, ErrorHandling)
{
  Error e1;
  e1.error_type = "Hardware";
  e1.error_level = ErrorLevel::FATAL;
  Error e2;
  e2.error_type = "Software";
  e2.error_level = ErrorLevel::WARNING;

  sm.add_error(e1);
  sm.add_error(e2);

  auto errors = sm.get_errors();
  ASSERT_EQ(errors.size(), 2);
  EXPECT_EQ(errors[0].error_type, "Hardware");
  EXPECT_EQ(errors[1].error_type, "Software");

  // Test Clear
  sm.clear_errors();
  EXPECT_TRUE(sm.get_errors().empty());
}

TEST_F(StateManagerTest, InfoHandling)
{
  Info i1;
  i1.info_type = "Status";
  i1.info_description = "All Good";

  sm.add_info(i1);

  auto infos = sm.get_info();
  ASSERT_TRUE(infos.has_value());
  EXPECT_EQ(infos->size(), 1);
  EXPECT_EQ(infos->at(0).info_type, "Status");

  // Test Clear
  sm.clear_info();
  EXPECT_FALSE(sm.get_info().has_value());
}

//=============================================================================
// 6. Action States Tests
//=============================================================================

TEST_F(StateManagerTest, ActionStateManagement)
{
  ActionState as1;
  as1.action_id = "a1";
  as1.action_status = ActionStatus::WAITING;

  ActionState as2;
  as2.action_id = "a2";
  as2.action_status = ActionStatus::RUNNING;

  std::vector<ActionState> actions = {as1, as2};

  sm.set_action_states(actions);

  auto ret = sm.get_action_states();
  ASSERT_EQ(ret.size(), 2);
  EXPECT_EQ(ret[0].action_id, "a1");
  EXPECT_EQ(ret[1].action_id, "a2");
}

TEST_F(StateManagerTest, AreActionsStillExecuting)
{
  // If no actions are executing -> return false
  EXPECT_FALSE(sm.are_action_states_still_executing());

  // If any action is NOT FINISHED and NOT FAILED -> return fralse
  ActionState active;
  active.action_status = ActionStatus::RUNNING;
  sm.set_action_states({active});
  EXPECT_FALSE(sm.are_action_states_still_executing());

  ActionState waiting;
  waiting.action_status = ActionStatus::WAITING;
  sm.set_action_states({waiting});
  EXPECT_FALSE(sm.are_action_states_still_executing());

  // If all are FINISHED or FAILED -> return True
  ActionState done1;
  done1.action_status = ActionStatus::FINISHED;
  ActionState done2;
  done2.action_status = ActionStatus::FAILED;
  sm.set_action_states({done1, done2});
  EXPECT_TRUE(sm.are_action_states_still_executing());

  // If an action is still active
  sm.set_action_states({done1, done2, active});
  EXPECT_FALSE(sm.are_action_states_still_executing());
}

//=============================================================================
// 7. Order Lifecycle Tests
//=============================================================================

TEST_F(StateManagerTest, SetNewOrder)
{
  Order o;
  o.order_id = "O1";
  o.order_update_id = 0;
  o.zone_set_id = "Z1";

  Node n1;
  n1.node_id = "n1";
  n1.released = true;
  Node n2;
  n2.node_id = "n2";
  n2.released = false;
  o.nodes = {n1, n2};

  Edge e1;
  e1.edge_id = "e1";
  e1.released = true;
  o.edges = {e1};

  sm.set_new_order(o);

  EXPECT_EQ(sm.get_order_id(), "O1");
  EXPECT_FALSE(sm.is_node_states_empty());

  State s;
  sm.dump_to(s);
  EXPECT_EQ(s.node_states.size(), 2);
  EXPECT_EQ(s.edge_states.size(), 1);
}

TEST_F(StateManagerTest, CleanupPreviousOrder)
{
  sm.set_last_node_id("keep_me");
  sm.set_order_id("delete_me");
  sm.set_driving_status(true);

  sm.cleanup_previous_order();

  // Order ID should be gone
  EXPECT_TRUE(sm.get_order_id().empty());

  // Driving status should be reset (false is default in State constructor usually)
  EXPECT_TRUE(sm.get_driving_status());

  // Last Node ID MUST persist
  EXPECT_EQ(sm.get_last_node_id(), "keep_me");
}

TEST_F(StateManagerTest, ClearHorizon)
{
  // Setup order with released and unreleased elements
  Order o;
  Node n1;
  n1.node_id = "rel_n";
  n1.released = true;
  Node n2;
  n2.node_id = "unrel_n";
  n2.released = false;
  o.nodes = {n1, n2};

  Edge e1;
  e1.edge_id = "rel_e";
  e1.released = true;
  Edge e2;
  e2.edge_id = "unrel_e";
  e2.released = false;
  o.edges = {e1, e2};

  sm.set_new_order(o);

  // Action
  sm.clear_horizon();

  // Assert
  State s;
  sm.dump_to(s);

  // Only released items should remain
  ASSERT_EQ(s.node_states.size(), 1);
  EXPECT_EQ(s.node_states[0].node_id, "rel_n");

  ASSERT_EQ(s.edge_states.size(), 1);
  EXPECT_EQ(s.edge_states[0].edge_id, "rel_e");
}

//=============================================================================
// 8. General Accessor Tests
//=============================================================================

TEST_F(StateManagerTest, DumpToAndGetState)
{
  sm.set_order_id("dump_id");
  sm.set_last_node_sequence_id(99);

  // Test dump_to
  State s_dump;
  sm.dump_to(s_dump);
  EXPECT_EQ(s_dump.order_id, "dump_id");
  EXPECT_EQ(s_dump.last_node_sequence_id, 99);

  // Test get_state (const ref)
  const State& s_ref = sm.get_state();
  EXPECT_EQ(s_ref.order_id, "dump_id");
  EXPECT_EQ(s_ref.last_node_sequence_id, 99);
}

TEST_F(StateManagerTest, AssignAllFieldsFromPreviousTests_DumpEqualsExpected)
{
  // Create a VDA5050 State
  State expected;

  Header h;
  h.header_id = 999;
  expected.header = h;
  expected.order_id = "order_alpha";
  expected.order_update_id = 12u;
  expected.zone_set_id = std::optional<std::string>("zone_beta");

  expected.last_node_id = "node_start";
  expected.last_node_sequence_id = 5u;

  AGVPosition pos;
  pos.x = 10.0;
  pos.y = 20.0;
  pos.theta = 1.57;
  pos.map_id = "map_1";
  expected.agv_position = pos;

  Velocity vel;
  vel.vx = 2.5;
  vel.omega = 0.5;
  expected.velocity = vel;
  expected.driving =
    true;  // we'll toggle later to exercise both true/false in manager
  expected.distance_since_last_node = std::optional<double>(12.5);

  Load load1;
  load1.load_id = "L1";
  load1.load_type = "pallet";

  Load load2;
  load2.load_id = "L2";
  load2.load_type = "box";
  expected.loads = std::optional<std::vector<Load>>({load1, load2});
  expected.operating_mode = OperatingMode::SEMIAUTOMATIC;

  BatteryState bat;
  bat.battery_charge = 0.95;
  bat.charging = true;
  expected.battery_state = bat;

  SafetyState safe;
  safe.e_stop = EStop::AUTOACK;
  safe.field_violation = true;
  expected.safety_state = safe;
  expected.new_base_request = std::optional<bool>(true);

  Error e1;
  e1.error_type = "Hardware";
  e1.error_level = ErrorLevel::FATAL;

  Error e2;
  e2.error_type = "Software";
  e2.error_level = ErrorLevel::WARNING;
  expected.errors = {e1, e2};

  Info i1;
  i1.info_type = "Status";
  i1.info_description = "All Good";
  expected.information = std::optional<std::vector<Info>>({i1});

  ActionState as1;
  as1.action_id = "a1";
  as1.action_status = ActionStatus::WAITING;

  ActionState as2;
  as2.action_id = "a2";
  as2.action_status = ActionStatus::RUNNING;
  expected.action_states = {as1, as2};
  expected.order_id = "O1";
  expected.order_update_id = 0u;
  expected.zone_set_id = std::optional<std::string>("Z1");

  NodeState n1;
  n1.node_id = "n1";

  n1.released = true;
  NodeState n2;

  n2.node_id = "n2";
  n2.released = false;
  expected.node_states = {n1, n2};

  EdgeState edge1;
  edge1.edge_id = "e1";
  edge1.released = true;
  expected.edge_states = {edge1};
  expected.last_node_id = "keep_me";

  // Create a VDA50500 order

  sm.set_header(h);

  Order order;
  order.order_id = "O1";
  order.order_update_id = 0;
  order.zone_set_id = "Z1";

  Node on1;
  on1.node_id = "n1";
  on1.sequence_id = n1.sequence_id;
  on1.node_description = n1.node_description;
  on1.node_position = n1.node_position;
  on1.released = n1.released;
  Node on2;
  on2.node_id = "n2";
  on2.sequence_id = n2.sequence_id;
  on2.node_description = n2.node_description;
  on2.node_position = n2.node_position;
  on2.released = n2.released;
  order.nodes = {on1, on2};

  Edge oe1;
  oe1.edge_id = "e1";
  oe1.sequence_id = edge1.sequence_id;
  oe1.edge_description = edge1.edge_description;
  oe1.trajectory = edge1.trajectory;
  oe1.released = edge1.released;
  order.edges = {oe1};

  // Assign values rto State Manager
  sm.set_new_order(order);
  sm.set_last_node_id(expected.last_node_id);
  sm.set_last_node_sequence_id(expected.last_node_sequence_id);
  sm.set_agv_position(pos);
  sm.set_velocity(vel);
  sm.set_load({});
  sm.add_load(load1);
  sm.add_load(load2);
  sm.set_driving_status(true);
  sm.set_distance_since_last_node(*expected.distance_since_last_node);
  sm.set_request_new_base(*expected.new_base_request);
  sm.set_action_states(expected.action_states);
  sm.set_battery_state(expected.battery_state);
  sm.set_operating_mode(expected.operating_mode);
  sm.clear_errors();
  sm.add_error(e1);
  sm.add_error(e2);
  sm.clear_info();
  sm.add_info(i1);
  sm.set_safety_state(expected.safety_state);
  State actual;
  sm.dump_to(actual);

  // Use VDA5050 type equality operator to test (check for assignment)
  EXPECT_EQ(actual, expected)
    << "StateManager::dump_to produced a state different from expected.";

  // check get state (return by value)
  State ref = sm.get_state();
  EXPECT_EQ(ref, expected)
    << "StateManager::get_state returned a state different from expected.";
}
