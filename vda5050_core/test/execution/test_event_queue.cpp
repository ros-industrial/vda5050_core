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
#include <string>
#include <thread>

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/execution/event_queue.hpp"

namespace {

using namespace vda5050_core::execution;  // NOLINT

struct EventA : public Initialize<EventA, EventBase>
{
  int arg;

  explicit EventA(int arg) : arg(arg)
  {
    // Nothing to do here ...
  }
};

struct EventB : public Initialize<EventB, EventBase>
{
  std::string arg;

  explicit EventB(const std::string& arg) : arg(arg)
  {
    // Nothing to do here ...
  }
};

struct ComplexEvent : public Initialize<ComplexEvent, EventBase>
{
  double val;
  std::string str;

  ComplexEvent(int arg_1, double arg_2, const std::string& arg_3)
  : val(arg_1 + arg_2), str(arg_3)
  {
    // Nothing to do here ...
  }
};

}  // namespace

class EventQueueTest : public ::testing::Test
{
protected:
  std::shared_ptr<EventQueue> queue = std::make_shared<EventQueue>();
};

TEST_F(EventQueueTest, MultiEventPop)
{
  queue->push(std::make_shared<EventA>(1));
  queue->push(std::make_shared<EventB>("second"));
  queue->push(std::make_shared<EventA>(3));

  auto event_1 = std::static_pointer_cast<EventA>(queue->pop());
  EXPECT_NE(event_1, nullptr);
  EXPECT_EQ(event_1->arg, 1);

  auto event_2 = std::static_pointer_cast<EventB>(queue->pop());
  EXPECT_NE(event_2, nullptr);
  EXPECT_EQ(event_2->arg, "second");

  auto event_3 = std::static_pointer_cast<EventA>(queue->pop());
  EXPECT_NE(event_3, nullptr);
  EXPECT_EQ(event_3->arg, 3);

  EXPECT_TRUE(queue->empty());
  EXPECT_EQ(queue->pop(), nullptr);
}

TEST_F(EventQueueTest, PriorityOrdering)
{
  queue->push(std::make_shared<EventB>("normal_1"), Priority::NORMAL);
  queue->push(std::make_shared<EventB>("critical_1"), Priority::CRITICAL);
  queue->push(std::make_shared<EventB>("normal_2"), Priority::NORMAL);

  auto event_1 = std::static_pointer_cast<EventB>(queue->pop());
  ASSERT_NE(event_1, nullptr);
  EXPECT_EQ(event_1->arg, "critical_1");

  auto event_2 = std::static_pointer_cast<EventB>(queue->pop());
  ASSERT_NE(event_2, nullptr);
  EXPECT_EQ(event_2->arg, "normal_1");

  auto event_3 = std::static_pointer_cast<EventB>(queue->pop());
  ASSERT_NE(event_3, nullptr);
  EXPECT_EQ(event_3->arg, "normal_2");
}

TEST_F(EventQueueTest, PopCriticalOnly)
{
  queue->push(std::make_shared<EventB>("normal"), Priority::NORMAL);
  queue->push(std::make_shared<EventB>("critical"), Priority::CRITICAL);

  auto event_1 = std::static_pointer_cast<EventB>(queue->pop_critical_only());
  ASSERT_NE(event_1, nullptr);
  EXPECT_EQ(event_1->arg, "critical");

  auto event_2 = std::static_pointer_cast<EventB>(queue->pop_critical_only());
  EXPECT_EQ(event_2, nullptr);
  EXPECT_FALSE(queue->empty());
}

TEST_F(EventQueueTest, ClearNormalPreservesCritical)
{
  queue->push(std::make_shared<EventA>(1), Priority::NORMAL);
  queue->push(std::make_shared<EventB>("critical"), Priority::CRITICAL);

  queue->clear_normal();

  EXPECT_FALSE(queue->empty());

  auto event = std::static_pointer_cast<EventB>(queue->pop());
  EXPECT_EQ(event->arg, "critical");
  EXPECT_TRUE(queue->empty());
}

TEST_F(EventQueueTest, NullPush)
{
  queue->push(nullptr);
  EXPECT_TRUE(queue->empty());
}

TEST_F(EventQueueTest, ConcurrentPushPop)
{
  std::atomic_int received_count = 0;
  int iterations = 100;

  auto producer = std::thread([&]() {
    for (int i = 0; i < iterations; i++)
    {
      queue->push(std::make_shared<EventA>(0));
    }
  });

  auto consumer = std::thread([&]() {
    int count = 0;
    while (count < iterations)
    {
      if (queue->pop())
      {
        count++;
        received_count++;
      }
    }
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(received_count, iterations);
  EXPECT_TRUE(queue->empty());
}
