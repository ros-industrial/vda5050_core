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

#include <atomic>
#include <chrono>
#include <thread>

#include "../test_helpers.hpp"
#include "test_helpers.hpp"
#include "vda5050_master/communication/mqtt.hpp"

// Using declarations instead of using-directives for cpplint compliance
using vda5050_master::test::make_numbered_payloads;
using vda5050_master::test::make_test_topic;
using vda5050_master::test::verify_messages_in_order;
using vda5050_master::test::wait_for_condition;
using vda5050_master::test::constants::default_payload_json;
using vda5050_master::test::mqtt::constants::is_broker_available;
using vda5050_master::test::mqtt::constants::MQTT_BROKER;
using vda5050_master::test::mqtt::constants::MQTT_POLL_INTERVAL;

class MqttCommunicationTestFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Skip tests if MQTT broker is not available
    if (!is_broker_available())
    {
      GTEST_SKIP() << "MQTT broker at " << MQTT_BROKER
                   << " is not available. Skipping test.";
    }

    broker_endpoint_ = MQTT_BROKER;
    qos_ = 0;
    container_ = std::make_shared<std::vector<std::string>>();
  }

  void TearDown() override
  {
    // Cleanup if needed
  }

  std::string get_last_subscription_message()
  {
    auto msg = *(container_->begin());
    container_->erase(container_->begin());
    return msg;
  }

  std::shared_ptr<std::vector<std::string>> container_;

  std::string broker_endpoint_;
  int qos_;
};
TEST_F(MqttCommunicationTestFixture, SubscriptionTest)
{
  std::string topic = make_test_topic("mqtt/subscription");
  std::string payload = default_payload_json().dump();

  auto mqtt_comms = MqttCommunication(broker_endpoint_, "test_id");
  ASSERT_NO_THROW(mqtt_comms.connect());
  mqtt_comms.subscribe(
    topic,
    [this](const std::string& topic, const std::string& payload) {
      // Parse JSON and call the connection callback
      container_->push_back(payload);
    },
    qos_);

  auto talker = vda5050_core::mqtt_client::create_default_client(
    broker_endpoint_, "talker");
  ASSERT_NO_THROW(talker->connect());
  ASSERT_NO_THROW(talker->publish(topic, payload, qos_));

  // Poll for message with timeout (accounting for network latency)
  bool received = wait_for_condition(
    [&]() { return container_->size() > 0; }, MQTT_POLL_INTERVAL);

  ASSERT_EQ(1, container_->size());
  container_->clear();
  ASSERT_NO_THROW(talker->disconnect());
  ASSERT_NO_THROW(mqtt_comms.disconnect());
}

TEST_F(MqttCommunicationTestFixture, PublishTest)
{
  std::string topic = make_test_topic("mqtt/publish");
  std::string payload = default_payload_json().dump();

  std::atomic_bool received = false;

  auto listener = vda5050_core::mqtt_client::create_default_client(
    broker_endpoint_, "listener");
  ASSERT_NO_THROW(listener->connect());
  ASSERT_NO_THROW(listener->subscribe(
    topic,
    [&](const std::string& topic_, const std::string& payload_) {
      received = true;
      ASSERT_EQ(topic, topic_);
      ASSERT_EQ(payload, payload_);
    },
    qos_));

  auto mqtt_comms = MqttCommunication(broker_endpoint_, "test_id");
  ASSERT_NO_THROW(mqtt_comms.connect());
  mqtt_comms.send_message(topic, payload, qos_);

  // Poll for message with timeout (accounting for network latency)
  ASSERT_TRUE(
    wait_for_condition([&]() { return received.load(); }, MQTT_POLL_INTERVAL));

  ASSERT_TRUE(received);
  ASSERT_NO_THROW(mqtt_comms.disconnect());
  ASSERT_NO_THROW(listener->disconnect());
}

