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

#include <chrono>
#include <thread>

#include "../test_helpers.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "test_helpers.hpp"
#include "vda5050_master/communication/ros2.hpp"
namespace {
auto payload_json = R"(
  {
    "happy": true,
    "pi": 3.141
  }
)"_json;
}  // namespace

// Using declarations instead of using-directives for cpplint compliance
using vda5050_master::test::make_numbered_payloads;
using vda5050_master::test::make_test_topic;
using vda5050_master::test::verify_messages_in_order;
using vda5050_master::test::constants::default_payload_json;
using vda5050_master::test::ros2::publish_messages;
using vda5050_master::test::ros2::TestExecutor;
using vda5050_master::test::ros2::wait_for_condition_with_spin;
using vda5050_master::test::ros2::wait_for_discovery;
using vda5050_master::test::ros2::wait_for_publisher_discovery;

class Ros2CommunicationTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr);
    }
    qos_ = 0;
    container_ = std::make_shared<std::vector<std::string>>();
  }

  void TearDown() override
  {
    // Note: We don't shutdown rclcpp here to allow multiple tests to run
  }

  std::string get_last_subscription_message()
  {
    auto msg = *(container_->begin());
    container_->erase(container_->begin());
    return msg;
  }
  std::shared_ptr<std::vector<std::string>> container_;

  int qos_;
};

TEST_F(Ros2CommunicationTest, SubscriptionTest)
{
  std::string topic = make_test_topic("ros2/subscription");
  std::string payload = default_payload_json().dump();

  // Create node for Ros2Communication
  auto test_node = std::make_shared<rclcpp::Node>("test_ros2_subscriber");
  auto ros2_comms = Ros2Communication(test_node, "test_id");
  ASSERT_NO_THROW(ros2_comms.connect());
  ros2_comms.subscribe(
    topic,
    [this](const std::string& topic, const std::string& payload) {
      // Parse JSON and call the connection callback
      container_->push_back(payload);
    },
    qos_);

  // Create a separate publisher node
  auto talker_node = std::make_shared<rclcpp::Node>("test_talker");
  auto talker_pub =
    talker_node->create_publisher<std_msgs::msg::String>(topic, 10);

  // Create executor to spin both nodes
  TestExecutor executor;
  executor.add_node(test_node);
  executor.add_node(talker_node);

  // Publish message
  auto msg = std_msgs::msg::String();
  msg.data = payload;
  talker_pub->publish(msg);

  bool received = executor.wait_for([&]() { return container_->size() > 0; });

  // Verify message was received
  ASSERT_TRUE(received);
  ASSERT_EQ(payload, get_last_subscription_message());
  ASSERT_EQ(0, container_->size());

  ASSERT_NO_THROW(ros2_comms.disconnect());
}

TEST_F(Ros2CommunicationTest, PublishTest)
{
  std::string topic = make_test_topic("ros2/publish");
  std::string payload = default_payload_json().dump();

  std::atomic_bool ros2_received = false;
  std::string received_payload;

  // Create listener node with subscription
  auto listener_node = std::make_shared<rclcpp::Node>("test_listener");
  auto listener_sub = listener_node->create_subscription<std_msgs::msg::String>(
    topic, rclcpp::QoS(10).best_effort().durability_volatile(),
    [&](const std_msgs::msg::String::SharedPtr msg) {
      ros2_received = true;
      received_payload = msg->data;
    });

  // Create Ros2Communication instance
  auto publisher_node = std::make_shared<rclcpp::Node>("test_ros2_publisher");
  auto ros2_comms = Ros2Communication(publisher_node, "test_id");
  ASSERT_NO_THROW(ros2_comms.connect());

  TestExecutor executor;
  executor.add_node(listener_node);
  executor.add_node(publisher_node);

  // Publish message (publisher will be created and discovery will happen)
  ros2_comms.send_message(topic, payload, qos_);

  // Wait for message to be received (allows time for discovery and transmission)
  ASSERT_TRUE(executor.wait_for([&]() { return ros2_received.load(); }));
  ASSERT_EQ(payload, received_payload);
  ASSERT_NO_THROW(ros2_comms.disconnect());
}

