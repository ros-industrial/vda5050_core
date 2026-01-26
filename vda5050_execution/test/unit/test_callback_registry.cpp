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

#include "vda5050_execution/callback_registry.hpp"
#include "vda5050_execution/event.hpp"

struct EventA : public vda5050_execution::EventBase
{
  std::type_index get_type() const override
  {
    return typeid(EventA);
  }
};

struct EventB : public vda5050_execution::EventBase
{
  std::type_index get_type() const override
  {
    return typeid(EventB);
  }
};

TEST(CallbackRegistryTest, SingleEventDispatch)
{
  vda5050_execution::CallbackRegistry registry;

  int call_count_1 = 0;
  int call_count_2 = 0;
  int wront_type_count = 0;

  registry.register_callback<EventA>(
    [&](std::shared_ptr<EventA> /*event*/) { call_count_1++; });
  registry.register_callback<EventA>(
    [&](std::shared_ptr<EventA> /*event*/) { call_count_2++; });
  registry.register_callback<EventB>(
    [&](std::shared_ptr<EventB> /*event*/) { wront_type_count++; });

  auto event = std::make_shared<EventA>();
  registry.dispatch(event);

  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 1);
  EXPECT_EQ(wront_type_count, 0);
}

TEST(CallbackRegistryTest, MultiEventDispatch)
{
  vda5050_execution::CallbackRegistry registry;

  int call_count_1 = 0;
  int call_count_2 = 0;

  registry.register_callback<EventA>(
    [&](std::shared_ptr<EventA> /*event*/) { call_count_1++; });
  registry.register_callback<EventB>(
    [&](std::shared_ptr<EventB> /*event*/) { call_count_2++; });

  auto event_a = std::make_shared<EventA>();
  registry.dispatch(event_a);

  auto event_b = std::make_shared<EventB>();
  registry.dispatch(event_b);

  registry.dispatch(event_a);

  EXPECT_EQ(call_count_1, 2);
  EXPECT_EQ(call_count_2, 1);
}

TEST(CallbackRegistryTest, EmptyCalls)
{
  vda5050_execution::CallbackRegistry registry;

  auto event = std::make_shared<EventA>();
  EXPECT_NO_THROW(registry.dispatch(event));
}
