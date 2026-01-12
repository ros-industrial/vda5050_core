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
// Mock Communication Strategy for Operational State Tests
// =============================================================================

class MockCommunicationForOperationalTests : public ICommunicationStrategy
{
public:
  MockCommunicationForOperationalTests() : state_(ConnectionState::DISCONNECTED)
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

class AGVOperationalStateTestFixture : public ::testing::Test
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
      agv_.reset();
    }
    mock_ptr_ = nullptr;
  }

  std::unique_ptr<AGV>& create_agv()
  {
    auto mock = std::make_unique<MockCommunicationForOperationalTests>();
    mock_ptr_ = mock.get();
    agv_ =
      std::make_unique<AGV>(manufacturer_, serial_number_, std::move(mock));
    return agv_;
  }

  std::unique_ptr<AGV>& create_agv_with_heartbeat_intervals(
    int connection_heartbeat_interval, int state_heartbeat_interval)
  {
    auto mock = std::make_unique<MockCommunicationForOperationalTests>();
    mock_ptr_ = mock.get();
    agv_ = std::make_unique<AGV>(
      manufacturer_, serial_number_, std::move(mock),
      AGV::DEFAULT_MAX_QUEUE_SIZE, true, connection_heartbeat_interval,
      state_heartbeat_interval);
    return agv_;
  }

  std::unique_ptr<AGV> agv_;

  std::string create_state_json()
  {
    return
      R"({
      "headerId": 1,
      "timestamp": "2025-01-01T00:00:00.000Z",
      "version": "2.0.0",
      "manufacturer": "TestManufacturer",
      "serialNumber": "SN001",
      "orderId": "test_order",
      "orderUpdateId": 0,
      "lastNodeId": "",
      "lastNodeSequenceId": 0,
      "driving": false,
      "paused": false,
      "newBaseRequest": false,
      "distanceSinceLastNode": 0.0,
      "operatingMode": "AUTOMATIC",
      "nodeStates": [],
      "edgeStates": [],
      "actionStates": [],
      "batteryState": {
        "batteryCharge": 100.0,
        "charging": false
      },
      "errors": [],
      "safetyState": {
        "eStop": "NONE",
        "fieldViolation": false
      }
    })";
  }

  std::string manufacturer_;
  std::string serial_number_;
  MockCommunicationForOperationalTests* mock_ptr_ = nullptr;
};

// =============================================================================
// Initial State Tests
// =============================================================================

TEST_F(AGVOperationalStateTestFixture, InitialOperationalStateIsUnknown)
{
  auto& agv = create_agv();
  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);
}

TEST_F(
  AGVOperationalStateTestFixture,
  OperationalStateRemainsUnknownAfterConnectBeforeStateMessage)
{
  auto& agv = create_agv();
  agv->connect();

  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);
}

// =============================================================================
// State Message Tests
// =============================================================================

TEST_F(
  AGVOperationalStateTestFixture,
  OperationalStateOnlineAfterReceivingStateMessage)
{
  auto& agv = create_agv();

  bool callback_invoked = false;
  std::string received_agv_id;

  agv->setup_subscriptions(
    nullptr,
    [&](const std::string& agv_id, const vda5050_types::State& msg) {
      callback_invoked = true;
      received_agv_id = agv_id;
      (void)msg;
    },
    nullptr);
  agv->connect();

  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  ASSERT_FALSE(state_topic.empty()) << "State topic not found in subscriptions";

  mock_ptr_->simulate_receive(state_topic, create_state_json());

  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);
  EXPECT_TRUE(callback_invoked);
  EXPECT_EQ(received_agv_id, manufacturer_ + "/" + serial_number_);
}

// =============================================================================
// State Heartbeat Timeout Tests
// =============================================================================

TEST_F(
  AGVOperationalStateTestFixture,
  StateHeartbeatTimeoutTransitionsToStateUnknown)
{
  // Create AGV with short state heartbeat interval (1 second)
  auto& agv = create_agv_with_heartbeat_intervals(
    vda5050_master::ConnectionHeartbeatInterval, 1);

  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  ASSERT_FALSE(state_topic.empty());

  // Receive state message to go ONLINE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  // Wait for the state heartbeat timeout to trigger
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  // Verify state has transitioned to STATE_UNKNOWN due to timeout
  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);
}

TEST_F(
  AGVOperationalStateTestFixture, StateHeartbeatReceivingMessagesPreventTimeout)
{
  // Create AGV with short state heartbeat interval (2 seconds)
  auto& agv = create_agv_with_heartbeat_intervals(
    vda5050_master::ConnectionHeartbeatInterval, 2);

  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  ASSERT_FALSE(state_topic.empty());

  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  // Send state messages periodically to keep operational state alive
  for (int i = 0; i < 3; ++i)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mock_ptr_->simulate_receive(state_topic, create_state_json());
    EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);
  }

  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);
}

TEST_F(
  AGVOperationalStateTestFixture,
  TransitionOnlineToUnknownViaTimeoutThenRecover)
{
  // Create AGV with short state heartbeat interval (1 second)
  auto& agv = create_agv_with_heartbeat_intervals(
    vda5050_master::ConnectionHeartbeatInterval, 1);

  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  ASSERT_FALSE(state_topic.empty());

  // Initial state is UNKNOWN
  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);

  // Receive state message to go ONLINE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  // Wait for timeout
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));
  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);

  // Recover by receiving state message
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);
}

