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

#include <string>
#include <typeindex>

#include "vda5050_execution/event.hpp"
#include "vda5050_execution/event_queue.hpp"

struct EventA : public vda5050_execution::EventBase
{
  int arg;

  explicit EventA(int arg) : arg(arg)
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(EventA));
  }
};

struct EventB : public vda5050_execution::EventBase
{
  std::string arg;

  explicit EventB(const std::string& arg) : arg(arg)
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(EventB));
  }
};

struct ComplexEvent : public vda5050_execution::EventBase
{
  double val;
  std::string str;

  ComplexEvent(int arg_1, double arg_2, const std::string& arg_3)
  : val(arg_1 + arg_2), str(arg_3)
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(ComplexEvent));
  }
};

TEST(EventQueueTest, MultiEventPop)
{
  vda5050_execution::EventQueue queue;

  queue.push<EventA>(1);
  queue.push<EventB>("second");
  queue.push<EventA>(3);

  auto event_1 = queue.pop();
  EXPECT_NE(event_1, nullptr);
  EXPECT_EQ(static_cast<EventA*>(event_1.get())->arg, 1);

  auto event_2 = queue.pop();
  EXPECT_NE(event_2, nullptr);
  EXPECT_EQ(static_cast<EventB*>(event_2.get())->arg, "second");

  auto event_3 = queue.pop();
  EXPECT_NE(event_3, nullptr);
  EXPECT_EQ(static_cast<EventA*>(event_3.get())->arg, 3);

  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.pop(), nullptr);
}

TEST(EventQueueTest, ArgumentForwarding)
{
  vda5050_execution::EventQueue queue;

  queue.push<ComplexEvent>(8, 2.5, "test");

  auto event = queue.pop();
  auto concrete = static_cast<ComplexEvent*>(event.get());

  EXPECT_EQ(concrete->val, 10.5);
  EXPECT_EQ(concrete->str, "test");
}
