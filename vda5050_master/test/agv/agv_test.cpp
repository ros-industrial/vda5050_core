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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "vda5050_master/agv/agv.hpp"
#include "vda5050_master/communication/communication.hpp"
#include "vda5050_master/vda5050_master/master.hpp"

namespace vda5050_master::test {

// =============================================================================
// Mock Communication Strategy
// =============================================================================

class MockCommunicationStrategy : public ICommunicationStrategy
{
public:
  MockCommunicationStrategy() : state_(ConnectionState::DISCONNECTED) {}

  void connect() override
  {
    state_ = ConnectionState::CONNECTED;
  }

  void disconnect() override
  {
    state_ = ConnectionState::DISCONNECTED;
  }

  void subscribe(
    const std::string& topic, MessageCallback callback,
    const int /*qos*/ = 0) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_[topic] = callback;
  }

  void unsubscribe(const std::string& topic) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.erase(topic);
  }

  void send_message(
    const std::string& topic, const std::string& message,
    const int /*qos*/ = 0) override
  {
    // Wait until unblocked (for queue policy testing)
    {
      std::unique_lock<std::mutex> lock(block_mutex_);
      blocked_waiters_++;
      blocked_waiter_cv_.notify_all();  // Notify that we're waiting
      block_cv_.wait(lock, [this] { return !blocked_; });
      blocked_waiters_--;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    sent_messages_.push_back({topic, message});
    send_count_++;
    cv_.notify_all();
  }

  ConnectionState get_state() const override
  {
    return state_;
  }

  // Blocking control for queue policy testing
  void block()
  {
    std::lock_guard<std::mutex> lock(block_mutex_);
    blocked_ = true;
  }
  void unblock()
  {
    std::lock_guard<std::mutex> lock(block_mutex_);
    blocked_ = false;
    block_cv_.notify_all();
  }

  // Wait until at least one message is blocked in send_message
  bool wait_for_blocked(
    std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
  {
    std::unique_lock<std::mutex> lock(block_mutex_);
    return blocked_waiter_cv_.wait_for(
      lock, timeout, [this] { return blocked_waiters_ > 0; });
  }

  // Test helper methods
  size_t get_send_count() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return send_count_;
  }

  std::vector<std::pair<std::string, std::string>> get_sent_messages() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return sent_messages_;
  }

  void clear_sent_messages()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    sent_messages_.clear();
    send_count_ = 0;
  }

  bool wait_for_messages(
    size_t count,
    std::chrono::milliseconds timeout = std::chrono::milliseconds(5000))
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(
      lock, timeout, [this, count] { return send_count_ >= count; });
  }

  // Simulate receiving a message on a subscribed topic
  void simulate_receive(const std::string& topic, const std::string& payload)
  {
    MessageCallback callback;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = subscriptions_.find(topic);
      if (it != subscriptions_.end())
      {
        callback = it->second;
      }
    }
    // Call callback outside lock to avoid deadlock
    if (callback)
    {
      callback(topic, payload);
    }
  }

private:
  std::atomic<ConnectionState> state_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::map<std::string, MessageCallback> subscriptions_;
  std::vector<std::pair<std::string, std::string>> sent_messages_;
  size_t send_count_ = 0;

  // Blocking support for queue policy testing
  bool blocked_{false};
  size_t blocked_waiters_{0};
  std::mutex block_mutex_;
  std::condition_variable block_cv_;
  std::condition_variable blocked_waiter_cv_;
};

// =============================================================================
// Shared Mock Registry - allows test fixture to access mocks created by factory
// =============================================================================

class MockRegistry
{
public:
  static MockRegistry& instance()
  {
    static MockRegistry registry;
    return registry;
  }

  void register_mock(const std::string& agv_id, MockCommunicationStrategy* mock)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    mocks_[agv_id] = mock;
  }

  MockCommunicationStrategy* get_mock(const std::string& agv_id)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = mocks_.find(agv_id);
    return it != mocks_.end() ? it->second : nullptr;
  }

  void clear()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    mocks_.clear();
  }

