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
#include <vector>

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/execution/provider.hpp"

namespace {

using namespace vda5050_core::execution;  // NOLINT

struct UpdateA : public Initialize<UpdateA, UpdateBase>
{};

struct UpdateB : public Initialize<UpdateB, UpdateBase>
{};

struct MockBatteryStatus : public Initialize<MockBatteryStatus, UpdateBase>
{
  double percentage;
  bool charging;

  MockBatteryStatus(double percentage, bool charging)
  : percentage(percentage), charging(charging)
  {
    // Nothing to do here ...
  }
};

struct MockNavigationStatus
: public Initialize<MockNavigationStatus, UpdateBase>
{
  uint32_t sequence_id;

  explicit MockNavigationStatus(uint32_t sequence_id) : sequence_id(sequence_id)
  {
    // Nothing to do here ...
  }
};

};  // namespace

class ProviderTest : public ::testing::Test
{
protected:
  std::shared_ptr<Provider> provider = std::make_shared<Provider>();
};

TEST_F(ProviderTest, SingleUpdatePush)
{
  int call_count = 0;
  int wront_type_count = 0;

  provider->on<UpdateA>([&](std::shared_ptr<UpdateA>) { call_count++; });
  provider->on<UpdateB>([&](std::shared_ptr<UpdateB>) { wront_type_count++; });

  auto update = std::make_shared<UpdateA>();
  provider->push_shared(update);

  EXPECT_EQ(call_count, 1);
  EXPECT_EQ(wront_type_count, 0);
}

TEST_F(ProviderTest, MultipleUpdatePush)
{
  int call_count_1 = 0;
  int call_count_2 = 0;

  provider->on<UpdateA>([&](std::shared_ptr<UpdateA>) { call_count_1++; });
  provider->on<UpdateB>([&](std::shared_ptr<UpdateB>) { call_count_2++; });

  auto update_a = std::make_shared<UpdateA>();
  provider->push_shared(update_a);

  auto update_b = std::make_shared<UpdateB>();
  provider->push_shared(update_b);

  provider->push_shared(update_a);

  EXPECT_EQ(call_count_1, 2);
  EXPECT_EQ(call_count_2, 1);
}

TEST_F(ProviderTest, EmptyCalls)
{
  auto update = std::make_shared<UpdateA>();
  EXPECT_NO_THROW(provider->push_shared(update));
}

TEST_F(ProviderTest, PushUpdate)
{
  int call_count_1 = 0;
  int call_count_2 = 0;

  provider->on<MockBatteryStatus>([&](auto update) {
    call_count_1++;
    EXPECT_EQ(update->percentage, 47.3);
    EXPECT_FALSE(update->charging);
  });

  provider->on<MockNavigationStatus>([&](auto update) {
    call_count_2++;
    EXPECT_EQ(update->sequence_id, 2);
  });

  provider->push<MockBatteryStatus>(47.3, false);
  provider->push<MockNavigationStatus>(2);
  provider->push<MockBatteryStatus>(47.3, false);

  EXPECT_EQ(call_count_1, 2);
  EXPECT_EQ(call_count_2, 1);
}

TEST_F(ProviderTest, ConcurrentRegistrationAndPush)
{
  std::atomic_int total_received = 0;
  const int num_threads = 10;
  const int num_thread_updates = 100;
  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; i++)
  {
    threads.emplace_back([&] {
      provider->on<MockBatteryStatus>(
        [&](auto /*update*/) { total_received++; });
    });
  }

  for (auto& t : threads) t.join();
  threads.clear();

  for (int i = 0; i < num_threads; i++)
  {
    threads.emplace_back([&] {
      for (int j = 0; j < num_thread_updates; j++)
      {
        provider->push<MockBatteryStatus>(35.8, false);
      }
    });
  }

  for (auto& t : threads) t.join();

  EXPECT_EQ(total_received, num_threads * num_threads * num_thread_updates);
}

TEST_F(ProviderTest, ConcurrentMultiplePush)
{
  const int num_thread_updates = 100;
  std::atomic_int battery_count = 0;
  std::atomic_int navigation_count = 0;

  provider->on<MockBatteryStatus>([&](auto /*update*/) { battery_count++; });

  provider->on<MockNavigationStatus>(
    [&](auto /*update*/) { navigation_count++; });

  auto thread_1 = std::thread([&] {
    for (int i = 0; i < num_thread_updates; i++)
    {
      provider->push<MockBatteryStatus>(76.3, true);
    }
  });

  auto thread_2 = std::thread([&] {
    for (int i = 0; i < num_thread_updates; i++)
    {
      provider->push<MockNavigationStatus>(2);
    }
  });

  thread_1.join();
  thread_2.join();

  EXPECT_EQ(battery_count, 100);
  EXPECT_EQ(navigation_count, 100);
}
