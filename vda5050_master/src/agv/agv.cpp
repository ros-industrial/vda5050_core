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
#include "vda5050_json_utils/serialization.hpp"
#include "vda5050_master/standard_names.hpp"

// TODO(vda5050_json_utils): Add serializers for InstantActions, Visualization,
// and Factsheet to vda5050_json_utils/serialization.hpp

// ============================================================================
// Constructor / Destructor
// ============================================================================

AGV::AGV(
  const std::string& manufacturer, const std::string& serial_number,
  std::unique_ptr<ICommunicationStrategy> communication, size_t max_queue_size,
  bool drop_oldest, int state_heartbeat_interval)
: manufacturer_(manufacturer),
  serial_number_(serial_number),
  agv_id_(manufacturer + "/" + serial_number),
  communication_(std::move(communication)),
  registered_time_(Clock::now()),
  max_queue_size_(max_queue_size),
  drop_oldest_(drop_oldest),
  state_heartbeat_interval_(state_heartbeat_interval)
{
  // AGV components will be set up when ONLINE connection message is received
}

AGV::~AGV()
{
  // Reuse cleanup logic - safe to call even if already cleaned up
  cleanup_agv_components();
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
  // Stop state heartbeat listener
  if (state_heartbeat_)
  {
    state_heartbeat_->stop_connection_heartbeat();
  }

  if (communication_)
  {
    communication_->disconnect();
  }

  // Set explicit disconnect state to OFFLINE (intentional disconnection)
  set_connection_status(vda5050_types::ConnectionState::OFFLINE);
}

void AGV::setup_agv_components()
{
  VDA5050_INFO("[AGV] Setting up AGV components for {}", agv_id_);

  // Create and start state heartbeat listener
  state_heartbeat_ =
    std::make_unique<vda5050_master::communication::HeartbeatListener>(
      agv_id_ + "_state_heartbeat", state_heartbeat_interval_,
      [this]() { on_state_heartbeat_timeout(); });
  state_heartbeat_->start_connection_heartbeat();

  // Subscribe to remaining topics
  communication_->subscribe(
    build_topic(vda5050_master::StateTopic),
    [this](const std::string& /*topic*/, const std::string& payload) {
      handle_state_message(payload, stored_state_callback_);
    },
    vda5050_master::StateQos);

  communication_->subscribe(
    build_topic(vda5050_master::FactsheetTopic),
    [this](const std::string& /*topic*/, const std::string& payload) {
      handle_factsheet_message(payload, stored_factsheet_callback_);
    },
    vda5050_master::FactsheetQos);

  communication_->subscribe(
    build_topic(vda5050_master::VisualizationTopic),
    [this](const std::string& /*topic*/, const std::string& payload) {
      handle_visualization_message(payload, stored_visualization_callback_);
    },
    vda5050_master::VisualizationQos);

  // Start queue processing thread
  queue_thread_ = std::thread(&AGV::process_queues, this);

  VDA5050_INFO("[AGV] AGV components setup complete for {}", agv_id_);
}

void AGV::cleanup_agv_components()
{
  // Only clean up if components were previously set up
  if (!connection_established_)
  {
    return;
  }

  VDA5050_INFO("[AGV] Cleaning up AGV components for {}", agv_id_);

  // Stop heartbeat listener
  if (state_heartbeat_)
  {
    state_heartbeat_->stop_connection_heartbeat();
    state_heartbeat_.reset();
  }

  // Stop queue processing thread
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    stop_processing_ = true;
  }
  queue_cv_.notify_one();

  if (queue_thread_.joinable())
  {
    queue_thread_.join();
  }

  // Reset stop flag for potential re-initialization
  stop_processing_ = false;

  // Unsubscribe from topics (connection topic remains subscribed)
  if (communication_)
  {
    communication_->unsubscribe(build_topic(vda5050_master::StateTopic));
    communication_->unsubscribe(build_topic(vda5050_master::FactsheetTopic));
    communication_->unsubscribe(
      build_topic(vda5050_master::VisualizationTopic));
  }

  // Mark as not established so components can be re-initialized on next ONLINE
  connection_established_ = false;

  VDA5050_INFO("[AGV] AGV components cleanup complete for {}", agv_id_);
}

bool AGV::is_connected() const
{
  if (communication_)
  {
    return communication_->get_state() == ConnectionState::CONNECTED;
  }
  return false;
}

vda5050_types::ConnectionState AGV::get_connection_status() const
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  return connection_status_;
}

