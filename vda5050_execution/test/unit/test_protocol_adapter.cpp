/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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
#include <memory>
#include <string>

#include <vda5050_core/mqtt_client/mqtt_client_interface.hpp>

#include <vda5050_types/connection.hpp>

#include "vda5050_execution/protocol_adapter.hpp"

using vda5050_types::Connection;
using vda5050_types::Order;
using vda5050_types::State;

using MessageTypes = testing::Types<Connection, Order, State>;

class MockMqttClient : public vda5050_core::mqtt_client::MqttClientInterface
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
    void, set_will, (const std::string&, const std::string&, int), (override));
};

template <typename T>
class ProtocolAdapterTest : public testing::Test
{
protected:
  std::shared_ptr<MockMqttClient> mock_;
  std::shared_ptr<vda5050_execution::ProtocolAdapter> adapter_;

  std::string interface_;
  std::string version_;
  std::string manufacturer_;
  std::string serial_number_;

  std::string topic_prefix_;

  int qos_;
  bool retained_;

  void SetUp()
  {
    interface_ = "uagv";
    version_ = "v2";
    manufacturer_ = "ROS-I";
    serial_number_ = "S001";

    mock_ = std::make_shared<MockMqttClient>();

    adapter_ = vda5050_execution::ProtocolAdapter::make(
      mock_, interface_, version_, manufacturer_, serial_number_);

    topic_prefix_ = fmt::format(
      "{}/{}/{}/{}/", interface_, version_, manufacturer_, serial_number_);

    qos_ = 0;
    retained_ = false;
  }

  void TearDown()
  {
    mock_.reset();
    adapter_.reset();
  }
};

TYPED_TEST_SUITE(ProtocolAdapterTest, MessageTypes);

TYPED_TEST(ProtocolAdapterTest, PublishMessage)
{
  TypeParam msg;

  EXPECT_CALL(
    *this->mock_, publish(
                    testing::StartsWith(this->topic_prefix_), testing::_,
                    this->qos_, this->retained_))
    .WillOnce([&](
                const std::string& /*topic*/, const std::string& message,
                int /*qos*/, bool /*retained*/) {
      auto j = nlohmann::json::parse(message);

      EXPECT_EQ(j["headerId"], 0);
      EXPECT_EQ(j["version"], this->version_);
      EXPECT_EQ(j["manufacturer"], this->manufacturer_);
      EXPECT_EQ(j["serialNumber"], this->serial_number_);
    });

  this->adapter_->template publish<TypeParam>(msg, this->qos_, this->retained_);
}

TYPED_TEST(ProtocolAdapterTest, SubscribeMessage)
{
  vda5050_core::mqtt_client::MqttClientInterface::MessageHandler
    captured_handler;

  EXPECT_CALL(
    *this->mock_,
    subscribe(testing::StartsWith(this->topic_prefix_), testing::_, this->qos_))
    .WillOnce(testing::SaveArg<1>(&captured_handler));

  std::atomic_bool success = false;

  this->adapter_->template subscribe<TypeParam>(
    [&](TypeParam /*msg*/, std::optional<vda5050_types::Error> err) {
      if (!err.has_value()) success = true;
    },
    this->qos_);

  TypeParam msg;
  nlohmann::json j = msg;

  captured_handler(this->topic_prefix_, j.dump());
  EXPECT_TRUE(success);
}

TYPED_TEST(ProtocolAdapterTest, HeaderIncrement)
{
  TypeParam msg;

  std::atomic_uint32_t header_count = 0;

  EXPECT_CALL(
    *this->mock_, publish(
                    testing::StartsWith(this->topic_prefix_), testing::_,
                    this->qos_, this->retained_))
    .WillRepeatedly([&](
                      const std::string& /*topic*/, const std::string& message,
                      int /*qos*/, bool /*retained*/) {
      auto j = nlohmann::json::parse(message);

      EXPECT_EQ(j["headerId"].get<uint32_t>(), header_count);
      header_count++;
    });

  this->adapter_->template publish<TypeParam>(msg, this->qos_, this->retained_);
  this->adapter_->template publish<TypeParam>(msg, this->qos_, this->retained_);
}
