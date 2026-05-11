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
#include <thread>

#include "vda5050_core/types/edge.hpp"
#include "vda5050_core/types/node.hpp"

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/execution/engine.hpp"

namespace {

using namespace vda5050_core::execution;  // NOLINT

struct EventA : public Initialize<EventA, EventBase>
{};

struct EventB : public Initialize<EventB, EventBase>
{};

struct MockNavigationCommand
: public Initialize<MockNavigationCommand, EventBase>
{
  std::string node_id;

  explicit MockNavigationCommand(const std::string& node_id) : node_id(node_id)
  {
    // Nothing to do here ...
  }
};

struct MockNavigationAcknowledgement
: public Initialize<MockNavigationAcknowledgement, UpdateBase>
{
  std::string node_id;

  explicit MockNavigationAcknowledgement(const std::string& node_id)
  : node_id(node_id)
  {
    // Nothing to do here ...
  }
};

struct MockActionCommand : public Initialize<MockActionCommand, EventBase>
{
  std::string action_name;
  std::string action_parameter;
  uint32_t flag;

  MockActionCommand(
    const std::string& action_name, const std::string& action_parameter,
    uint32_t flag)
  : action_name(action_name), action_parameter(action_parameter), flag(flag)
  {
    // Nothing to do here ...
  }
};

}  // namespace

class EngineTest : public ::testing::Test
{
protected:
  std::shared_ptr<Engine> engine = std::make_shared<Engine>();
};

TEST_F(EngineTest, SingleEventDispatch)
{
  int call_count_1 = 0;
  int call_count_2 = 0;
  int wrong_type_count = 0;

  engine->on<EventA>(
    [&](std::shared_ptr<EventA> /*event*/) { call_count_1++; });
  engine->on<EventA>(
    [&](std::shared_ptr<EventA> /*event*/) { call_count_2++; });
  engine->on<EventB>(
    [&](std::shared_ptr<EventB> /*event*/) { wrong_type_count++; });

  auto event = std::make_shared<EventA>();
  engine->emit_shared(event);

  engine->step();
  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 1);
  EXPECT_EQ(wrong_type_count, 0);

  engine->step();
  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 1);
  EXPECT_EQ(wrong_type_count, 0);
}

TEST_F(EngineTest, MultiEventDispatch)
{
  int call_count_1 = 0;
  int call_count_2 = 0;

  engine->on<EventA>(
    [&](std::shared_ptr<EventA> /*event*/) { call_count_1++; });
  engine->on<EventB>(
    [&](std::shared_ptr<EventB> /*event*/) { call_count_2++; });

  auto event_a = std::make_shared<EventA>();
  engine->emit_shared(event_a);

  auto event_b = std::make_shared<EventB>();
  engine->emit_shared(event_b);

  engine->emit_shared(event_a);

  engine->step();
  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 0);

  engine->step();
  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 1);
}

TEST_F(EngineTest, EmptyQueueTest)
{
  EXPECT_NO_THROW(engine->step());
}

TEST_F(EngineTest, EmptyCalls)
{
  auto event = std::make_shared<EventA>();
  EXPECT_NO_THROW(engine->emit_shared(event));
}

TEST_F(EngineTest, DispatchEvent)
{
  int call_count_1 = 0;
  int call_count_2 = 0;

  engine->on<MockNavigationCommand>([&](auto event) {
    call_count_1++;
    EXPECT_EQ(event->node_id, "node_1");
  });

  engine->on<MockActionCommand>([&](auto event) {
    call_count_2++;
    EXPECT_EQ(event->action_name, "stop");
    EXPECT_EQ(event->action_parameter, "object");
    EXPECT_EQ(event->flag, 1);
  });

  engine->emit<MockNavigationCommand>(Priority::NORMAL, "node_1");
  engine->emit<MockActionCommand>(Priority::NORMAL, "stop", "object", 1);

  engine->step();

  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 0);

  engine->step();

  EXPECT_EQ(call_count_1, 1);
  EXPECT_EQ(call_count_2, 1);
}

TEST_F(EngineTest, PriorityProcessing)
{
  std::vector<std::string> execution_order;

  engine->on<MockNavigationCommand>(
    [&](auto event) { execution_order.push_back(event->node_id); });

  engine->emit<MockNavigationCommand>(Priority::NORMAL, "node_2");
  engine->emit<MockNavigationCommand>(Priority::CRITICAL, "node_charging");

  engine->step();
  engine->step();

  ASSERT_EQ(execution_order.size(), 2);
  EXPECT_EQ(execution_order[0], "node_charging");
  EXPECT_EQ(execution_order[1], "node_2");
}

TEST_F(EngineTest, WaitConditionSucess)
{
  engine->suspend_for<MockNavigationAcknowledgement>(
    std::chrono::milliseconds(1000),
    [](auto update) { return update->node_id == "node_3"; });

  EXPECT_TRUE(engine->waiting());

  engine->notify(std::make_shared<MockNavigationAcknowledgement>("node_2"));
  EXPECT_TRUE(engine->waiting());

  engine->notify(std::make_shared<MockNavigationAcknowledgement>("node_3"));
  EXPECT_FALSE(engine->waiting());
}

TEST_F(EngineTest, WaitTimeout)
{
  std::vector<std::string> execution_log;

  engine->on<MockNavigationCommand>(
    [&](auto event) { execution_log.push_back(event->node_id); });

  engine->emit<MockNavigationCommand>(Priority::NORMAL, "node_delayed");

  engine->suspend_for<MockNavigationAcknowledgement>(
    std::chrono::milliseconds(10));
  EXPECT_TRUE(engine->waiting());

  engine->step();
  EXPECT_TRUE(execution_log.empty());

  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  EXPECT_FALSE(engine->waiting());

  engine->step();
  ASSERT_EQ(execution_log.size(), 1);
  EXPECT_EQ(execution_log[0], "node_delayed");
}

TEST_F(EngineTest, ConcurrentEmitAndStep)
{
  std::atomic_int processed_count = 0;
  const int steps = 500;

  engine->on<MockNavigationCommand>([&](auto /*event*/) { processed_count++; });

  auto emitter = std::thread([&]() {
    for (int i = 0; i < 500; i++)
    {
      engine->emit<MockNavigationCommand>(Priority::NORMAL, std::to_string(i));
    }
  });

  auto stepper = std::thread([&]() {
    for (int i = 0; i < 500; i++)
    {
      engine->step();
      std::this_thread::yield();
    }
  });

  emitter.join();
  stepper.join();

  while (processed_count < steps) engine->step();

  EXPECT_EQ(processed_count, 500);
}