AGVState AGV::get_operational_state() const
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  return operational_state_;
}

void AGV::set_connection_status(vda5050_types::ConnectionState status)
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  connection_status_ = status;

  // When connection is lost, AGV becomes unavailable
  if (
    status == vda5050_types::ConnectionState::OFFLINE ||
    status == vda5050_types::ConnectionState::CONNECTIONBROKEN)
  {
    if (operational_state_ != AGVState::UNAVAILABLE)
    {
      operational_state_ = AGVState::UNAVAILABLE;
      VDA5050_INFO(
        "[AGV] Operational state changed to UNAVAILABLE for {} (connection {})",
        agv_id_,
        status == vda5050_types::ConnectionState::OFFLINE ? "OFFLINE"
                                                          : "CONNECTIONBROKEN");
    }
  }
}

void AGV::set_operational_state(AGVState state)
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  operational_state_ = state;

  const char* state_str = "UNKNOWN";
  switch (state)
  {
    case AGVState::STATE_UNKNOWN:
      state_str = "STATE_UNKNOWN";
      break;
    case AGVState::AVAILABLE:
      state_str = "AVAILABLE";
      break;
    case AGVState::UNAVAILABLE:
      state_str = "UNAVAILABLE";
      break;
    case AGVState::ERROR:
      state_str = "ERROR";
      break;
  }
  VDA5050_INFO(
    "[AGV] Operational state changed to {} for {}", state_str, agv_id_);
}

void AGV::on_state_heartbeat_timeout()
{
  set_operational_state(AGVState::STATE_UNKNOWN);
  VDA5050_WARN("[AGV] State heartbeat timeout for {}", agv_id_);
}

// ============================================================================
// Cached Messages - Update
// ============================================================================

void AGV::update_connection(const vda5050_types::Connection& msg)
{
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    last_connection_ = msg;
    last_connection_time_ = Clock::now();
  }

  // Update connection status based on the connectionState field in the message
  set_connection_status(msg.connection_state);

  // Clean up AGV components when connection is lost
  if (
    msg.connection_state == vda5050_types::ConnectionState::OFFLINE ||
    msg.connection_state == vda5050_types::ConnectionState::CONNECTIONBROKEN)
  {
    cleanup_agv_components();
  }
  // Set up AGV components when ONLINE message is received
  else if (
    msg.connection_state == vda5050_types::ConnectionState::ONLINE &&
    !connection_established_)
  {
    setup_agv_components();
    connection_established_ = true;
  }
}

void AGV::update_state(const vda5050_types::State& msg)
{
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    last_state_ = msg;
    last_state_time_ = Clock::now();
  }

  // Notify heartbeat listener and update state
  if (state_heartbeat_)
  {
    state_heartbeat_->received_connection();
  }
  set_operational_state(AGVState::AVAILABLE);
}

void AGV::update_factsheet(const vda5050_types::Factsheet& msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  last_factsheet_ = msg;
  last_factsheet_time_ = Clock::now();
}

void AGV::update_visualization(const vda5050_types::Visualization& msg)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  last_visualization_ = msg;
  last_visualization_time_ = Clock::now();
}

// ============================================================================
// Cached Messages - Get
// ============================================================================

std::optional<vda5050_types::Connection> AGV::get_last_connection() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_connection_;
}

std::optional<vda5050_types::State> AGV::get_last_state() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_state_;
}

std::optional<vda5050_types::Factsheet> AGV::get_last_factsheet() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return last_factsheet_;
}

std::optional<vda5050_types::Visualization> AGV::get_last_visualization() const
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

  // Store callbacks for later use when connection is established
  stored_connection_callback_ = on_connection;
  stored_state_callback_ = on_state;
  stored_factsheet_callback_ = on_factsheet;
  stored_visualization_callback_ = on_visualization;

  // Initially only subscribe to connection topic to wait for ONLINE status
  communication_->subscribe(
    build_topic(vda5050_master::ConnectionTopic),
    [this](const std::string& /*topic*/, const std::string& payload) {
      handle_connection_message(payload, stored_connection_callback_);
    },
    vda5050_master::ConnectionQos);

  VDA5050_INFO(
    "[AGV] Setup connection subscription for AGV: {} (waiting for ONLINE)",
    agv_id_);
}

