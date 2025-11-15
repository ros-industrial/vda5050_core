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

#include "vda5050_master/communication/heartbeat.hpp"

#include <gmock/gmock.h>

#include <chrono>
#include <thread>

#include "vda5050_master/standard_names.hpp"

class MockConnectionHeartbeatListener
: public vda5050_master::communication::ConnectionHeartbeatListener
{
public:
  MockConnectionHeartbeatListener(
    const std::string& id, const int heartbeat_interval,
    std::function<void()> disconnection_callback, int time_to_skip = 0)
  : ConnectionHeartbeatListener(id, heartbeat_interval, disconnection_callback),
    time_to_skip_(time_to_skip)
  {
    // ASSERT_NEAR(
    //   std::chrono::duration_cast<std::chrono::seconds>(
    //     get_current_time().time_since_epoch())
    //     .count(),
    //   std::chrono::duration_cast<std::chrono::seconds>(
    //     std::chrono::steady_clock::now().time_since_epoch())
    //     .count(),
    //   time_to_skip);

    // ASSERT_NEAR(
    //   std::chrono::duration_cast<std::chrono::seconds>(
    //     get_last_connection_report().time_since_epoch())
    //     .count(),
    //   std::chrono::duration_cast<std::chrono::seconds>(
    //     std::chrono::steady_clock::now().time_since_epoch())
    //     .count(),
    //   time_to_skip);

    // ASSERT_NEAR(
    //   std::chrono::duration_cast<std::chrono::seconds>(
    //     get_last_connection_report().time_since_epoch())
    //     .count(),
    //   std::chrono::duration_cast<std::chrono::seconds>(
    //     get_current_time().time_since_epoch())
    //     .count(),
    //   1e9);

    start_connection_heartbeat();
  }

  std::chrono::steady_clock::time_point get_current_time() override
  {
    VDA5050_INFO(
      "get_current_time with skip of " + std::to_string(time_to_skip_));
    return std::chrono::steady_clock::now() +
           std::chrono::seconds(time_to_skip_);
  }

  // Override check interval to speed up tests
  // Instead of waiting 15 seconds, wait only 1 second
  int get_check_interval() override
  {
    return 1;  // Check every 1 second in tests (15x faster than production)
  }

  ~MockConnectionHeartbeatListener()
  {
    VDA5050_INFO("MockConnectionHeartbeatListener destroyed");
  }

  void trigger_timeout()
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    message_received_.notify_all();
  }

  int time_to_skip_;
};

TEST(ConnectionHeartbeatListenerTest, HeartbeatListenerInit)
{
  auto hb_listener = MockConnectionHeartbeatListener(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&]() {
      // Timeout callback
      VDA5050_INFO("Timeout callback");
    },
    vda5050_master::ConnectionHeartbeatInterval - 1);

  ASSERT_TRUE(hb_listener.is_running());
  ASSERT_NO_THROW(hb_listener.stop_connection_heartbeat());
  ASSERT_FALSE(hb_listener.is_running());
}

TEST(ConnectionHeartbeatListenerTest, HeartbeatReceivedNoTimeout)
{
  auto hb_listener = MockConnectionHeartbeatListener(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&]() {
      // Timeout callback
      VDA5050_INFO("Timeout callback");
    },
    vda5050_master::ConnectionHeartbeatInterval - 1);

  ASSERT_TRUE(hb_listener.is_running());
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  hb_listener.received_connection();

  ASSERT_NEAR(
    std::chrono::duration_cast<std::chrono::seconds>(
      hb_listener.get_last_connection_report().time_since_epoch())
      .count(),
    std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now().time_since_epoch())
      .count(),
    vda5050_master::ConnectionHeartbeatInterval - 1);

  ASSERT_NO_THROW(hb_listener.stop_connection_heartbeat());
}

TEST(ConnectionHeartbeatListenerTest, HeartbeatNotReceivedTimeout)
{
  std::atomic<bool> heartbeat_failed{false};
  auto hb_listener = MockConnectionHeartbeatListener(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&heartbeat_failed]() {
      // Timeout callback
      VDA5050_INFO("Timeout callback");
      VDA5050_INFO(
        "Heartbeat_failed before store: " +
        std::to_string(heartbeat_failed.load()));
      heartbeat_failed.store(true);
      VDA5050_INFO(
        "Heartbeat_failed after store: " +
        std::to_string(heartbeat_failed.load()));
      // ASSERT_TRUE(heartbeat_failed->load());
    },
    vda5050_master::ConnectionHeartbeatInterval + 1);
  hb_listener.trigger_timeout();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_NO_THROW(hb_listener.~MockConnectionHeartbeatListener());
  ASSERT_TRUE(heartbeat_failed.load());
}

TEST(ConnectionHeartbeatListenerTest, HeartbeatReceivedTimeout)
{
  std::atomic<bool> heartbeat_failed{false};
  auto hb_listener = MockConnectionHeartbeatListener(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&heartbeat_failed]() {
      // Timeout callback
      VDA5050_INFO("Timeout callback");
      heartbeat_failed.store(true);
    },
    vda5050_master::ConnectionHeartbeatInterval + 1);

  hb_listener.trigger_timeout();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  hb_listener.received_connection();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_NO_THROW(hb_listener.~MockConnectionHeartbeatListener());
  ASSERT_TRUE(heartbeat_failed.load());
}

// TEST(ConnectionHeartbeatListenerTest, HeartbeatReceivedNoTimeout)
// {
//   std::string broker = "tcp://test.mosquitto.org:1883";
//   std::string topic = InterfaceName + "rmf2" + "/" + Version +
//     "/test_manufacturer/test_serial_number/connection";
//   std::string payload = "test";
//   int qos = 0;

//   std::atomic_bool received = false;

//   std::atomic_bool heartbeat_received = false;

//   auto listener =
//     vda5050_core::mqtt_client::create_default_client(broker, "listener");
//   ASSERT_NO_THROW(listener->connect());
//   ASSERT_NO_THROW(listener->subscribe(
//     topic,
//       [&](const std::string & topic_, const std::string & payload_) {
//         received = true;
//         ASSERT_EQ(topic, topic_);
//         ASSERT_EQ(payload, payload_);
//     },
//     qos));

//   auto hb_listener = ConnectionHeartbeatListener(
//     topic,
//     ConnectionHeartbeatInterval,
//     [&]() {
//       // Timeout callback
//       heartbeat_received = true;
//     });

//   std::this_thread::sleep_for(std::chrono::seconds(ConnectionHeartbeatInterval - 1));
//   auto talker =
//     vda5050_core::mqtt_client::create_default_client(broker, "talker");
//   ASSERT_NO_THROW(talker->connect());
//   auto talker_pub_time = std::chrono::steady_clock::now();
//   ASSERT_NO_THROW(talker->publish(topic, payload, ConnectionQos));

//   ASSERT_TRUE(received);
//   double time_diff = std::chrono::duration<double>(hb_listener.get_last_connection_report() -
//     talker_pub_time).count();
//   // ASSERT_TRUE(std::chrono::duration<long double, std::ratio<1, 1000000000>>(time_diff) < 1.0);
//   ASSERT_NEAR(time_diff, 1, 0.01);
//   ASSERT_NO_THROW(talker->disconnect());
//   ASSERT_NO_THROW(listener->disconnect());
// }
