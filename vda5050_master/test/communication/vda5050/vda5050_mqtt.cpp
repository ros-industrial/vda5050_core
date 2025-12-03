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
#include <mutex>
#include <vector>

#include "../mqtt/test_helpers.hpp"
#include "vda5050_master/communication/mqtt.hpp"
#include "vda5050_master/communication/vda5050.hpp"

using vda5050_master::test::mqtt::constants::MQTT_BROKER;

class VDA5050MqttCommunicationTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    broker_endpoint_ = MQTT_BROKER;
    setup_config();
    setup_handlers();
  }

  void setup_config()
  {
    config_.connection_topic = "test/connection";
    config_.state_topic = "test/state";
    config_.factsheet_topic = "test/factsheet";
    config_.visualization_topic = "test/visualization";
    config_.order_topic = "test/order";
    config_.instant_actions_topic = "test/instant_actions";
  }

  void setup_handlers()
  {
    handlers_.on_connection = [this](const vda5050_msgs::msg::Connection& msg) {
      std::lock_guard<std::mutex> lock(mutex_);
      received_connections_.push_back(msg);
      connection_count_++;
    };

    handlers_.on_state = [this](const vda5050_msgs::msg::State& msg) {
      std::lock_guard<std::mutex> lock(mutex_);
      received_states_.push_back(msg);
      state_count_++;
    };

    handlers_.on_factsheet = [this](const vda5050_msgs::msg::Factsheet& msg) {
      std::lock_guard<std::mutex> lock(mutex_);
      received_factsheets_.push_back(msg);
    };

    handlers_.on_visualization =
      [this](const vda5050_msgs::msg::Visualization& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        received_visualizations_.push_back(msg);
      };
  }

  void TearDown() override
  {
    // Note: We don't shutdown rclcpp here to allow multiple tests to run
  }

  std::string broker_endpoint_;
  VDA5050TopicConfig config_;
  VDA5050Handlers handlers_;

  // Thread-safe storage controlled by test
  std::mutex mutex_;
  std::vector<vda5050_msgs::msg::Connection> received_connections_;
  std::vector<vda5050_msgs::msg::State> received_states_;
  std::vector<vda5050_msgs::msg::Factsheet> received_factsheets_;
  std::vector<vda5050_msgs::msg::Visualization> received_visualizations_;
  std::atomic<int> connection_count_{0};
  std::atomic<int> state_count_{0};
};

TEST_F(VDA5050MqttCommunicationTest, TopicConfigValidation)
{
  VDA5050TopicConfig config;
  config.connection_topic = "test/connection";
  config.state_topic = "test/state";
  config.factsheet_topic = "test/factsheet";
  config.visualization_topic = "test/visualization";
  config.order_topic = "test/order";
  config.instant_actions_topic = "test/instant_actions";

  ASSERT_NO_THROW(config.validate());
  ASSERT_TRUE(config.has_factsheet());
  ASSERT_TRUE(config.has_order());
  ASSERT_TRUE(config.has_instant_actions());
  ASSERT_TRUE(config.has_visualization());
}

TEST_F(VDA5050MqttCommunicationTest, TopicConfigValidationFailsOnEmptyRequired)
{
  VDA5050TopicConfig config;
  config.connection_topic = "";  // Empty - should fail
  config.state_topic = "test/state";

  ASSERT_THROW(config.validate(), std::runtime_error);
}

TEST_F(VDA5050MqttCommunicationTest, HandlersValidation)
{
  VDA5050Handlers handlers;
  handlers.on_connection = [](const vda5050_msgs::msg::Connection&) {};
  handlers.on_state = [](const vda5050_msgs::msg::State&) {};
  // on_factsheet and on_visualization have defaults, no need to set

  ASSERT_NO_THROW(handlers.validate());
}

TEST_F(VDA5050MqttCommunicationTest, HandlersValidationFailsOnMissingRequired)
{
  VDA5050Handlers handlers;
  handlers.on_connection = [](const vda5050_msgs::msg::Connection&) {};
  // on_state not set - should fail

  ASSERT_THROW(handlers.validate(), std::runtime_error);
}

TEST_F(VDA5050MqttCommunicationTest, HandlersHaveDefaultsForOptional)
{
  VDA5050Handlers handlers;
  handlers.on_connection = [](const vda5050_msgs::msg::Connection&) {};
  handlers.on_state = [](const vda5050_msgs::msg::State&) {};

  // Defaults should be set
  ASSERT_TRUE(handlers.on_factsheet != nullptr);
  ASSERT_TRUE(handlers.on_visualization != nullptr);
  ASSERT_NO_THROW(handlers.validate());
}

TEST_F(VDA5050MqttCommunicationTest, HandlersCanOverrideDefaults)
{
  VDA5050Handlers handlers;
  handlers.on_connection = [](const vda5050_msgs::msg::Connection&) {};
  handlers.on_state = [](const vda5050_msgs::msg::State&) {};

  bool custom_factsheet_called = false;
  handlers.on_factsheet =
    [&custom_factsheet_called](const vda5050_msgs::msg::Factsheet&) {
      custom_factsheet_called = true;
    };

  // Call the handler to verify override works
  vda5050_msgs::msg::Factsheet msg;
  handlers.on_factsheet(msg);
  ASSERT_TRUE(custom_factsheet_called);
}

TEST_F(VDA5050MqttCommunicationTest, Setup)
{
  auto vda_comms = std::make_unique<Vda5050Communication>(
    std::make_unique<MqttCommunication>(broker_endpoint_, "test_id"), config_,
    handlers_);

  ASSERT_NO_THROW(vda_comms->start_communication());
  ASSERT_NO_THROW(vda_comms->stop_communication());
}