TEST_F(Ros2CommunicationTest, MultipleMessagesTest)
{
  std::string topic = make_test_topic("ros2/multiple");
  auto payloads = make_numbered_payloads(3);  // Helper generates test data

  // Create node for Ros2Communication
  auto test_node = std::make_shared<rclcpp::Node>("test_ros2_multi");
  auto ros2_comms = Ros2Communication(test_node, "test_multi");
  ASSERT_NO_THROW(ros2_comms.connect());
  ros2_comms.subscribe(
    topic,
    [this](const std::string& topic, const std::string& payload) {
      // Parse JSON and call the connection callback
      container_->push_back(payload);
    },
    qos_);

  // Create publisher node
  auto talker_node = std::make_shared<rclcpp::Node>("test_talker_multi");
  auto talker_pub =
    talker_node->create_publisher<std_msgs::msg::String>(topic, 10);

  // Create executor
  TestExecutor executor;
  executor.add_node(test_node);
  executor.add_node(talker_node);

  // Use helper to publish multiple messages
  publish_messages(talker_pub, payloads);

  // Wait for all messages using helper
  bool all_received =
    executor.wait_for([&]() { return container_->size() >= payloads.size(); });
  ASSERT_TRUE(all_received);

  // Use helper to verify messages in order
  verify_messages_in_order(
    [&]() { return get_last_subscription_message(); }, payloads,
    [&](size_t expected) { ASSERT_EQ(expected, container_->size()); });

  ASSERT_NO_THROW(ros2_comms.disconnect());
}

// =============================================================================
// Lifecycle State Tests
// =============================================================================

TEST_F(Ros2CommunicationTest, InitialStateIsDisconnected)
{
  // Before connect() is called, should be in DISCONNECTED state
  auto test_node = std::make_shared<rclcpp::Node>("test_initial_state");
  auto ros2_comms = Ros2Communication(test_node, "test_id");

  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED)
    << "Initial state should be DISCONNECTED";
}

TEST_F(Ros2CommunicationTest, ConnectedStateAfterConnect)
{
  // After connect() is called, should be in CONNECTED state
  auto test_node = std::make_shared<rclcpp::Node>("test_connected_state");
  auto ros2_comms = Ros2Communication(test_node, "test_id");

  ASSERT_NO_THROW(ros2_comms.connect());

  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::CONNECTED)
    << "State should be CONNECTED after connect()";

  ASSERT_NO_THROW(ros2_comms.disconnect());
}

TEST_F(Ros2CommunicationTest, DisconnectedStateAfterDisconnect)
{
  // After disconnect() completes, should be in DISCONNECTED state
  auto test_node = std::make_shared<rclcpp::Node>("test_disconnected_state");
  auto ros2_comms = Ros2Communication(test_node, "test_id");

  ASSERT_NO_THROW(ros2_comms.connect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::CONNECTED);

  ASSERT_NO_THROW(ros2_comms.disconnect());

  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED)
    << "State should be DISCONNECTED after disconnect()";
}

TEST_F(Ros2CommunicationTest, ReconnectAfterDisconnect)
{
  // Verify state transitions work correctly on reconnect
  auto test_node = std::make_shared<rclcpp::Node>("test_reconnect");
  auto ros2_comms = Ros2Communication(test_node, "test_id");

  // First connection cycle
  ASSERT_NO_THROW(ros2_comms.connect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::CONNECTED);

  ASSERT_NO_THROW(ros2_comms.disconnect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED);

  // Second connection cycle - should work the same
  ASSERT_NO_THROW(ros2_comms.connect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::CONNECTED)
    << "State should be CONNECTED after reconnect";

  ASSERT_NO_THROW(ros2_comms.disconnect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED);
}

TEST_F(Ros2CommunicationTest, SendMessageFailsWhenNotConnected)
{
  // Verify send_message doesn't crash when not connected
  auto test_node = std::make_shared<rclcpp::Node>("test_send_not_connected");
  auto ros2_comms = Ros2Communication(test_node, "test_id");

  // Should not throw, just log warning
  ASSERT_NO_THROW(ros2_comms.send_message("/test/topic", "test_payload", 0));

  // Also test after disconnect
  ASSERT_NO_THROW(ros2_comms.connect());
  ASSERT_NO_THROW(ros2_comms.disconnect());
  ASSERT_NO_THROW(ros2_comms.send_message("/test/topic", "test_payload", 0));
}