bool AGV::send_order(const vda5050_types::Order& order)
{
  std::lock_guard<std::mutex> lock(queue_mutex_);

  if (order_queue_.size() >= max_queue_size_)
  {
    if (!drop_oldest_)
    {
      VDA5050_WARN(
        "[AGV] Dropping new order: queue full ({}/{}) for {}",
        order_queue_.size(), max_queue_size_, agv_id_);
      return false;
    }
    // Drop oldest order to make room
    VDA5050_WARN(
      "[AGV] Dropping oldest order: queue full ({}/{}) for {}",
      order_queue_.size(), max_queue_size_, agv_id_);
    order_queue_.pop();
  }

  order_queue_.push(order);
  queue_cv_.notify_one();

  VDA5050_INFO("[AGV] Queued order for AGV: {}", agv_id_);
  return true;
}

bool AGV::send_instant_actions(const vda5050_types::InstantActions& actions)
{
  std::lock_guard<std::mutex> lock(queue_mutex_);

  if (instant_actions_queue_.size() >= max_queue_size_)
  {
    if (!drop_oldest_)
    {
      VDA5050_WARN(
        "[AGV] Dropping new instant actions: queue full ({}/{}) for {}",
        instant_actions_queue_.size(), max_queue_size_, agv_id_);
      return false;
    }
    // Drop oldest instant actions to make room
    VDA5050_WARN(
      "[AGV] Dropping oldest instant actions: queue full ({}/{}) for {}",
      instant_actions_queue_.size(), max_queue_size_, agv_id_);
    instant_actions_queue_.pop();
  }

  instant_actions_queue_.push(actions);
  queue_cv_.notify_one();

  VDA5050_INFO("[AGV] Queued instant actions for AGV: {}", agv_id_);
  return true;
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
    vda5050_types::Connection msg;
    vda5050_types::from_json(json_msg, msg);

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
    vda5050_types::State msg;
    vda5050_types::from_json(json_msg, msg);

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
    vda5050_types::Factsheet msg;
    vda5050_types::from_json(json_msg, msg);

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
    vda5050_types::Visualization msg;
    vda5050_types::from_json(json_msg, msg);

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

// ============================================================================
// Queue Processing
// ============================================================================

void AGV::process_queues()
{
  VDA5050_INFO("[AGV] Queue processing thread started for {}", agv_id_);

  while (true)
  {
    std::optional<vda5050_types::Order> order;
    std::optional<vda5050_types::InstantActions> actions;

    {
      std::unique_lock<std::mutex> lock(queue_mutex_);

      // Wait for a message or stop signal
      queue_cv_.wait(lock, [this] {
        return stop_processing_ || !order_queue_.empty() ||
               !instant_actions_queue_.empty();
      });

      // Check if we should stop
      if (
        stop_processing_ && order_queue_.empty() &&
        instant_actions_queue_.empty())
      {
        break;
      }

      // Process instant actions first (higher priority)
      if (!instant_actions_queue_.empty())
      {
        actions = std::move(instant_actions_queue_.front());
        instant_actions_queue_.pop();
      }
      else if (!order_queue_.empty())
      {
        order = std::move(order_queue_.front());
        order_queue_.pop();
      }
    }

    // Send the message (outside the lock)
    if (actions)
    {
      send_instant_actions_now(*actions);
    }
    else if (order)
    {
      send_order_now(*order);
    }
  }

  VDA5050_INFO("[AGV] Queue processing thread stopped for {}", agv_id_);
}

void AGV::send_order_now(const vda5050_types::Order& order)
{
  if (!communication_)
  {
    VDA5050_WARN("[AGV] Cannot send order: no communication for {}", agv_id_);
    return;
  }

  nlohmann::json j;
  vda5050_types::to_json(j, order);
  communication_->send_message(
    build_topic(vda5050_master::OrderTopic), j.dump(),
    vda5050_master::OrderQos);

  VDA5050_INFO("[AGV] Sent order to AGV: {}", agv_id_);
}

void AGV::send_instant_actions_now(const vda5050_types::InstantActions& actions)
{
  if (!communication_)
  {
    VDA5050_WARN(
      "[AGV] Cannot send instant actions: no communication for {}", agv_id_);
    return;
  }

  nlohmann::json j;
  vda5050_types::to_json(j, actions);
  communication_->send_message(
    build_topic(vda5050_master::InstantActionsTopic), j.dump(),
    vda5050_master::InstantActionsQos);

  VDA5050_INFO("[AGV] Sent instant actions to AGV: {}", agv_id_);
}