private:
  MockRegistry() = default;
  std::mutex mutex_;
  std::map<std::string, MockCommunicationStrategy*> mocks_;
};

// =============================================================================
// Test Master - Concrete implementation of VDA5050Master for testing
// =============================================================================

class TestMaster : public VDA5050Master
{
public:
  TestMaster()
  : VDA5050Master([](const std::string& agv_id) {
      auto mock = std::make_unique<MockCommunicationStrategy>();
      MockRegistry::instance().register_mock(agv_id, mock.get());
      return mock;
    })
  {
  }

  // Track received messages for verification
  std::vector<std::pair<std::string, vda5050_msgs::msg::Connection>>
    received_connections;
  std::vector<std::pair<std::string, vda5050_msgs::msg::State>> received_states;
  std::mutex callback_mutex;

protected:
  void on_connection(
    const std::string& agv_id,
    const vda5050_msgs::msg::Connection& msg) override
  {
    std::lock_guard<std::mutex> lock(callback_mutex);
    received_connections.push_back({agv_id, msg});
  }

  void on_state(
    const std::string& agv_id, const vda5050_msgs::msg::State& msg) override
  {
    std::lock_guard<std::mutex> lock(callback_mutex);
    received_states.push_back({agv_id, msg});
  }
};

// =============================================================================
// Test Fixture
// =============================================================================

class AGVIntegrationTestFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
    MockRegistry::instance().clear();
    manufacturer_ = "TestManufacturer";
    serial_number_ = "SN001";
    agv_id_ = manufacturer_ + "/" + serial_number_;
  }

  void TearDown() override
  {
    MockRegistry::instance().clear();
  }

  vda5050_msgs::msg::Order create_test_order(const std::string& order_id)
  {
    vda5050_msgs::msg::Order order;
    order.order_id = order_id;
    order.order_update_id = 0;
    return order;
  }

  vda5050_msgs::msg::InstantActions create_test_instant_actions(uint32_t id)
  {
    vda5050_msgs::msg::InstantActions actions;
    actions.header.header_id = id;
    return actions;
  }

  MockCommunicationStrategy* get_mock()
  {
    return MockRegistry::instance().get_mock(agv_id_);
  }

  std::string manufacturer_;
  std::string serial_number_;
  std::string agv_id_;
};

// =============================================================================
// Basic Registration Tests
// =============================================================================

TEST_F(AGVIntegrationTestFixture, RegisterAGVCreatesConnection)
{
  TestMaster master;

  EXPECT_FALSE(master.is_agv_registered(manufacturer_, serial_number_));

  master.register_agv(manufacturer_, serial_number_);

  EXPECT_TRUE(master.is_agv_registered(manufacturer_, serial_number_));
}

TEST_F(AGVIntegrationTestFixture, UnregisterAGVRemovesConnection)
{
  TestMaster master;

  master.register_agv(manufacturer_, serial_number_);
  EXPECT_TRUE(master.is_agv_registered(manufacturer_, serial_number_));

  master.unregister_agv(manufacturer_, serial_number_);
  EXPECT_FALSE(master.is_agv_registered(manufacturer_, serial_number_));
}

TEST_F(AGVIntegrationTestFixture, PublishOrderToUnregisteredAGVThrows)
{
  TestMaster master;

  EXPECT_THROW(
    master.publish_order(manufacturer_, serial_number_, create_test_order("1")),
    std::runtime_error);
}

TEST_F(AGVIntegrationTestFixture, PublishInstantActionsToUnregisteredAGVThrows)
{
  TestMaster master;

  EXPECT_THROW(
    master.publish_instant_actions(
      manufacturer_, serial_number_, create_test_instant_actions(1)),
    std::runtime_error);
}

// =============================================================================
// Message Publishing Tests
// =============================================================================

TEST_F(AGVIntegrationTestFixture, PublishOrderSendsMessage)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_1")));

  // Wait for message to be processed by queue thread
  ASSERT_TRUE(mock->wait_for_messages(1));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), 1u);

  // Verify topic contains "order"
  EXPECT_NE(messages[0].first.find("order"), std::string::npos);

  // Verify payload contains order_id
  EXPECT_NE(messages[0].second.find("order_1"), std::string::npos);
}

