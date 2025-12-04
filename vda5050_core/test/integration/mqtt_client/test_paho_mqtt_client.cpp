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

#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"

TEST(PahoMqttClientTest, PublishSubscribe)
{
  std::string broker = "tcp://test.mosquitto.org:1883";
  std::string topic = "/test/integration";
  std::string payload = "hello";
  int qos = 0;

  std::atomic_bool received = false;

  auto listener =
    vda5050_core::mqtt_client::create_default_client(broker, "listener");
  ASSERT_NO_THROW(listener->connect());
  ASSERT_NO_THROW(listener->subscribe(
    topic,
    [&](const std::string& topic_, const std::string& payload_) {
      received = true;
      ASSERT_EQ(topic, topic_);
      ASSERT_EQ(payload, payload_);
    },
    qos));

  auto talker =
    vda5050_core::mqtt_client::create_default_client(broker, "talker");
  ASSERT_NO_THROW(talker->connect());
  ASSERT_NO_THROW(talker->publish(topic, payload, qos));

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  ASSERT_TRUE(received);

  ASSERT_NO_THROW(talker->disconnect());
  ASSERT_NO_THROW(listener->disconnect());
}

TEST(PahoMqttClientTest, UnsubscribeStopsMessages)
{
  std::string broker = "tcp://test.mosquitto.org:1883";
  std::string topic = "/test/integration/unsubscribe";
  std::string payload = "hello";
  int qos = 0;

  std::atomic_int message_count{0};

  auto listener =
    vda5050_core::mqtt_client::create_default_client(broker, "unsub_listener");
  ASSERT_NO_THROW(listener->connect());
  ASSERT_NO_THROW(listener->subscribe(
    topic,
    [&](const std::string& /*topic_*/, const std::string& /*payload_*/) {
      message_count++;
    },
    qos));

  auto talker =
    vda5050_core::mqtt_client::create_default_client(broker, "unsub_talker");
  ASSERT_NO_THROW(talker->connect());

  // Publish first message and verify it is received
  ASSERT_NO_THROW(talker->publish(topic, payload, qos));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  ASSERT_EQ(message_count.load(), 1);

  // Unsubscribe from the topic
  ASSERT_NO_THROW(listener->unsubscribe(topic));
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Publish second message after unsubscribe
  ASSERT_NO_THROW(talker->publish(topic, payload, qos));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Message count should still be 1 (no new messages received)
  ASSERT_EQ(message_count.load(), 1);

  ASSERT_NO_THROW(talker->disconnect());
  ASSERT_NO_THROW(listener->disconnect());
}
