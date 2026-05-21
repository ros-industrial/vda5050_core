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

#include "vda5050_core/transport/mqtt_client_interface.hpp"

class MockMqttClient : public vda5050_core::transport::MqttClientInterface
{
public:
  MOCK_METHOD(void, connect, (), (override));
  MOCK_METHOD(void, disconnect, (), (override));
  MOCK_METHOD(bool, connected, (), (override));
  MOCK_METHOD(
    void, publish, (const std::string&, const std::string&, int, bool),
    (override));
  MOCK_METHOD(
    void, subscribe,
    (const std::string&,
     std::function<void(const std::string&, const std::string&)>, int),
    (override));
  MOCK_METHOD(void, unsubscribe, (const std::string&), (override));
  MOCK_METHOD(
    void, set_will, (const std::string&, const std::string&, int, bool),
    (override));
};

TEST(MqttClientInterfaceTest, ConnectCall)
{
  MockMqttClient mock;
  EXPECT_CALL(mock, connect()).Times(1);
  mock.connect();
}

TEST(MqttClientInterfaceTest, DisconnectCall)
{
  MockMqttClient mock;
  EXPECT_CALL(mock, disconnect()).Times(1);
  mock.disconnect();
}

TEST(MqttClientInterfaceTest, CheckConnection)
{
  MockMqttClient mock;
  EXPECT_CALL(mock, connected()).Times(1);
  mock.connected();
}

TEST(MqttClientInterfaceTest, PublishMessage)
{
  MockMqttClient mock;
  EXPECT_CALL(mock, publish("topic", "{payload: 'data'}", 0, false)).Times(1);
  mock.publish("topic", "{payload: 'data'}", 0, false);
}

TEST(MqttClientInterfaceTest, SubscribeTopic)
{
  MockMqttClient mock;
  auto cb = [](const std::string&, const std::string&) {};
  EXPECT_CALL(mock, subscribe("topic", testing::_, 0));
  mock.subscribe("topic", cb, 0);
}

TEST(MqttClientInterfaceTest, UnsubscribeTopic)
{
  MockMqttClient mock;
  EXPECT_CALL(mock, unsubscribe("topic")).Times(1);
  mock.unsubscribe("topic");
}

TEST(MqttClientInterfaceTest, SetWill)
{
  MockMqttClient mock;
  EXPECT_CALL(mock, set_will("topic", "{payload: 'data'}", 1, true)).Times(1);
  mock.set_will("topic", "{payload: 'data'}", 1, true);
}