TEST_F(AGVIntegrationTestFixture, PublishInstantActionsSendsMessage)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(42)));

  // Wait for message to be processed
  ASSERT_TRUE(mock->wait_for_messages(1));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), 1u);

  // Verify topic contains "instant_actions"
  EXPECT_NE(messages[0].first.find("instant_actions"), std::string::npos);
}

TEST_F(AGVIntegrationTestFixture, PublishMultipleOrdersInSequence)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  constexpr int num_orders = 5;
  for (int i = 0; i < num_orders; ++i)
  {
    EXPECT_TRUE(master.publish_order(
      manufacturer_, serial_number_,
      create_test_order("order_" + std::to_string(i))));
  }

  // Wait for all messages
  ASSERT_TRUE(mock->wait_for_messages(num_orders));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), static_cast<size_t>(num_orders));

  // Verify messages arrived in order
  for (int i = 0; i < num_orders; ++i)
  {
    std::string expected_id = "order_" + std::to_string(i);
    EXPECT_NE(messages[i].second.find(expected_id), std::string::npos)
      << "Message " << i << " should contain " << expected_id;
  }
}

TEST_F(AGVIntegrationTestFixture, PublishMultipleInstantActionsInSequence)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  constexpr int num_actions = 5;
  for (int i = 0; i < num_actions; ++i)
  {
    EXPECT_TRUE(master.publish_instant_actions(
      manufacturer_, serial_number_,
      create_test_instant_actions(static_cast<uint32_t>(i))));
  }

  // Wait for all messages
  ASSERT_TRUE(mock->wait_for_messages(num_actions));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), static_cast<size_t>(num_actions));

  // Verify messages arrived in order (header_id should be 0, 1, 2, 3, 4)
  for (int i = 0; i < num_actions; ++i)
  {
    std::string expected_id = "\"headerId\":" + std::to_string(i);
    EXPECT_NE(messages[i].second.find(expected_id), std::string::npos)
      << "Message " << i << " should contain headerId " << i;
  }
}

// =============================================================================
// Concurrent Access Tests
// =============================================================================

TEST_F(AGVIntegrationTestFixture, ConcurrentPublishOrdersAreThreadSafe)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  // Use fewer messages to stay within default queue size (10)
  // This ensures all messages are sent without drops
  constexpr int num_threads = 2;
  constexpr int orders_per_thread = 3;
  constexpr int total_orders = num_threads * orders_per_thread;
  std::vector<std::thread> threads;

  for (int t = 0; t < num_threads; ++t)
  {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < orders_per_thread; ++i)
      {
        std::string order_id =
          "thread_" + std::to_string(t) + "_order_" + std::to_string(i);
        master.publish_order(
          manufacturer_, serial_number_, create_test_order(order_id));
      }
    });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }

  // Wait for all messages to be sent
  ASSERT_TRUE(
    mock->wait_for_messages(total_orders, std::chrono::milliseconds(10000)));

  auto messages = mock->get_sent_messages();
  EXPECT_EQ(messages.size(), static_cast<size_t>(total_orders));
}

TEST_F(AGVIntegrationTestFixture, ConcurrentPublishInstantActionsAreThreadSafe)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  // Use fewer messages to stay within default queue size (10)
  // This ensures all messages are sent without drops
  constexpr int num_threads = 2;
  constexpr int actions_per_thread = 3;
  constexpr int total_actions = num_threads * actions_per_thread;
  std::vector<std::thread> threads;

  for (int t = 0; t < num_threads; ++t)
  {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < actions_per_thread; ++i)
      {
        uint32_t action_id = static_cast<uint32_t>(t * actions_per_thread + i);
        master.publish_instant_actions(
          manufacturer_, serial_number_,
          create_test_instant_actions(action_id));
      }
    });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }

  // Wait for all messages to be sent
  ASSERT_TRUE(
    mock->wait_for_messages(total_actions, std::chrono::milliseconds(10000)));

  auto messages = mock->get_sent_messages();
  EXPECT_EQ(messages.size(), static_cast<size_t>(total_actions));
}

