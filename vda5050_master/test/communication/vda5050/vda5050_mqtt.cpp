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
#include "vda5050_master/vda5050_master/master.hpp"

using vda5050_master::test::mqtt::constants::MQTT_BROKER;

/**
 * @brief Test implementation of VDA5050Master for unit testing
 *
 * This class provides a concrete implementation of the abstract VDA5050Master
 * that stores received messages for test verification.
 */
class TestVDA5050Master : public VDA5050Master
{
public:
  explicit TestVDA5050Master(
    std::unique_ptr<ICommunicationStrategy> communication)
  : VDA5050Master(std::move(communication))
  {
  }

  // Thread-safe storage for received messages
  std::mutex mutex_;
  std::vector<std::pair<std::string, vda5050_msgs::msg::Connection>>
    received_connections_;
  std::vector<std::pair<std::string, vda5050_msgs::msg::State>>
    received_states_;
  std::vector<std::pair<std::string, vda5050_msgs::msg::Factsheet>>
    received_factsheets_;
  std::vector<std::pair<std::string, vda5050_msgs::msg::Visualization>>
    received_visualizations_;
  std::atomic<int> connection_count_{0};
  std::atomic<int> state_count_{0};

protected:
  void on_connection(
    const std::string& agv_id,
    const vda5050_msgs::msg::Connection& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_connections_.push_back({agv_id, msg});
    connection_count_++;
  }

  void on_state(
    const std::string& agv_id, const vda5050_msgs::msg::State& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_states_.push_back({agv_id, msg});
    state_count_++;
  }

  void on_factsheet(
    const std::string& agv_id, const vda5050_msgs::msg::Factsheet& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_factsheets_.push_back({agv_id, msg});
  }

  void on_visualization(
    const std::string& agv_id,
    const vda5050_msgs::msg::Visualization& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_visualizations_.push_back({agv_id, msg});
  }
};

class VDA5050MasterTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    broker_endpoint_ = MQTT_BROKER;
  }

  void TearDown() override
  {
    // Note: We don't shutdown rclcpp here to allow multiple tests to run
  }

  std::string broker_endpoint_;
};

TEST_F(VDA5050MasterTest, TopicConfigValidation)
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

TEST_F(VDA5050MasterTest, TopicConfigValidationFailsOnEmptyRequired)
{
  VDA5050TopicConfig config;
  config.connection_topic = "";  // Empty - should fail
  config.state_topic = "test/state";

  ASSERT_THROW(config.validate(), std::runtime_error);
}

TEST_F(VDA5050MasterTest, Setup)
{
  auto master = std::make_unique<TestVDA5050Master>(
    std::make_unique<MqttCommunication>(broker_endpoint_, "test_id"));

  ASSERT_NO_THROW(master->connect());

  AGVInfo agv_info{"test_manufacturer", "test_serial"};
  ASSERT_NO_THROW(master->register_agv(agv_info));
  ASSERT_TRUE(
    master->is_agv_registered(agv_info.manufacturer, agv_info.serial_number));

  ASSERT_NO_THROW(master->disconnect());
}

TEST_F(VDA5050MasterTest, AGVRegistrationAndUnregistration)
{
  auto master = std::make_unique<TestVDA5050Master>(
    std::make_unique<MqttCommunication>(broker_endpoint_, "test_id"));

  master->connect();

  AGVInfo agv1{"manufacturer1", "serial1"};
  AGVInfo agv2{"manufacturer2", "serial2"};

  // Register AGVs
  master->register_agv(agv1);
  master->register_agv(agv2);

  ASSERT_TRUE(master->is_agv_registered("manufacturer1", "serial1"));
  ASSERT_TRUE(master->is_agv_registered("manufacturer2", "serial2"));
  ASSERT_FALSE(master->is_agv_registered("unknown", "unknown"));

  // Unregister one AGV
  master->unregister_agv("manufacturer1", "serial1");
  ASSERT_FALSE(master->is_agv_registered("manufacturer1", "serial1"));
  ASSERT_TRUE(master->is_agv_registered("manufacturer2", "serial2"));

  master->disconnect();
}
