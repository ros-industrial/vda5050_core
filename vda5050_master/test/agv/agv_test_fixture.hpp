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

#ifndef AGV__AGV_TEST_FIXTURE_HPP_
#define AGV__AGV_TEST_FIXTURE_HPP_

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "vda5050_master/agv/agv.hpp"

namespace vda5050_master::test {

/**
 * @brief Common test fixture for AGV unit tests
 *
 * Provides helper methods for creating AGV instances and test messages.
 */
class AGVTestFixture : public ::testing::Test
{
protected:
  void SetUp() override
  {
    manufacturer_ = "TestManufacturer";
    serial_number_ = "SN001";
    agv_id_ = manufacturer_ + "/" + serial_number_;
  }

  void TearDown() override
  {
    agv_.reset();
  }

  std::unique_ptr<AGV>& create_agv()
  {
    // Use empty broker address for unit tests
    agv_ = std::make_unique<AGV>(manufacturer_, serial_number_, "");
    return agv_;
  }

  std::unique_ptr<AGV>& create_agv_with_heartbeat_interval(
    int state_heartbeat_interval)
  {
    // Use empty broker address for unit tests
    agv_ = std::make_unique<AGV>(
      manufacturer_, serial_number_, "", 10, true, state_heartbeat_interval);
    return agv_;
  }

  vda5050_types::Connection create_connection_msg(const std::string& state)
  {
    vda5050_types::Connection msg;
    msg.header.header_id = 1;
    msg.header.timestamp = std::chrono::system_clock::now();
    msg.header.version = "2.0.0";
    msg.header.manufacturer = manufacturer_;
    msg.header.serial_number = serial_number_;

    if (state == "ONLINE")
    {
      msg.connection_state = vda5050_types::ConnectionState::ONLINE;
    }
    else if (state == "OFFLINE")
    {
      msg.connection_state = vda5050_types::ConnectionState::OFFLINE;
    }
    else
    {
      msg.connection_state = vda5050_types::ConnectionState::CONNECTIONBROKEN;
    }

    return msg;
  }

  vda5050_types::State create_state_msg()
  {
    vda5050_types::State msg;
    msg.header.header_id = 1;
    msg.header.timestamp = std::chrono::system_clock::now();
    msg.header.version = "2.0.0";
    msg.header.manufacturer = manufacturer_;
    msg.header.serial_number = serial_number_;
    msg.order_id = "test_order";
    msg.order_update_id = 0;
    msg.driving = false;
    msg.paused = false;
    msg.new_base_request = false;
    msg.distance_since_last_node = 0.0;
    return msg;
  }

  vda5050_types::Order create_test_order(const std::string& order_id)
  {
    vda5050_types::Order order;
    order.order_id = order_id;
    order.order_update_id = 0;
    return order;
  }

  vda5050_types::InstantActions create_test_instant_actions(uint32_t id)
  {
    vda5050_types::InstantActions actions;
    actions.header.header_id = id;
    return actions;
  }

  std::unique_ptr<AGV> agv_;
  std::string manufacturer_;
  std::string serial_number_;
  std::string agv_id_;
};

}  // namespace vda5050_master::test

#endif  // AGV__AGV_TEST_FIXTURE_HPP_