TEST_F(AGVIntegrationTestFixture, ConcurrentOrdersAndInstantActions)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);

  constexpr int num_orders = 10;
  constexpr int num_actions = 10;

  std::thread order_thread([&]() {
    for (int i = 0; i < num_orders; ++i)
    {
      master.publish_order(
        manufacturer_, serial_number_,
        create_test_order("order_" + std::to_string(i)));
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  });

  std::thread action_thread([&]() {
    for (int i = 0; i < num_actions; ++i)
    {
      master.publish_instant_actions(
        manufacturer_, serial_number_,
        create_test_instant_actions(static_cast<uint32_t>(i)));
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  });

  order_thread.join();
  action_thread.join();

  // Wait for all messages
  ASSERT_TRUE(mock->wait_for_messages(
    num_orders + num_actions, std::chrono::milliseconds(10000)));

  auto messages = mock->get_sent_messages();
  EXPECT_EQ(messages.size(), static_cast<size_t>(num_orders + num_actions));

  // Count orders and instant actions
  int order_count = 0;
  int action_count = 0;
  for (const auto& msg : messages)
  {
    if (msg.first.find("instant_actions") != std::string::npos)
    {
      action_count++;
    }
    else if (msg.first.find("order") != std::string::npos)
    {
      order_count++;
    }
  }

  EXPECT_EQ(order_count, num_orders);
  EXPECT_EQ(action_count, num_actions);
}

// =============================================================================
// Multi-AGV Tests
// =============================================================================

TEST_F(AGVIntegrationTestFixture, MultipleAGVsCanBeRegistered)
{
  TestMaster master;

  master.register_agv("Manufacturer1", "SN001");
  master.register_agv("Manufacturer1", "SN002");
  master.register_agv("Manufacturer2", "SN001");

  EXPECT_TRUE(master.is_agv_registered("Manufacturer1", "SN001"));
  EXPECT_TRUE(master.is_agv_registered("Manufacturer1", "SN002"));
  EXPECT_TRUE(master.is_agv_registered("Manufacturer2", "SN001"));
}

TEST_F(AGVIntegrationTestFixture, MessagesRoutedToCorrectAGV)
{
  TestMaster master;

  master.register_agv("Mfg1", "AGV1");
  master.register_agv("Mfg1", "AGV2");

  auto* mock1 = MockRegistry::instance().get_mock("Mfg1/AGV1");
  auto* mock2 = MockRegistry::instance().get_mock("Mfg1/AGV2");
  ASSERT_NE(mock1, nullptr);
  ASSERT_NE(mock2, nullptr);

  // Send to AGV1
  master.publish_order("Mfg1", "AGV1", create_test_order("order_for_agv1"));
  ASSERT_TRUE(mock1->wait_for_messages(1));

  // Send to AGV2
  master.publish_order("Mfg1", "AGV2", create_test_order("order_for_agv2"));
  ASSERT_TRUE(mock2->wait_for_messages(1));

  // Verify each AGV got its own message
  auto messages1 = mock1->get_sent_messages();
  auto messages2 = mock2->get_sent_messages();

  ASSERT_EQ(messages1.size(), 1u);
  ASSERT_EQ(messages2.size(), 1u);

  EXPECT_NE(messages1[0].second.find("order_for_agv1"), std::string::npos);
  EXPECT_NE(messages2[0].second.find("order_for_agv2"), std::string::npos);
}

// =============================================================================
// Graceful Shutdown Tests
// =============================================================================

TEST_F(AGVIntegrationTestFixture, DestructorCompletesWithPendingMessages)
{
  auto start = std::chrono::steady_clock::now();

  {
    TestMaster master;
    master.register_agv(manufacturer_, serial_number_);

    // Queue many messages
    for (int i = 0; i < 50; ++i)
    {
      master.publish_order(
        manufacturer_, serial_number_,
        create_test_order("pending_" + std::to_string(i)));
    }

    // Master destructor should complete gracefully
  }

  auto elapsed = std::chrono::steady_clock::now() - start;

  // Should complete in reasonable time
  EXPECT_LT(elapsed, std::chrono::seconds(10))
    << "Destructor took too long with pending messages";
}

TEST_F(AGVIntegrationTestFixture, UnregisterWithPendingMessages)
{
  TestMaster master;
  master.register_agv(manufacturer_, serial_number_);

  // Queue some messages
  for (int i = 0; i < 10; ++i)
  {
    master.publish_order(
      manufacturer_, serial_number_,
      create_test_order("pending_" + std::to_string(i)));
  }

  // Unregister should complete gracefully
  auto start = std::chrono::steady_clock::now();
  master.unregister_agv(manufacturer_, serial_number_);
  auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_LT(elapsed, std::chrono::seconds(5)) << "Unregister took too long";

  EXPECT_FALSE(master.is_agv_registered(manufacturer_, serial_number_));
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(AGVIntegrationTestFixture, DuplicateRegistrationIsIgnored)
{
  TestMaster master;

  master.register_agv(manufacturer_, serial_number_);
  EXPECT_TRUE(master.is_agv_registered(manufacturer_, serial_number_));

  // Second registration should be ignored (no crash, no duplicate)
  master.register_agv(manufacturer_, serial_number_);
  EXPECT_TRUE(master.is_agv_registered(manufacturer_, serial_number_));
}

TEST_F(AGVIntegrationTestFixture, UnregisterNonExistentAGVIsIgnored)
{
  TestMaster master;

  // Should not throw or crash
  EXPECT_NO_THROW(master.unregister_agv("NonExistent", "AGV"));
}

// =============================================================================
// Queue Policy Tests
// =============================================================================

TEST_F(AGVIntegrationTestFixture, DropNewestPolicyRejectsNewOrderWhenQueueFull)
{
  constexpr size_t queue_size = 3;
  constexpr bool drop_oldest = false;  // Reject new messages when full

  TestMaster master;
  master.register_agv(manufacturer_, serial_number_, queue_size, drop_oldest);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);
  mock->block();  // Block message processing so queue fills up

  // Publish first message and wait for it to be blocked in send_message
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_0")));
  ASSERT_TRUE(mock->wait_for_blocked());

  // Now fill the queue (size=3) with orders 1, 2, 3
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_1")));
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_2")));
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_3")));

  // Queue is full - new messages should be rejected (return false)
  EXPECT_FALSE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_4")));
  EXPECT_FALSE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_5")));

  // Unblock and let messages be sent
  mock->unblock();

  // Wait for all messages: 1 + queue_size
  constexpr size_t expected_messages = queue_size + 1;
  ASSERT_TRUE(mock->wait_for_messages(expected_messages));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), expected_messages);

  // Should have order_0, order_1, order_2, order_3 (new ones were rejected)
  EXPECT_NE(messages[0].second.find("order_0"), std::string::npos);
  EXPECT_NE(messages[1].second.find("order_1"), std::string::npos);
  EXPECT_NE(messages[2].second.find("order_2"), std::string::npos);
  EXPECT_NE(messages[3].second.find("order_3"), std::string::npos);
}

