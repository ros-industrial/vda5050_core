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

/**
 * @file master_mqtt_test.cpp
 * @brief MQTT connection tests for VDA5050Master
 *
 * These tests verify MQTT connection and subscription setup using
 * a real MQTT broker. Tests are skipped if no broker is available.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <thread>

#include "../communication/mqtt/test_helpers.hpp"
#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"
#include "vda5050_master/vda5050_master/master.hpp"

using vda5050_master::test::mqtt::constants::is_broker_available;
using vda5050_master::test::mqtt::constants::MQTT_BROKER;

namespace vda5050_master::test {

// =============================================================================
// Test Fixture for MQTT Tests
// =============================================================================

class MasterMqttTestFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!is_broker_available())
    {
      GTEST_SKIP() << "MQTT broker at " << MQTT_BROKER
                   << " is not available. Skipping test.";
    }

    manufacturer_ = "TestManufacturer";
    serial_number_ = "SN001";
  }

  void TearDown() override
  {
    // Allow time for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::unique_ptr<VDA5050Master> create_master()
  {
    auto client = vda5050_core::mqtt_client::create_default_client(
      MQTT_BROKER, "master_mqtt_test_" + std::to_string(test_id_++));
    return std::make_unique<VDA5050Master>(client, MQTT_BROKER);
  }

  std::string manufacturer_;
  std::string serial_number_;
  static int test_id_;
};

int MasterMqttTestFixture::test_id_ = 0;

// =============================================================================
// Connection Management Tests
// =============================================================================

TEST_F(MasterMqttTestFixture, ConnectToMqttBroker)
{
  auto master = create_master();

  EXPECT_FALSE(master->is_connected());

  master->connect();

  EXPECT_TRUE(master->is_connected());

  master->disconnect();

  EXPECT_FALSE(master->is_connected());
}

TEST_F(MasterMqttTestFixture, ReconnectAfterDisconnect)
{
  auto master = create_master();

  master->connect();
  EXPECT_TRUE(master->is_connected());

  master->disconnect();
  EXPECT_FALSE(master->is_connected());

  master->connect();
  EXPECT_TRUE(master->is_connected());

  master->disconnect();
}

TEST_F(MasterMqttTestFixture, OnboardAGVWhileConnected)
{
  auto master = create_master();
  master->connect();

  EXPECT_FALSE(master->is_agv_onboarded(manufacturer_, serial_number_));

  master->onboard_agv(manufacturer_, serial_number_);

  EXPECT_TRUE(master->is_agv_onboarded(manufacturer_, serial_number_));

  auto agv = master->get_agv(manufacturer_, serial_number_);
  ASSERT_NE(agv, nullptr);
  EXPECT_EQ(agv->get_manufacturer(), manufacturer_);
  EXPECT_EQ(agv->get_serial_number(), serial_number_);

  master->disconnect();
}

TEST_F(MasterMqttTestFixture, OffboardAGVWhileConnected)
{
  auto master = create_master();
  master->connect();

  master->onboard_agv(manufacturer_, serial_number_);
  EXPECT_TRUE(master->is_agv_onboarded(manufacturer_, serial_number_));

  master->offboard_agv(manufacturer_, serial_number_);
  EXPECT_FALSE(master->is_agv_onboarded(manufacturer_, serial_number_));

  master->disconnect();
}

TEST_F(MasterMqttTestFixture, MultipleAGVsWhileConnected)
{
  auto master = create_master();
  master->connect();

  master->onboard_agv("Mfg1", "AGV1");
  master->onboard_agv("Mfg1", "AGV2");
  master->onboard_agv("Mfg2", "AGV1");

  EXPECT_TRUE(master->is_agv_onboarded("Mfg1", "AGV1"));
  EXPECT_TRUE(master->is_agv_onboarded("Mfg1", "AGV2"));
  EXPECT_TRUE(master->is_agv_onboarded("Mfg2", "AGV1"));

  auto agv1 = master->get_agv("Mfg1", "AGV1");
  auto agv2 = master->get_agv("Mfg1", "AGV2");
  auto agv3 = master->get_agv("Mfg2", "AGV1");

  ASSERT_NE(agv1, nullptr);
  ASSERT_NE(agv2, nullptr);
  ASSERT_NE(agv3, nullptr);

  master->disconnect();
}

TEST_F(MasterMqttTestFixture, AGVsPersistedAcrossReconnect)
{
  auto master = create_master();
  master->connect();

  master->onboard_agv(manufacturer_, serial_number_);
  auto agv_before = master->get_agv(manufacturer_, serial_number_);
  ASSERT_NE(agv_before, nullptr);

  master->disconnect();
  EXPECT_TRUE(master->is_agv_onboarded(manufacturer_, serial_number_));

  master->connect();
  EXPECT_TRUE(master->is_agv_onboarded(manufacturer_, serial_number_));

  auto agv_after = master->get_agv(manufacturer_, serial_number_);
  ASSERT_NE(agv_after, nullptr);

  // Same AGV instance should be preserved
  EXPECT_EQ(agv_before.get(), agv_after.get());

  master->disconnect();
}

}  // namespace vda5050_master::test
