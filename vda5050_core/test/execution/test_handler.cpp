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

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/handler.hpp"
#include "vda5050_core/execution/strategy_interface.hpp"

namespace {

using namespace vda5050_core::execution;  // NOLINT

class MockContext : public ContextInterface
{
public:
  std::atomic_int init_calls = 0;

  void init() override
  {
    init_calls++;
  }

  std::shared_ptr<UpdateBase> get_update_raw(
    std::type_index /*type*/) const override
  {
    return nullptr;
  }

  std::shared_ptr<ResourceBase> get_resource_raw(
    std::type_index /*type*/) const override
  {
    return nullptr;
  }
};

class MockStrategy : public StrategyInterface
{
public:
  std::atomic_int init_calls = 0;
  std::atomic_int step_calls = 0;

  void init(std::shared_ptr<ContextInterface> /*context*/) override
  {
    init_calls++;
  }

  void step(std::shared_ptr<ContextInterface> /*context*/) override
  {
    step_calls++;
  }
};

}  // namespace

TEST(HandlerTest, SpinOnceRunsStrategy)
{
  auto context = std::make_shared<MockContext>();
  auto strategy = std::make_shared<MockStrategy>();
  std::vector<std::shared_ptr<StrategyInterface>> strategies = {strategy};

  auto handler = Handler::make(context, strategies);
  EXPECT_EQ(strategy->init_calls, 1);
  EXPECT_EQ(context->init_calls, 1);
  EXPECT_EQ(strategy->step_calls, 1);

  handler->spin_once();
  EXPECT_EQ(strategy->step_calls, 2);
}

TEST(HandlerTest, InitAndRemoveStrategies)
{
  auto context = std::make_shared<MockContext>();
  auto strategy_1 = std::make_shared<MockStrategy>();
  auto strategy_2 = std::make_shared<MockStrategy>();
  std::vector<std::shared_ptr<StrategyInterface>> strategies = {
    strategy_1, strategy_2};

  auto handler = Handler::make(context, strategies);
  EXPECT_EQ(strategy_1->init_calls, 1);
  EXPECT_EQ(strategy_2->init_calls, 1);
  EXPECT_EQ(context->init_calls, 1);

  handler->remove_strategy_by_type<MockStrategy>();
  handler->spin_once();

  EXPECT_EQ(strategy_1->step_calls, 1);
  EXPECT_EQ(strategy_2->step_calls, 1);
}

TEST(HandlerTest, AddAndRemoveStrategy)
{
  auto context = std::make_shared<MockContext>();
  auto strategy = std::make_shared<MockStrategy>();

  auto handler = Handler::make(context);

  handler->add_strategy(strategy);
  EXPECT_EQ(strategy->init_calls, 1);

  handler->spin_once();
  EXPECT_EQ(strategy->step_calls, 1);

  handler->remove_strategy(strategy);
  handler->spin_once();
  EXPECT_EQ(strategy->step_calls, 1);
}

TEST(HandlerTest, SpinInThread)
{
  auto context = std::make_shared<MockContext>();
  auto strategy = std::make_shared<MockStrategy>();
  std::vector<std::shared_ptr<StrategyInterface>> strategies = {strategy};

  auto handler = Handler::make(context, strategies);

  std::atomic_bool thread_running = false;
  auto spin_thread = std::thread([&] {
    thread_running = true;
    handler->spin(std::chrono::milliseconds(100));
  });

  while (!thread_running) std::this_thread::yield();

  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  EXPECT_EQ(strategy->step_calls, 1);

  handler->wake();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  EXPECT_GE(strategy->step_calls, 2);

  handler->stop();
  handler->wake();
  if (spin_thread.joinable()) spin_thread.join();
}

TEST(HandlerTest, ConcurrentStrategyModification)
{
  auto context = std::make_shared<MockContext>();
  auto handler = Handler::make(context);

  std::atomic_bool keep_running = true;

  auto spin_thread = std::thread([&] {
    while (keep_running) handler->spin_once();
  });

  auto modifying_thread = std::thread([&] {
    for (int i = 0; i < 100; i++)
    {
      auto strategy = std::make_shared<MockStrategy>();
      handler->add_strategy(strategy);
      handler->remove_strategy(strategy);
    }
    keep_running = false;
  });

  modifying_thread.join();
  spin_thread.join();
  SUCCEED();
}

TEST(HandlerTest, SpinAndSpinOnceSimultaneously)
{
  auto context = std::make_shared<MockContext>();
  auto strategy = std::make_shared<MockStrategy>();
  std::vector<std::shared_ptr<StrategyInterface>> strategies = {strategy};

  auto handler = Handler::make(context, strategies);

  std::atomic_bool thread_running = false;
  auto spin_thread = std::thread([&] {
    thread_running = true;
    handler->spin(std::chrono::milliseconds(100));
  });

  while (!thread_running) std::this_thread::yield();

  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  EXPECT_EQ(strategy->step_calls, 1);

  handler->spin_once();
  EXPECT_EQ(strategy->step_calls, 2);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  EXPECT_EQ(strategy->step_calls, 2);

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  EXPECT_GE(strategy->step_calls, 3);

  handler->stop();
  handler->wake();
  if (spin_thread.joinable()) spin_thread.join();
}

TEST(HandlerTest, WakeWithContext)
{
  auto context = std::make_shared<MockContext>();
  auto strategy = std::make_shared<MockStrategy>();
  std::vector<std::shared_ptr<StrategyInterface>> strategies = {strategy};

  auto handler = Handler::make(context, strategies);

  std::atomic_bool thread_running = false;
  auto spin_thread = std::thread([&] {
    thread_running = true;
    handler->spin(std::chrono::milliseconds(100));
  });

  while (!thread_running) std::this_thread::yield();

  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  EXPECT_EQ(strategy->step_calls, 1);

  context->notify_on_change();

  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  EXPECT_EQ(strategy->step_calls, 2);

  handler->stop();
  handler->wake();
  if (spin_thread.joinable()) spin_thread.join();
}