TEST_F(
  AGVIntegrationTestFixture,
  DropNewestPolicyRejectsNewInstantActionsWhenQueueFull)
{
  constexpr size_t queue_size = 3;
  constexpr bool drop_oldest = false;  // Reject new messages when full

  TestMaster master;
  master.register_agv(manufacturer_, serial_number_, queue_size, drop_oldest);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);
  mock->block();  // Block message processing so queue fills up

  // Publish first message and wait for it to be blocked in send_message
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(0)));
  ASSERT_TRUE(mock->wait_for_blocked());

  // Now fill the queue (size=3) with actions 1, 2, 3
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(1)));
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(2)));
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(3)));

  // Queue is full - new messages should be rejected (return false)
  EXPECT_FALSE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(4)));
  EXPECT_FALSE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(5)));

  // Unblock and let messages be sent
  mock->unblock();

  // Wait for all messages: 1 + queue_size
  constexpr size_t expected_messages = queue_size + 1;
  ASSERT_TRUE(mock->wait_for_messages(expected_messages));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), expected_messages);

  // Should have header IDs 0, 1, 2, 3 (new ones were rejected)
  EXPECT_NE(messages[0].second.find("\"headerId\":0"), std::string::npos);
  EXPECT_NE(messages[1].second.find("\"headerId\":1"), std::string::npos);
  EXPECT_NE(messages[2].second.find("\"headerId\":2"), std::string::npos);
  EXPECT_NE(messages[3].second.find("\"headerId\":3"), std::string::npos);
}

