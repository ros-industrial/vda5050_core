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

#include <vda5050_types/agv_position.hpp>
#include <vda5050_types/battery_state.hpp>

#include "vda5050_execution/provider.hpp"
#include "vda5050_execution/update.hpp"

TEST(ProviderTest, PushUpdate)
{
  auto provider = std::make_shared<vda5050_execution::Provider>();

  int call_count_1 = 0;
  int call_count_2 = 0;

  vda5050_types::AGVPosition position;
  position.x = 23.8;
  position.y = 50.7;
  position.theta = 1.57;
  position.position_initialized = true;

  provider->on<vda5050_execution::PositionData>(
    [&](std::shared_ptr<vda5050_execution::PositionData> update) {
      call_count_1++;
      EXPECT_EQ(*update->agv_position, position);
    });

  vda5050_types::BatteryState battery;
  battery.battery_charge = 78.5;

  provider->on<vda5050_execution::BatteryData>(
    [&](std::shared_ptr<vda5050_execution::BatteryData> update) {
      call_count_2++;
      EXPECT_EQ(*update->battery_state, battery);
    });

  provider->push<vda5050_execution::PositionData>(
    std::make_shared<vda5050_types::AGVPosition>(position));
  provider->push<vda5050_execution::BatteryData>(
    std::make_shared<vda5050_types::BatteryState>(battery));
  provider->push<vda5050_execution::PositionData>(
    std::make_shared<vda5050_types::AGVPosition>(position));

  EXPECT_EQ(call_count_1, 2);
  EXPECT_EQ(call_count_2, 1);
}
