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

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "../communication/test_helpers.hpp"
#include "mock_mqtt_client.hpp"
#include "vda5050_master/agv/agv.hpp"
#include "vda5050_master/standard_names.hpp"

namespace vda5050_master::test {

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
    auto mock = std::make_shared<MockMqttClient>();
    mock_ptr_ = mock.get();
    agv_ =
      std::make_unique<AGV>(manufacturer_, serial_number_, std::move(mock));
    return agv_;
  }

  std::unique_ptr<AGV>& create_agv_with_heartbeat_intervals(
    int state_heartbeat_interval = vda5050_master::StateHeartbeatInterval)
  {
    auto mock = std::make_shared<MockMqttClient>();
    mock_ptr_ = mock.get();
    agv_ = std::make_unique<AGV>(
      manufacturer_, serial_number_, std::move(mock),
      AGV::DEFAULT_MAX_QUEUE_SIZE, true, state_heartbeat_interval);
    return agv_;
  }

  std::unique_ptr<AGV> agv_;

  std::string create_connection_json(const std::string& state = "ONLINE")
  {
    return make_connection_json(manufacturer_, serial_number_, state);
  }

  std::string manufacturer_;
  std::string serial_number_;
  MockMqttClient* mock_ptr_ = nullptr;
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

// =============================================================================
// Component Cleanup Tests
// =============================================================================

TEST_F(AGVConnectionStateTestFixture, ComponentsCleanedUpOnOfflineMessage)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // Establish connection - this sets up AGV components
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  // Verify subscriptions are set up after ONLINE
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be subscribed after ONLINE";
  EXPECT_FALSE(mock_ptr_->find_topic_containing("factsheet").empty())
    << "Factsheet topic should be subscribed after ONLINE";
  EXPECT_FALSE(mock_ptr_->find_topic_containing("visualization").empty())
    << "Visualization topic should be subscribed after ONLINE";

  // Send OFFLINE message - should trigger cleanup
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);

  // Verify subscriptions are cleaned up after OFFLINE
  EXPECT_TRUE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be unsubscribed after OFFLINE";
  EXPECT_TRUE(mock_ptr_->find_topic_containing("factsheet").empty())
    << "Factsheet topic should be unsubscribed after OFFLINE";
  EXPECT_TRUE(mock_ptr_->find_topic_containing("visualization").empty())
    << "Visualization topic should be unsubscribed after OFFLINE";

  // Connection topic should still be subscribed for reconnection
  EXPECT_FALSE(mock_ptr_->find_topic_containing("connection").empty())
    << "Connection topic should remain subscribed";
}

TEST_F(
  AGVConnectionStateTestFixture, ComponentsCleanedUpOnConnectionBrokenMessage)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // Establish connection
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  // Verify subscriptions are set up
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty());
  EXPECT_FALSE(mock_ptr_->find_topic_containing("factsheet").empty());
  EXPECT_FALSE(mock_ptr_->find_topic_containing("visualization").empty());

  // Send CONNECTIONBROKEN message - should trigger cleanup
  mock_ptr_->simulate_receive(
    conn_topic, create_connection_json("CONNECTIONBROKEN"));
  EXPECT_EQ(
    agv->get_connection_status(),
    vda5050_types::ConnectionState::CONNECTIONBROKEN);
  EXPECT_EQ(agv->get_operational_state(), AGVState::UNAVAILABLE);

  // Verify subscriptions are cleaned up
  EXPECT_TRUE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be unsubscribed after CONNECTIONBROKEN";
  EXPECT_TRUE(mock_ptr_->find_topic_containing("factsheet").empty())
    << "Factsheet topic should be unsubscribed after CONNECTIONBROKEN";
  EXPECT_TRUE(mock_ptr_->find_topic_containing("visualization").empty())
    << "Visualization topic should be unsubscribed after CONNECTIONBROKEN";
}