TEST_F(Ros2CommunicationTest, MultipleDisconnectCallsSafe)
{
  // Verify multiple disconnect() calls don't cause issues
  auto test_node = std::make_shared<rclcpp::Node>("test_multi_disconnect");
  auto ros2_comms = Ros2Communication(test_node, "test_id");

  ASSERT_NO_THROW(ros2_comms.connect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::CONNECTED);

  // First disconnect
  ASSERT_NO_THROW(ros2_comms.disconnect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED);

  // Second disconnect - should not throw or change state
  ASSERT_NO_THROW(ros2_comms.disconnect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED);

  // Third disconnect - still safe
  ASSERT_NO_THROW(ros2_comms.disconnect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::DISCONNECTED);
}

TEST_F(Ros2CommunicationTest, SubscribeWithInvalidTopicThrows)
{
  // ROS2 throws for invalid topic names
  auto test_node = std::make_shared<rclcpp::Node>("test_invalid_topic");
  auto ros2_comms = Ros2Communication(test_node, "test_id");
  ASSERT_NO_THROW(ros2_comms.connect());

  // Invalid topic name (contains invalid characters) should throw
  ASSERT_THROW(
    ros2_comms.subscribe(
      "invalid topic with spaces",
      [](const std::string&, const std::string&) {}, 0),
    std::exception)
    << "subscribe() should throw for invalid topic names";
}

TEST_F(Ros2CommunicationTest, PublishWithInvalidTopicThrows)
{
  // ROS2 throws for invalid topic names when creating publisher
  auto test_node = std::make_shared<rclcpp::Node>("test_invalid_pub_topic");
  auto ros2_comms = Ros2Communication(test_node, "test_id");
  ASSERT_NO_THROW(ros2_comms.connect());

  // Invalid topic name should throw when trying to publish
  ASSERT_THROW(
    ros2_comms.send_message("invalid topic with spaces", "{}", 0),
    std::exception)
    << "send_message() should throw for invalid topic names";
}

TEST_F(Ros2CommunicationTest, SendMessageAfterNodeShutdownThrows)
{
  // Create a node with its own context so we can shutdown independently
  auto context = std::make_shared<rclcpp::Context>();
  context->init(0, nullptr);

  rclcpp::NodeOptions options;
  options.context(context);
  auto test_node =
    std::make_shared<rclcpp::Node>("test_shutdown_node", options);

  auto ros2_comms = Ros2Communication(test_node, "test_id");
  ASSERT_NO_THROW(ros2_comms.connect());
  ASSERT_EQ(ros2_comms.get_state(), ConnectionState::CONNECTED);

  // Shutdown the context externally - state is still CONNECTED
  context->shutdown("test shutdown");

  // Attempting to send should throw since node context is shutdown
  ASSERT_THROW(ros2_comms.send_message("/test/topic", "{}", 0), std::exception)
    << "send_message() should throw when node context is shutdown";
}

// TEST_F(Ros2CommunicationTest, QoSReliableTest)
// {
//   std::string topic = "/test/ros2/qos_reliable";
//   std::string payload = payload_json.dump();
//   int qos = 1;  // Reliable QoS

//   // Create node for Ros2Communication with QoS 1 (reliable)
//   auto test_node = std::make_shared<rclcpp::Node>("test_ros2_qos");
//   auto ros2_comms = Ros2Communication(test_node, "test_qos");
//   ASSERT_NO_THROW(ros2_comms.connect());
//   ros2_comms.subscribe(topic, qos);

//   // Create a separate publisher node with matching QoS
//   auto talker_node = std::make_shared<rclcpp::Node>("test_talker_qos");
//   auto talker_pub = talker_node->create_publisher<std_msgs::msg::String>(
//     topic, rclcpp::QoS(10).reliable());

//   // Create executor
//   rclcpp::executors::SingleThreadedExecutor executor;
//   executor.add_node(test_node);
//   executor.add_node(talker_node);

//   // Publish message
//   auto msg = std_msgs::msg::String();
//   msg.data = payload;
//   talker_pub->publish(msg);

//   // Spin to process callbacks
//   auto start_time = std::chrono::steady_clock::now();
//   while (ros2_comms.storage_[topic].empty() &&
//          (std::chrono::steady_clock::now() - start_time) <
//            std::chrono::seconds(2)) {
//     executor.spin_some(std::chrono::milliseconds(100));
//   }

//   // Verify message was received
//   ASSERT_EQ(1, ros2_comms.storage_[topic].size());
//   ASSERT_EQ(payload, ros2_comms.get_last_subscription_message(topic));

//   ASSERT_NO_THROW(ros2_comms.disconnect());
// }
