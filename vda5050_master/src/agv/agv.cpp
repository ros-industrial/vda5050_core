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

#include "vda5050_master/agv/agv.hpp"

#include <utility>

#include "nlohmann/json.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_master/standard_names.hpp"

// ============================================================================
// Constructor
// ============================================================================

AGV::AGV(
  const std::string& manufacturer, const std::string& serial_number,
  std::unique_ptr<ICommunicationStrategy> communication)
: manufacturer_(manufacturer),
  serial_number_(serial_number),
  agv_id_(manufacturer + "/" + serial_number),
  communication_(std::move(communication)),
  registered_time_(Clock::now())
{
}

// ============================================================================
// Communication
// ============================================================================

void AGV::connect()
{
  if (communication_)
  {
    communication_->connect();
  }
}

void AGV::disconnect()
{
  if (communication_)
  {
    communication_->disconnect();
  }
}

bool AGV::is_connected() const
{
  if (communication_)
  {
    return communication_->get_state() == ConnectionState::CONNECTED;
  }
  return false;
}

// ============================================================================
// Cached Messages - Update
// ============================================================================

void AGV::update_connection(const vda5050_msgs::msg::Connection& msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  last_connection_ = msg;
  last_connection_time_ = Clock::now();
}

void AGV::update_state(const vda5050_msgs::msg::State& msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  last_state_ = msg;
  last_state_time_ = Clock::now();
}

void AGV::update_factsheet(const vda5050_msgs::msg::Factsheet& msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  last_factsheet_ = msg;
  last_factsheet_time_ = Clock::now();
}

void AGV::update_visualization(const vda5050_msgs::msg::Visualization& msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  last_visualization_ = msg;
  last_visualization_time_ = Clock::now();
}

// ============================================================================
// Cached Messages - Get
// ============================================================================

std::optional<vda5050_msgs::msg::Connection> AGV::get_last_connection() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_connection_;
}

std::optional<vda5050_msgs::msg::State> AGV::get_last_state() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_state_;
}

std::optional<vda5050_msgs::msg::Factsheet> AGV::get_last_factsheet() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_factsheet_;
}

std::optional<vda5050_msgs::msg::Visualization> AGV::get_last_visualization()
  const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_visualization_;
}

// ============================================================================
// Timestamps
// ============================================================================

std::optional<AGV::TimePoint> AGV::get_last_connection_time() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_connection_time_;
}

std::optional<AGV::TimePoint> AGV::get_last_state_time() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_state_time_;
}

std::optional<AGV::TimePoint> AGV::get_last_factsheet_time() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_factsheet_time_;
}

std::optional<AGV::TimePoint> AGV::get_last_visualization_time() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_visualization_time_;
}

// ============================================================================
// Subscriptions and Publishing
// ============================================================================

void AGV::setup_subscriptions(
  ConnectionCallback on_connection, StateCallback on_state,
  FactsheetCallback on_factsheet, VisualizationCallback on_visualization)
{
  if (!communication_)
  {
    VDA5050_WARN(
      "[AGV] Cannot setup subscriptions: no communication for {}", agv_id_);
    return;
  }

  // Subscribe to connection topic
  communication_->subscribe(
    build_topic(vda5050_master::ConnectionTopic),
    [this, on_connection](
      const std::string& /*topic*/, const std::string& payload) {
      handle_connection_message(payload, on_connection);
    },
    vda5050_master::ConnectionQos);

  // Subscribe to state topic
  communication_->subscribe(
    build_topic(vda5050_master::StateTopic),
    [this, on_state](const std::string& /*topic*/, const std::string& payload) {
      handle_state_message(payload, on_state);
    },
    vda5050_master::StateQos);

  // Subscribe to factsheet topic (callback is optional, but subscription always created)
  communication_->subscribe(
    build_topic(vda5050_master::FactsheetTopic),
    [this, on_factsheet](
      const std::string& /*topic*/, const std::string& payload) {
      handle_factsheet_message(payload, on_factsheet);
    },
    vda5050_master::FactsheetQos);

  // Subscribe to visualization topic (callback is optional, but subscription always created)
  communication_->subscribe(
    build_topic(vda5050_master::VisualizationTopic),
    [this, on_visualization](
      const std::string& /*topic*/, const std::string& payload) {
      handle_visualization_message(payload, on_visualization);
    },
    vda5050_master::VisualizationQos);

  VDA5050_INFO("[AGV] Setup subscriptions for AGV: {}", agv_id_);
}

void AGV::send_order(const vda5050_msgs::msg::Order& order)
{
  if (!communication_)
  {
    VDA5050_WARN("[AGV] Cannot send order: no communication for {}", agv_id_);
    return;
  }

  nlohmann::json j;
  vda5050_msgs::msg::to_json(j, order);
  communication_->send_message(
    build_topic(vda5050_master::OrderTopic), j.dump(),
    vda5050_master::OrderQos);

  VDA5050_INFO("[AGV] Sent order to AGV: {}", agv_id_);
}

void AGV::send_instant_actions(const vda5050_msgs::msg::InstantActions& actions)
{
  if (!communication_)
  {
    VDA5050_WARN(
      "[AGV] Cannot send instant actions: no communication for {}", agv_id_);
    return;
  }

  nlohmann::json j;
  vda5050_msgs::msg::to_json(j, actions);
  communication_->send_message(
    build_topic(vda5050_master::InstantActionsTopic), j.dump(),
    vda5050_master::InstantActionsQos);

  VDA5050_INFO("[AGV] Sent instant actions to AGV: {}", agv_id_);
}

// ============================================================================
// Private Helper Methods
// ============================================================================

std::string AGV::build_topic(const std::string& topic_name) const
{
  return vda5050_master::InterfaceName + "/" + vda5050_master::Version + "/" +
         manufacturer_ + "/" + serial_number_ + "/" + topic_name;
}

void AGV::handle_connection_message(
  const std::string& payload, const ConnectionCallback& callback)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::Connection msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    // Update cache
    update_connection(msg);

    // Invoke user callback
    if (callback)
    {
      callback(agv_id_, msg);
    }
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[AGV] Failed to parse connection message from {}: {}", agv_id_,
      e.what());
  }
}

void AGV::handle_state_message(
  const std::string& payload, const StateCallback& callback)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::State msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    // Update cache
    update_state(msg);

    // Invoke user callback
    if (callback)
    {
      callback(agv_id_, msg);
    }
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[AGV] Failed to parse state message from {}: {}", agv_id_, e.what());
  }
}

void AGV::handle_factsheet_message(
  const std::string& payload, const FactsheetCallback& callback)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::Factsheet msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    // Update cache
    update_factsheet(msg);

    // Invoke user callback
    if (callback)
    {
      callback(agv_id_, msg);
    }
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[AGV] Failed to parse factsheet message from {}: {}", agv_id_, e.what());
  }
}

void AGV::handle_visualization_message(
  const std::string& payload, const VisualizationCallback& callback)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::Visualization msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    // Update cache
    update_visualization(msg);

    // Invoke user callback
    if (callback)
    {
      callback(agv_id_, msg);
    }
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[AGV] Failed to parse visualization message from {}: {}", agv_id_,
      e.what());
  }
}
