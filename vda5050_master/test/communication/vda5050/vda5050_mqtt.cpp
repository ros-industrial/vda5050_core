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
  }

  void setup_config()
  {
    config_ = std::make_unique<VDA5050Config>();
    config_->connection_topic = "test/connection";
    config_->factsheet_topic = "test/factsheet";
    config_->order_topic = "test/order";
    config_->instant_actions_topic = "test/instant_actions";
    config_->visualization_topic = "test/visualization";
    config_->state_topic = "test/state";
    config_->connection_container =
      std::make_shared<std::vector<vda5050_msgs::msg::Connection>>();
    config_->state_container =
      std::make_shared<std::vector<vda5050_msgs::msg::State>>();
    config_->factsheet_container =
      std::make_shared<std::vector<vda5050_msgs::msg::Factsheet>>();
    config_->order_container =
      std::make_shared<std::vector<vda5050_msgs::msg::Order>>();
    config_->visualization_container =
      std::make_shared<std::vector<vda5050_msgs::msg::Visualization>>();
    config_->instant_actions_container =
      std::make_shared<std::vector<vda5050_msgs::msg::InstantActions>>();
  }

  void TearDown() override
  {
    // Note: We don't shutdown rclcpp here to allow multiple tests to run
  }
  std::string broker_endpoint_;
  std::unique_ptr<VDA5050Config> config_;
};

TEST_F(VDA5050MqttCommunicationTest, ConfigTest)
{
  auto config = std::make_unique<VDA5050Config>();
  config->connection_topic = "test/connection";
  config->factsheet_topic = "test/factsheet";
  config->order_topic = "test/order";
  config->instant_actions_topic = "test/instant_actions";
  config->visualization_topic = "test/visualization";
  config->state_topic = "test/state";
  config->connection_container =
    std::make_shared<std::vector<vda5050_msgs::msg::Connection>>();
  config->state_container =
    std::make_shared<std::vector<vda5050_msgs::msg::State>>();
  config->factsheet_container =
    std::make_shared<std::vector<vda5050_msgs::msg::Factsheet>>();
  config->order_container =
    std::make_shared<std::vector<vda5050_msgs::msg::Order>>();
  config->visualization_container =
    std::make_shared<std::vector<vda5050_msgs::msg::Visualization>>();
  config->instant_actions_container =
    std::make_shared<std::vector<vda5050_msgs::msg::InstantActions>>();
  ASSERT_NO_THROW(config->validate());
  ASSERT_TRUE(config->has_factsheet());
  ASSERT_TRUE(config->has_order());
  ASSERT_TRUE(config->has_instant_actions());
  ASSERT_TRUE(config->has_visualization());
}

TEST_F(VDA5050MqttCommunicationTest, Setup)
{
  std::unique_ptr<Vda5050Communication> vda_comms =
    std::make_unique<Vda5050Communication>(
      std::make_unique<MqttCommunication>(broker_endpoint_, "test_id"),
      std::move(config_));

  ASSERT_NO_THROW(vda_comms->start_communication());
  ASSERT_NO_THROW(vda_comms->stop_communication());
}