TEST_F(
  AGVConnectionStateTestFixture, ComponentsReInitializedAfterOfflineThenOnline)
{
  auto& agv = create_agv();

  int callback_count = 0;
  agv->setup_subscriptions(
    [&](const std::string&, const vda5050_types::Connection&) {
      callback_count++;
    },
    nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // First ONLINE - sets up components
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_EQ(callback_count, 1);
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty());

  // OFFLINE - cleans up components
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
  EXPECT_EQ(callback_count, 2);
  EXPECT_TRUE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be unsubscribed after OFFLINE";

  // Second ONLINE - should re-initialize components
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_EQ(callback_count, 3);
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be re-subscribed after second ONLINE";
}

TEST_F(
  AGVConnectionStateTestFixture,
  ComponentsReInitializedAfterConnectionBrokenThenOnline)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // ONLINE -> verify subscriptions
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty());

  // CONNECTIONBROKEN -> verify cleanup
  mock_ptr_->simulate_receive(
    conn_topic, create_connection_json("CONNECTIONBROKEN"));
  EXPECT_EQ(
    agv->get_connection_status(),
    vda5050_types::ConnectionState::CONNECTIONBROKEN);
  EXPECT_TRUE(mock_ptr_->find_topic_containing("state").empty());

  // Second ONLINE -> verify re-initialization
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be re-subscribed after reconnection";
}

TEST_F(AGVConnectionStateTestFixture, CachedDataRetainedAfterCleanup)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // Establish connection
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);

  // Verify last_connection is cached
  auto last_conn = agv->get_last_connection();
  ASSERT_TRUE(last_conn.has_value());
  EXPECT_EQ(
    last_conn->connection_state, vda5050_types::ConnectionState::ONLINE);

  // Send OFFLINE - triggers cleanup but should retain cached data
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  // Verify cached data is still accessible (now shows OFFLINE)
  last_conn = agv->get_last_connection();
  ASSERT_TRUE(last_conn.has_value());
  EXPECT_EQ(
    last_conn->connection_state, vda5050_types::ConnectionState::OFFLINE);

  // Verify connection time is still set
  auto last_conn_time = agv->get_last_connection_time();
  EXPECT_TRUE(last_conn_time.has_value());
}

TEST_F(AGVConnectionStateTestFixture, MultipleCleanupCyclesAreSafe)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // Run multiple ONLINE/OFFLINE cycles
  for (int i = 0; i < 3; ++i)
  {
    mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
    EXPECT_EQ(
      agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
    EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty())
      << "State should be subscribed after ONLINE in cycle " << i;

    mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));
    EXPECT_EQ(
      agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);
    EXPECT_TRUE(mock_ptr_->find_topic_containing("state").empty())
      << "State should be unsubscribed after OFFLINE in cycle " << i;
  }

  // Final ONLINE to verify components can still be set up
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty());
}

TEST_F(AGVConnectionStateTestFixture, CleanupWithoutPriorSetupIsNoOp)
{
  auto& agv = create_agv();
  agv->setup_subscriptions(nullptr, nullptr, nullptr);
  agv->connect();

  std::string conn_topic = mock_ptr_->find_topic_containing("connection");
  ASSERT_FALSE(conn_topic.empty());

  // Verify no state subscription before ONLINE
  EXPECT_TRUE(mock_ptr_->find_topic_containing("state").empty());

  // Send OFFLINE without ever receiving ONLINE - should be safe (no cleanup needed)
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("OFFLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::OFFLINE);

  // Now send ONLINE - should set up components normally
  mock_ptr_->simulate_receive(conn_topic, create_connection_json("ONLINE"));
  EXPECT_EQ(
    agv->get_connection_status(), vda5050_types::ConnectionState::ONLINE);
  EXPECT_FALSE(mock_ptr_->find_topic_containing("state").empty())
    << "State topic should be subscribed after ONLINE";
}

}  // namespace vda5050_master::test
