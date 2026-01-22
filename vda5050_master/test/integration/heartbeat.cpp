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

#include <atomic>
#include <chrono>
#include <thread>

#include "vda5050_master/standard_names.hpp"

using vda5050_master::communication::HeartbeatState;

class MockHeartbeatListener
: public vda5050_master::communication::HeartbeatListener
{
public:
  MockHeartbeatListener(
    const std::string& id, const int heartbeat_interval,
    std::function<void()> disconnection_callback, int time_to_skip = 0)
  : HeartbeatListener(id, heartbeat_interval, disconnection_callback),
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

  ~MockHeartbeatListener()
  {
    VDA5050_INFO("MockHeartbeatListener destroyed");
  }

  void trigger_timeout()
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    message_received_.notify_all();
  }

  int time_to_skip_;
};

TEST(HeartbeatListenerTest, HeartbeatListenerInit)
{
  auto hb_listener = MockHeartbeatListener(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&]() {
      // Timeout callback
      VDA5050_INFO("Timeout callback");
    },
    vda5050_master::ConnectionHeartbeatInterval - 1);

  ASSERT_EQ(hb_listener.get_state(), HeartbeatState::RUNNING);
  ASSERT_NO_THROW(hb_listener.stop_connection_heartbeat());
  ASSERT_EQ(hb_listener.get_state(), HeartbeatState::STOPPED);
}

TEST(HeartbeatListenerTest, HeartbeatReceivedNoTimeout)
{
  auto hb_listener = MockHeartbeatListener(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&]() {
      // Timeout callback
      VDA5050_INFO("Timeout callback");
    },
    vda5050_master::ConnectionHeartbeatInterval - 1);

  ASSERT_EQ(hb_listener.get_state(), HeartbeatState::RUNNING);
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

TEST(HeartbeatListenerTest, HeartbeatNotReceivedTimeout)
{
  std::atomic<bool> heartbeat_failed{false};
  auto hb_listener = MockHeartbeatListener(
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
  ASSERT_NO_THROW(hb_listener.~MockHeartbeatListener());
  ASSERT_TRUE(heartbeat_failed.load());
}

TEST(HeartbeatListenerTest, HeartbeatReceivedTimeout)
{
  std::atomic<bool> heartbeat_failed{false};
  auto hb_listener = MockHeartbeatListener(
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
  ASSERT_NO_THROW(hb_listener.~MockHeartbeatListener());
  ASSERT_TRUE(heartbeat_failed.load());
}

TEST(HeartbeatListenerTest, GracefulShutdownDoesNotBlock)
{
  std::atomic<bool> callback_called{false};

  // Use a short interval but time_to_skip that won't cause immediate timeout
  auto hb_listener = std::make_unique<MockHeartbeatListener>(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&callback_called]() { callback_called.store(true); },
    0);  // time_to_skip = 0, so no immediate timeout

  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::RUNNING);

  // This should complete quickly, not block forever
  auto start = std::chrono::steady_clock::now();
  hb_listener->stop_connection_heartbeat();
  auto elapsed = std::chrono::steady_clock::now() - start;

  // Should complete within 2 seconds (1 second check interval + margin)
  ASSERT_LT(
    std::chrono::duration_cast<std::chrono::seconds>(elapsed).count(), 2)
    << "stop_connection_heartbeat() blocked too long";

  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::STOPPED);
  ASSERT_FALSE(callback_called.load())
    << "Callback should NOT be called during graceful shutdown";
}

TEST(HeartbeatListenerTest, StateIsRunningWhileCallbackExecutes)
{
  std::atomic<bool> callback_started{false};
  std::atomic<bool> callback_finished{false};
  std::atomic<bool> was_running_during_callback{false};

  // We need a raw pointer to check get_state() from within callback
  MockHeartbeatListener* listener_ptr = nullptr;

  auto hb_listener = std::make_unique<MockHeartbeatListener>(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    [&callback_started, &callback_finished, &was_running_during_callback,
     &listener_ptr]() {
      callback_started.store(true);

      // Check get_state() during callback execution
      if (listener_ptr)
      {
        was_running_during_callback.store(
          listener_ptr->get_state() == HeartbeatState::RUNNING);
      }

      // Simulate work
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      callback_finished.store(true);
    },
    vda5050_master::ConnectionHeartbeatInterval +
      1);  // Causes immediate timeout

  listener_ptr = hb_listener.get();

  // Trigger the timeout
  hb_listener->trigger_timeout();

  // Wait for callback to finish
  while (!callback_finished.load())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Now stop - should be quick since callback already finished
  hb_listener->stop_connection_heartbeat();

  ASSERT_TRUE(callback_started.load()) << "Callback should have started";
  ASSERT_TRUE(callback_finished.load()) << "Callback should have finished";
  ASSERT_TRUE(was_running_during_callback.load())
    << "get_state() should return RUNNING while callback is executing";
  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::STOPPED)
    << "get_state() should return STOPPED after stop completes";
}

TEST(HeartbeatListenerTest, StateIsStoppedOnlyAfterFullStop)
{
  std::atomic<bool> stop_initiated{false};
  std::atomic<bool> stop_completed{false};
  std::atomic<HeartbeatState> state_during_stop{HeartbeatState::RUNNING};

  auto hb_listener = std::make_unique<MockHeartbeatListener>(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    []() { /* No-op callback */ },
    0);  // No immediate timeout

  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::RUNNING);

  // Start stop in a separate thread so we can observe state
  std::thread stop_thread([&]() {
    stop_initiated.store(true);
    hb_listener->stop_connection_heartbeat();
    stop_completed.store(true);
  });

  // Wait for stop to be initiated
  while (!stop_initiated.load())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // Give time for stop to progress
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  if (!stop_completed.load())
  {
    state_during_stop.store(hb_listener->get_state());
  }

  stop_thread.join();

  // After stop completes, must be STOPPED
  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::STOPPED)
    << "get_state() must be STOPPED after stop_connection_heartbeat() returns";
}

TEST(HeartbeatListenerTest, MultipleStopCallsSafe)
{
  auto hb_listener = std::make_unique<MockHeartbeatListener>(
    "test_listener", vda5050_master::ConnectionHeartbeatInterval,
    []() { /* No-op */ }, 0);

  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::RUNNING);

  ASSERT_NO_THROW(hb_listener->stop_connection_heartbeat());
  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::STOPPED);

  ASSERT_NO_THROW(hb_listener->stop_connection_heartbeat());
  ASSERT_EQ(hb_listener->get_state(), HeartbeatState::STOPPED);

  ASSERT_NO_THROW(hb_listener.reset());
}

// TEST(HeartbeatListenerTest, HeartbeatReceivedNoTimeout)
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

//   auto hb_listener = HeartbeatListener(
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