TEST_F(AGVIntegrationTestFixture, DropOldestPolicyDropsOldestOrder)
{
  constexpr size_t queue_size = 3;
  constexpr bool drop_oldest = true;  // Drop oldest messages when full

  TestMaster master;
  master.register_agv(manufacturer_, serial_number_, queue_size, drop_oldest);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);
  mock->block();  // Block message processing so queue fills up

  // Publish first message and wait for it to be blocked in send_message
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_0")));
  ASSERT_TRUE(mock->wait_for_blocked());

  // Now publish remaining messages - these will fill and overflow the queue
  // Queue will have order_1, order_2, order_3 initially (size=3)
  // order_4 causes oldest (order_1) to be dropped
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_1")));
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_2")));
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_3")));
  EXPECT_TRUE(master.publish_order(
    manufacturer_, serial_number_, create_test_order("order_4")));

  // Unblock and let messages be sent
  mock->unblock();

  // Wait for messages: order_0 (being processed) + queue_size remaining
  constexpr size_t expected_messages = queue_size + 1;
  ASSERT_TRUE(mock->wait_for_messages(expected_messages));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), expected_messages);

  // Should have order_0 (being processed), order_2, order_3, order_4 (order_1 dropped)
  EXPECT_NE(messages[0].second.find("order_0"), std::string::npos);
  EXPECT_NE(messages[1].second.find("order_2"), std::string::npos);
  EXPECT_NE(messages[2].second.find("order_3"), std::string::npos);
  EXPECT_NE(messages[3].second.find("order_4"), std::string::npos);
}

TEST_F(AGVIntegrationTestFixture, DropOldestPolicyDropsOldestInstantActions)
{
  constexpr size_t queue_size = 3;
  constexpr bool drop_oldest = true;  // Drop oldest messages when full

  TestMaster master;
  master.register_agv(manufacturer_, serial_number_, queue_size, drop_oldest);

  auto* mock = get_mock();
  ASSERT_NE(mock, nullptr);
  mock->block();  // Block message processing so queue fills up

  // Publish first message and wait for it to be blocked in send_message
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(0)));
  ASSERT_TRUE(mock->wait_for_blocked());

  // Now publish remaining messages - these will fill and overflow the queue
  // Queue will have actions 1, 2, 3 initially (size=3)
  // action 4 causes oldest (action 1) to be dropped
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(1)));
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(2)));
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(3)));
  EXPECT_TRUE(master.publish_instant_actions(
    manufacturer_, serial_number_, create_test_instant_actions(4)));

  // Unblock and let messages be sent
  mock->unblock();

  // Wait for messages: action 0 (being processed) + queue_size remaining
  constexpr size_t expected_messages = queue_size + 1;
  ASSERT_TRUE(mock->wait_for_messages(expected_messages));

  auto messages = mock->get_sent_messages();
  ASSERT_EQ(messages.size(), expected_messages);

  // Should have header IDs 0 (being processed), 2, 3, 4 (action 1 dropped)
  EXPECT_NE(messages[0].second.find("\"headerId\":0"), std::string::npos);
  EXPECT_NE(messages[1].second.find("\"headerId\":2"), std::string::npos);
  EXPECT_NE(messages[2].second.find("\"headerId\":3"), std::string::npos);
  EXPECT_NE(messages[3].second.find("\"headerId\":4"), std::string::npos);
}

}  // namespace vda5050_master::test
