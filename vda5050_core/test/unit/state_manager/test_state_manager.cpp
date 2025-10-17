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

class StateManagerTest : public ::testing::Test {
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

  const auto &loads = sm.get_loads();
  ASSERT_EQ(loads.size(), 1);
  EXPECT_EQ(loads[0].load_id, "test_load");

  EXPECT_TRUE(sm.remove_load("test_load"));
  EXPECT_TRUE(sm.get_loads().empty());
}

TEST_F(StateManagerTest, SetOperatingMode)
{
  EXPECT_TRUE(sm.set_operating_mode(OperatingMode::AUTOMATIC));
  EXPECT_EQ(sm.get_operating_mode(), OperatingMode::AUTOMATIC);
  EXPECT_FALSE(sm.set_operating_mode(OperatingMode::AUTOMATIC));  // no change
}

TEST_F(StateManagerTest, SetBatteryState)
{
  BatteryState battery;
  battery.battery_charge = 0.8;
  battery.battery_voltage = 48.2;

  sm.set_battery_state(battery);
  const auto &b = sm.get_battery_state();
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