// =============================================================================
// Connection State Affects Operational State Tests
// =============================================================================

TEST_F(
  AGVOperationalStateTestFixture, DisconnectSetsOperationalStateToUnavailable)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  agv->disconnect();

  // Operational state becomes UNAVAILABLE when disconnected
  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);
}

TEST_F(
  AGVOperationalStateTestFixture,
  ConnectionOfflineSetsOperationalStateToUnavailable)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(state_topic.empty());
  ASSERT_FALSE(conn_topic.empty());

  // Receive state message to go AVAILABLE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  // Receive OFFLINE connection message
  mock_ptr_->simulate_receive(
    conn_topic,
    R"({"headerId": 1, "timestamp": "2025-01-01T00:00:00.000Z", "version": "2.0.0", "manufacturer": "TestManufacturer", "serialNumber": "SN001", "connectionState": "OFFLINE"})");

  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);
}

TEST_F(
  AGVOperationalStateTestFixture,
  ConnectionBrokenSetsOperationalStateToUnavailable)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(state_topic.empty());
  ASSERT_FALSE(conn_topic.empty());

  // Receive state message to go AVAILABLE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  // Receive CONNECTIONBROKEN connection message
  mock_ptr_->simulate_receive(
    conn_topic,
    R"({"headerId": 1, "timestamp": "2025-01-01T00:00:00.000Z", "version": "2.0.0", "manufacturer": "TestManufacturer", "serialNumber": "SN001", "connectionState": "CONNECTIONBROKEN"})");

  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);
}

TEST_F(
  AGVOperationalStateTestFixture,
  ConnectionHeartbeatTimeoutSetsOperationalStateToUnavailable)
{
  // Create AGV with short connection heartbeat interval (1 second)
  auto& agv = create_agv_with_heartbeat_intervals(
    1, vda5050_master::StateHeartbeatInterval);

  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(state_topic.empty());
  ASSERT_FALSE(conn_topic.empty());

  // Receive state and connection messages to go AVAILABLE/ONLINE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  mock_ptr_->simulate_receive(
    conn_topic,
    R"({"headerId": 1, "timestamp": "2025-01-01T00:00:00.000Z", "version": "2.0.0", "manufacturer": "TestManufacturer", "serialNumber": "SN001", "connectionState": "ONLINE"})");
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  // Wait for connection heartbeat timeout
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  // Connection becomes CONNECTIONBROKEN, operational state becomes UNAVAILABLE
  EXPECT_EQ(
    agv->get_connection_status(),
    vda5050_types::ConnectionState::CONNECTIONBROKEN);
  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);
}

TEST_F(
  AGVOperationalStateTestFixture, RecoverFromUnavailableAfterConnectionRestored)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string state_topic = mock_ptr_->find_topic_containing("state");
  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(state_topic.empty());
  ASSERT_FALSE(conn_topic.empty());

  // Receive state message to go AVAILABLE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);

  // Receive OFFLINE connection message - becomes UNAVAILABLE
  mock_ptr_->simulate_receive(
    conn_topic,
    R"({"headerId": 1, "timestamp": "2025-01-01T00:00:00.000Z", "version": "2.0.0", "manufacturer": "TestManufacturer", "serialNumber": "SN001", "connectionState": "OFFLINE"})");
  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);

  // Receive ONLINE connection message
  mock_ptr_->simulate_receive(
    conn_topic,
    R"({"headerId": 2, "timestamp": "2025-01-01T00:00:01.000Z", "version": "2.0.0", "manufacturer": "TestManufacturer", "serialNumber": "SN001", "connectionState": "ONLINE"})");
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  // Operational state is still UNAVAILABLE until state message received
  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);

  // Receive state message to recover to AVAILABLE
  mock_ptr_->simulate_receive(state_topic, create_state_json());
  EXPECT_EQ(agv->get_operational_state(), AGVState::AVAILABLE);
}

// =============================================================================
// Enum Value Tests
// =============================================================================

TEST_F(AGVOperationalStateTestFixture, AGVStateEnumValues)
{
  EXPECT_NE(AGVState::STATE_UNKNOWN, AGVState::AVAILABLE);
  EXPECT_NE(AGVState::STATE_UNKNOWN, AGVState::UNAVAILABLE);
  EXPECT_NE(AGVState::STATE_UNKNOWN, AGVState::ERROR);
  EXPECT_NE(AGVState::AVAILABLE, AGVState::UNAVAILABLE);
  EXPECT_NE(AGVState::AVAILABLE, AGVState::ERROR);
  EXPECT_NE(AGVState::UNAVAILABLE, AGVState::ERROR);
}

// =============================================================================
// Thread Safety Tests
// =============================================================================

TEST_F(AGVOperationalStateTestFixture, ConcurrentOperationalStateAccessIsSafe)
{
  auto& agv = create_agv();
  std::atomic<bool> stop{false};

  std::thread reader([&]() {
    while (!stop.load())
    {
      auto op_state = agv->get_operational_state();
      (void)op_state;
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
// Initial State Tests
// =============================================================================

TEST_F(AGVOperationalStateTestFixture, InitialStatesBeforeAnyMessages)
{
  auto& agv = create_agv();

  // Both start in their initial states
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);

  agv->connect();

  // Both should remain in initial states (no messages received yet)
  // Note: connect() starts heartbeat listeners but doesn't change state
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
  EXPECT_EQ(agv->get_operational_state(), AGVState::STATE_UNKNOWN);
}

}  // namespace vda5050_master::test
