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

struct UpdateA : public vda5050_execution::UpdateBase
{
  std::type_index get_type() const override
  {
    return typeid(UpdateA);
  }
};

struct UpdateB : public vda5050_execution::UpdateBase
{
  std::type_index get_type() const override
  {
    return typeid(UpdateB);
  }
};

TEST(ProviderRegistryTest, SingleUpdateQuery)
{
  vda5050_execution::Provider provider;

  int call_count = 0;
  int wront_type_count = 0;

  provider.on<UpdateA>([&](std::shared_ptr<UpdateA>) { call_count++; });
  provider.on<UpdateB>([&](std::shared_ptr<UpdateB>) { wront_type_count++; });

  auto update = std::make_shared<UpdateA>();
  provider.push_shared(update);

  EXPECT_EQ(call_count, 1);
  EXPECT_EQ(wront_type_count, 0);
}

TEST(ProviderRegistryTest, MultiUpdateQuery)
{
  vda5050_execution::Provider provider;

  int call_count_1 = 0;
  int call_count_2 = 0;

  provider.on<UpdateA>([&](std::shared_ptr<UpdateA>) { call_count_1++; });
  provider.on<UpdateB>([&](std::shared_ptr<UpdateB>) { call_count_2++; });

  auto update_a = std::make_shared<UpdateA>();
  provider.push_shared(update_a);

  auto update_b = std::make_shared<UpdateB>();
  provider.push_shared(update_b);

  provider.push_shared(update_a);

  EXPECT_EQ(call_count_1, 2);
  EXPECT_EQ(call_count_2, 1);
}

TEST(ProviderRegistryTest, EmptyCalls)
{
  vda5050_execution::Provider provider;

  auto update = std::make_shared<UpdateA>();
  EXPECT_NO_THROW(provider.push_shared(update));
}

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