TEST_F(MqttCommunicationTestFixture, MultipleMessagesTest)
{
  std::string topic = make_test_topic("mqtt/multiple");
  auto payloads = make_numbered_payloads(3);

  // Create node for Ros2Communication
  auto mqtt_comms = MqttCommunication(broker_endpoint_, "test_id");
  ASSERT_NO_THROW(mqtt_comms.connect());
  mqtt_comms.subscribe(
    topic,
    [this](const std::string& topic, const std::string& payload) {
      // Parse JSON and call the connection callback
      container_->push_back(payload);
    },
    qos_);

  auto talker = vda5050_core::mqtt_client::create_default_client(
    broker_endpoint_, "talker");
  ASSERT_NO_THROW(talker->connect());

  // Publish all messages
  for (const auto& payload : payloads)
  {
    ASSERT_NO_THROW(talker->publish(topic, payload, qos_));
  }

  // Wait for all messages
  bool all_received = wait_for_condition(
    [&]() { return container_->size() >= payloads.size(); },
    MQTT_POLL_INTERVAL);

  ASSERT_TRUE(all_received);

  // Use helper to verify messages in order
  verify_messages_in_order(
    [&]() { return get_last_subscription_message(); }, payloads,
    [&](size_t expected) { ASSERT_EQ(expected, container_->size()); });

  ASSERT_NO_THROW(mqtt_comms.disconnect());
}

// =============================================================================
// Lifecycle State Tests
// =============================================================================

TEST_F(MqttCommunicationTestFixture, InitialStateIsDisconnected)
{
  // Before connect() is called, should be in DISCONNECTED state
  auto mqtt_comms = MqttCommunication(broker_endpoint_, "test_initial_state");

  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED)
    << "Initial state should be DISCONNECTED";
}

TEST_F(MqttCommunicationTestFixture, ConnectedStateAfterConnect)
{
  // After connect() is called, should be in CONNECTED state
  auto mqtt_comms = MqttCommunication(broker_endpoint_, "test_connected_state");

  ASSERT_NO_THROW(mqtt_comms.connect());

  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::CONNECTED)
    << "State should be CONNECTED after connect()";

  ASSERT_NO_THROW(mqtt_comms.disconnect());
}

TEST_F(MqttCommunicationTestFixture, DisconnectedStateAfterDisconnect)
{
  // After disconnect() completes, should be in DISCONNECTED state
  auto mqtt_comms =
    MqttCommunication(broker_endpoint_, "test_disconnected_state");

  ASSERT_NO_THROW(mqtt_comms.connect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::CONNECTED);

  ASSERT_NO_THROW(mqtt_comms.disconnect());

  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED)
    << "State should be DISCONNECTED after disconnect()";
}

TEST_F(MqttCommunicationTestFixture, ReconnectAfterDisconnect)
{
  // Verify state transitions work correctly on reconnect
  auto mqtt_comms = MqttCommunication(broker_endpoint_, "test_reconnect");

  // First connection cycle
  ASSERT_NO_THROW(mqtt_comms.connect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::CONNECTED);

  ASSERT_NO_THROW(mqtt_comms.disconnect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED);

  // Second connection cycle - should work the same
  ASSERT_NO_THROW(mqtt_comms.connect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::CONNECTED)
    << "State should be CONNECTED after reconnect";

  ASSERT_NO_THROW(mqtt_comms.disconnect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED);
}

TEST_F(MqttCommunicationTestFixture, MultipleDisconnectCallsSafe)
{
  // Verify multiple disconnect() calls don't cause issues
  auto mqtt_comms =
    MqttCommunication(broker_endpoint_, "test_multi_disconnect");

  ASSERT_NO_THROW(mqtt_comms.connect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::CONNECTED);

  // First disconnect
  ASSERT_NO_THROW(mqtt_comms.disconnect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED);

  // Second disconnect - should not throw or change state
  ASSERT_NO_THROW(mqtt_comms.disconnect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED);

  // Third disconnect - still safe
  ASSERT_NO_THROW(mqtt_comms.disconnect());
  ASSERT_EQ(mqtt_comms.get_state(), ConnectionState::DISCONNECTED);
}
