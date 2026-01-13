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
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "vda5050_master/agv/agv.hpp"
#include "vda5050_master/communication/communication.hpp"
#include "vda5050_master/standard_names.hpp"

namespace vda5050_master::test {

// =============================================================================
// Mock Communication Strategy for Connection State Tests
// =============================================================================

class MockCommunicationForConnectionTests : public ICommunicationStrategy
{
public:
  MockCommunicationForConnectionTests() : state_(ConnectionState::DISCONNECTED)
  {
  }

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
    const std::string& /*topic*/, const std::string& /*message*/,
    const int /*qos*/ = 0) override
  {
  }

  ConnectionState get_state() const override
  {
    return state_;
  }

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
    if (callback)
    {
      callback(topic, payload);
    }
  }

  std::string find_topic_containing(const std::string& substring) const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : subscriptions_)
    {
      if (pair.first.find(substring) != std::string::npos)
      {
        return pair.first;
      }
    }
    return "";
  }

private:
  std::atomic<ConnectionState> state_;
  mutable std::mutex mutex_;
  std::map<std::string, MessageCallback> subscriptions_;
};

// =============================================================================
// Test Fixture
// =============================================================================

class AGVConnectionStateTestFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
    manufacturer_ = "TestManufacturer";
    serial_number_ = "SN001";
  }

  void TearDown() override
  {
    if (agv_)
    {
      agv_->disconnect();
      EXPECT_EQ(
        agv_->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
      agv_.reset();
    }
    mock_ptr_ = nullptr;
  }

  std::unique_ptr<AGV>& create_agv()
  {
    auto mock = std::make_unique<MockCommunicationForConnectionTests>();
    mock_ptr_ = mock.get();
    agv_ =
      std::make_unique<AGV>(manufacturer_, serial_number_, std::move(mock));
    return agv_;
  }

  std::unique_ptr<AGV>& create_agv_with_heartbeat_intervals(
    int state_heartbeat_interval = vda5050_master::StateHeartbeatInterval)
  {
    auto mock = std::make_unique<MockCommunicationForConnectionTests>();
    mock_ptr_ = mock.get();
    agv_ = std::make_unique<AGV>(
      manufacturer_, serial_number_, std::move(mock),
      AGV::DEFAULT_MAX_QUEUE_SIZE, true, state_heartbeat_interval);
    return agv_;
  }

  std::unique_ptr<AGV> agv_;

  std::string create_connection_json(const std::string& state = "ONLINE")
  {
    return
      R"({
      "headerId": 1,
      "timestamp": "2025-01-01T00:00:00.000Z",
      "version": "2.0.0",
      "manufacturer": "TestManufacturer",
      "serialNumber": "SN001",
      "connectionState": ")" +
      state + R"("
    })";
  }

  std::string manufacturer_;
  std::string serial_number_;
  MockCommunicationForConnectionTests* mock_ptr_ = nullptr;
};

// =============================================================================
// Initial State Tests
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, InitialConnectionStateIsOffline)
{
  auto& agv = create_agv();
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
}

// =============================================================================
// Connection Message Tests
// =============================================================================

TEST_F(
  AGVConnectionStateTestFixture,
  ConnectionStateOnlineAfterReceivingOnlineMessage)
{
  auto& agv = create_agv();

  bool callback_invoked = false;
  std::string received_agv_id;
  vda5050_types::ConnectionState received_connection_state;

  agv->setup_subscriptions(
    [&](const std::string& agv_id, const vda5050_types::Connection& msg) {
      callback_invoked = true;
      received_agv_id = agv_id;
      received_connection_state = msg.connection_state;
    },
    nullptr, nullptr);
  agv->connect();

  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty())
    << "Connection topic not found in subscriptions";

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));

  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_TRUE(callback_invoked);
  EXPECT_EQ(received_agv_id, manufacturer_ + "/" + serial_number_);
  EXPECT_EQ(received_connection_state, vda5050_types::ConnectionState::ONLINE);
}

TEST_F(
  AGVConnectionStateTestFixture,
  ConnectionStateOfflineAfterReceivingOfflineMessage)
{
  auto& agv = create_agv();

  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));

  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
}

TEST_F(
  AGVConnectionStateTestFixture,
  ConnectionStateConnectionBrokenAfterReceivingConnectionBrokenMessage)
{
  auto& agv = create_agv();

  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  mock_ptr_->simulate_receive(
    conn_topic, create_connection_json("CONNECTIONBROKEN"));

  EXPECT_EQ(
    agv->get_connection_status(),
    vda5050_types::ConnectionState::CONNECTIONBROKEN);
}

// =============================================================================
// State Transition Tests
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, TransitionOnlineToOfflineViaMessage)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
}

TEST_F(AGVConnectionStateTestFixture, TransitionOnlineToOfflineViaDisconnect)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  agv->disconnect();
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
}

TEST_F(
  AGVConnectionStateTestFixture, TransitionOfflineToConnectionBrokenToOnline)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  mock_ptr_->simulate_receive(
    conn_topic, create_connection_json("CONNECTIONBROKEN"));
  EXPECT_EQ(
    agv->get_connection_status(),
    vda5050_types::ConnectionState::CONNECTIONBROKEN);

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
}

TEST_F(
  AGVConnectionStateTestFixture,
  TransitionOfflineToOnlineToConnectionBrokenToOnline)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  mock_ptr_->simulate_receive(
    conn_topic, create_connection_json("CONNECTIONBROKEN"));
  EXPECT_EQ(
    agv->get_connection_status(),
    vda5050_types::ConnectionState::CONNECTIONBROKEN);

  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
}

// =============================================================================
// Multiple Connect/Disconnect Cycles
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, MultipleConnectDisconnectCycles)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr);

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");

  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  for (int i = 0; i < 5; ++i)
  {
    agv->connect();

    mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
    EXPECT_EQ(
      agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

    agv->disconnect();
    EXPECT_EQ(
      agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
  }
}

// =============================================================================
// Enum Value Tests
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, AGVConnectionStateEnumValues)
{
  EXPECT_NE(
    vda5050_types::ConnectionState::ONLINE,
    vda5050_types::ConnectionState::OFFLINE);
  EXPECT_NE(
    vda5050_types::ConnectionState::ONLINE,
    vda5050_types::ConnectionState::CONNECTIONBROKEN);
  EXPECT_NE(
    vda5050_types::ConnectionState::OFFLINE,
    vda5050_types::ConnectionState::CONNECTIONBROKEN);
}

// =============================================================================
// Thread Safety Tests
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, ConcurrentConnectionStateAccessIsSafe)
{
  auto& agv = create_agv();
  std::atomic<bool> stop{false};

  std::thread reader([&]() {
    while (!stop.load())
    {
      auto conn_status = agv->get_connection_status();
      (void)conn_status;
    }
  });

  for (int i = 0; i < 100; ++i)
  {
    agv->connect();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    agv->disconnect();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  stop.store(true);
  reader.join();

  SUCCEED();
}

// =============================================================================
// AGV Lifecycle Tests
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, AGVCanBeDestroyedWhileConnected)
{
  {
    auto& agv = create_agv();
    agv->connect();
  }
  SUCCEED();
}

TEST_F(AGVConnectionStateTestFixture, AGVCanBeDestroyedWhileDisconnected)
{
  {
    auto& agv = create_agv();
    agv->connect();
    agv->disconnect();
  }
  SUCCEED();
}

TEST_F(AGVConnectionStateTestFixture, AGVCanBeDestroyedWithoutEverConnecting)
{
  auto& agv = create_agv();
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
}

}  // namespace vda5050_master::test
