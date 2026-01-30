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

using vda5050_master::VDA5050Master;
using vda5050_master::test::mqtt::constants::is_broker_available;
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
  explicit TestVDA5050Master(CommunicationFactory factory)
  : VDA5050Master(std::move(factory))
  {
  }

  // Thread-safe storage for received messages
  std::mutex mutex_;
  std::vector<std::pair<std::string, vda5050_types::Connection>>
    received_connections_;
  std::vector<std::pair<std::string, vda5050_types::State>> received_states_;
  std::vector<std::pair<std::string, vda5050_types::Factsheet>>
    received_factsheets_;
  std::vector<std::pair<std::string, vda5050_types::Visualization>>
    received_visualizations_;
  std::atomic<int> connection_count_{0};
  std::atomic<int> state_count_{0};

protected:
  void on_connection(
    const std::string& agv_id, const vda5050_types::Connection& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_connections_.push_back({agv_id, msg});
    connection_count_++;
  }

  void on_state(
    const std::string& agv_id, const vda5050_types::State& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_states_.push_back({agv_id, msg});
    state_count_++;
  }

  void on_factsheet(
    const std::string& agv_id, const vda5050_types::Factsheet& msg) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    received_factsheets_.push_back({agv_id, msg});
  }

  void on_visualization(
    const std::string& agv_id, const vda5050_types::Visualization& msg) override
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
    // Skip tests if MQTT broker is not available
    if (!is_broker_available())
    {
      GTEST_SKIP() << "MQTT broker at " << MQTT_BROKER
                   << " is not available. Skipping test.";
    }

    broker_endpoint_ = MQTT_BROKER;
    // Create a factory that creates MqttCommunication instances
    mqtt_factory_ = [this](const std::string& agv_id) {
      return std::make_unique<MqttCommunication>(broker_endpoint_, agv_id);
    };
  }

  void TearDown() override
  {
    // Note: We don't shutdown rclcpp here to allow multiple tests to run
  }

  std::string broker_endpoint_;
  CommunicationFactory mqtt_factory_;
};

TEST_F(VDA5050MasterTest, Setup)
{
  auto master = std::make_unique<TestVDA5050Master>(mqtt_factory_);

  // Register an AGV (this creates and connects the communication)
  ASSERT_NO_THROW(master->register_agv("test_manufacturer", "test_serial"));
  ASSERT_TRUE(master->is_agv_registered("test_manufacturer", "test_serial"));

  // Unregister to clean up (disconnects the AGV's communication)
  ASSERT_NO_THROW(master->unregister_agv("test_manufacturer", "test_serial"));
}

TEST_F(VDA5050MasterTest, AGVRegistrationAndUnregistration)
{
  auto master = std::make_unique<TestVDA5050Master>(mqtt_factory_);

  // Register AGVs (each creates and connects its own communication)
  master->register_agv("manufacturer1", "serial1");
  master->register_agv("manufacturer2", "serial2");

  ASSERT_TRUE(master->is_agv_registered("manufacturer1", "serial1"));
  ASSERT_TRUE(master->is_agv_registered("manufacturer2", "serial2"));
  ASSERT_FALSE(master->is_agv_registered("unknown", "unknown"));

  // Unregister one AGV
  master->unregister_agv("manufacturer1", "serial1");
  ASSERT_FALSE(master->is_agv_registered("manufacturer1", "serial1"));
  ASSERT_TRUE(master->is_agv_registered("manufacturer2", "serial2"));

  // Cleanup
  master->unregister_agv("manufacturer2", "serial2");
}
