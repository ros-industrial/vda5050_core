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

#ifndef VDA5050_MASTER__COMMUNICATION__VDA5050_MANAGER_HPP_
#define VDA5050_MASTER__COMMUNICATION__VDA5050_MANAGER_HPP_

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "nlohmann/json.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_master/communication/communication.hpp"
#include "vda5050_master/standard_names.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

/**
 * @brief AGV registration info
 * AGV ID is derived as "manufacturer/serial_number" per VDA5050 standard
 */
struct AGVInfo
{
  std::string manufacturer;
  std::string serial_number;

  /**
   * @brief Get unique AGV ID from manufacturer and serial number
   */
  std::string get_agv_id() const
  {
    return manufacturer + "/" + serial_number;
  }
};

/**
 * @brief Callback structure for VDA5050 messages (multi-AGV routing)
 * All callbacks receive AGV ID as first parameter for proper routing
 */
struct VDA5050Callbacks
{
  std::function<void(
    const std::string& agv_id, const vda5050_msgs::msg::Connection&)>
    on_connection;
  std::function<void(
    const std::string& agv_id, const vda5050_msgs::msg::State&)>
    on_state;
  std::function<void(
    const std::string& agv_id, const vda5050_msgs::msg::Factsheet&)>
    on_factsheet;
  std::function<void(
    const std::string& agv_id, const vda5050_msgs::msg::Visualization&)>
    on_visualization;
};

/**
 * @brief VDA5050 Communication Manager
 * Handles VDA5050-specific logic: AGV registration, topic structure, message routing
 * Uses a generic communication layer for actual transport
 */
class VDA5050CommunicationManager
{
public:
  VDA5050CommunicationManager(
    std::unique_ptr<ICommunicationStrategy> communication)
  : communication_(std::move(communication))
  {
  }

  /**
   * @brief Connect to the communication endpoint
   */
  void connect()
  {
    communication_->connect();
  }

  /**
   * @brief Disconnect from the communication endpoint
   */
  void disconnect()
  {
    communication_->disconnect();
  }

  /**
   * @brief Set VDA5050 callbacks (shared across all AGVs)
   * @param callbacks VDA5050 message callbacks with AGV ID routing
   *
   * All AGVs use the same callbacks. The callbacks receive agv_id
   * as a parameter to distinguish which AGV sent the message.
   * Must be called before register_agv().
   */
  void set_vda5050_callbacks(const VDA5050Callbacks& callbacks)
  {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    vda5050_callbacks_ = callbacks;
    VDA5050_INFO("[VDA5050Manager] VDA5050 callbacks configured");
  }

  /**
   * @brief Register an AGV for VDA5050 communication
   * @param agv_info AGV identification (manufacturer, serial_number)
   *
   * AGV ID is derived as "manufacturer/serial_number" per VDA5050 standard.
   * Topic structure: {interfaceName}/{majorVersion}/{manufacturer}/{serialNumber}/{topic}
   * Currently: rmf2/v2/{manufacturer}/{serialNumber}/{topic}
   */
  void register_agv(const AGVInfo& agv_info)
  {
    std::lock_guard<std::mutex> lock(agv_registry_mutex_);

    std::string agv_id = agv_info.get_agv_id();

    // Check if already registered
    if (agv_registry_.find(agv_id) != agv_registry_.end()) {
      VDA5050_WARN("[VDA5050Manager] AGV already registered: " + agv_id);
      return;
    }

    // Build VDA5050 topic prefix for this AGV
    // Pattern: {interfaceName}/{majorVersion}/{manufacturer}/{serialNumber}/
    std::string topic_base =
      vda5050_master::InterfaceName + "/" + vda5050_master::Version + "/" +
      agv_info.manufacturer + "/" + agv_info.serial_number + "/";

    // Subscribe to connection topic
    communication_->subscribe(
      topic_base + vda5050_master::ConnectionTopic,
      [this, agv_id](const std::string& topic, const std::string& payload) {
        this->handle_connection_message(agv_id, payload);
      },
      vda5050_master::ConnectionQos);

    // Subscribe to state topic
    communication_->subscribe(
      topic_base + vda5050_master::StateTopic,
      [this, agv_id](const std::string& topic, const std::string& payload) {
        this->handle_state_message(agv_id, payload);
      },
      vda5050_master::StateQos);

    // Subscribe to factsheet topic
    communication_->subscribe(
      topic_base + vda5050_master::FactsheetTopic,
      [this, agv_id](const std::string& topic, const std::string& payload) {
        this->handle_factsheet_message(agv_id, payload);
      },
      vda5050_master::FactsheetQos);

    // Subscribe to visualization topic
    communication_->subscribe(
      topic_base + vda5050_master::VisualizationTopic,
      [this, agv_id](const std::string& topic, const std::string& payload) {
        this->handle_visualization_message(agv_id, payload);
      },
      vda5050_master::VisualizationQos);

    // Store AGV info
    agv_registry_[agv_id] = agv_info;

    VDA5050_INFO("[VDA5050Manager] Registered AGV: " + agv_id);
  }

  /**
   * @brief Unregister an AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   *
   * Note: Subscriptions will remain active until communication disconnect
   */
  void unregister_agv(
    const std::string& manufacturer, const std::string& serial_number)
  {
    std::lock_guard<std::mutex> lock(agv_registry_mutex_);
    std::string agv_id = manufacturer + "/" + serial_number;
    agv_registry_.erase(agv_id);
    VDA5050_INFO("[VDA5050Manager] Unregistered AGV: " + agv_id);
  }

  /**
   * @brief Publish an order to a specific AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param order_json The order message as JSON
   */
  void publish_order(
    const std::string& manufacturer, const std::string& serial_number,
    const nlohmann::json& order_json)
  {
    std::string topic =
      build_topic(manufacturer, serial_number, vda5050_master::OrderTopic);
    communication_->send_message(
      topic, order_json.dump(), vda5050_master::OrderQos);
  }

  /**
   * @brief Publish instant actions to a specific AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param instant_actions_json The instant actions message as JSON
   */
  void publish_instant_actions(
    const std::string& manufacturer, const std::string& serial_number,
    const nlohmann::json& instant_actions_json)
  {
    std::string topic = build_topic(
      manufacturer, serial_number, vda5050_master::InstantActionsTopic);
    communication_->send_message(
      topic, instant_actions_json.dump(), vda5050_master::InstantActionsQos);
  }

private:
  /**
   * @brief Build VDA5050 topic path
   * Pattern: {interfaceName}/{majorVersion}/{manufacturer}/{serialNumber}/{topic}
   */
  std::string build_topic(
    const std::string& manufacturer, const std::string& serial_number,
    const std::string& topic_name)
  {
    return vda5050_master::InterfaceName + "/" + vda5050_master::Version + "/" +
           manufacturer + "/" + serial_number + "/" + topic_name;
  }

  void handle_connection_message(
    const std::string& agv_id, const std::string& payload)
  {
    try {
      auto json_msg = nlohmann::json::parse(payload);
      vda5050_msgs::msg::Connection msg;
      vda5050_msgs::msg::from_json(json_msg, msg);

      std::lock_guard<std::mutex> lock(callbacks_mutex_);
      if (vda5050_callbacks_.on_connection) {
        vda5050_callbacks_.on_connection(agv_id, msg);
      }
    }
    catch (const std::exception& e) {
      VDA5050_WARN(
        "[VDA5050Manager] Failed to parse connection message from " + agv_id +
        ": " + std::string(e.what()));
    }
  }

  void handle_state_message(
    const std::string& agv_id, const std::string& payload)
  {
    try {
      auto json_msg = nlohmann::json::parse(payload);
      vda5050_msgs::msg::State msg;
      vda5050_msgs::msg::from_json(json_msg, msg);

      std::lock_guard<std::mutex> lock(callbacks_mutex_);
      if (vda5050_callbacks_.on_state) {
        vda5050_callbacks_.on_state(agv_id, msg);
      }
    }
    catch (const std::exception& e) {
      VDA5050_WARN(
        "[VDA5050Manager] Failed to parse state message from " + agv_id + ": " +
        std::string(e.what()));
    }
  }

  void handle_factsheet_message(
    const std::string& agv_id, const std::string& payload)
  {
    try {
      auto json_msg = nlohmann::json::parse(payload);
      vda5050_msgs::msg::Factsheet msg;
      vda5050_msgs::msg::from_json(json_msg, msg);

      std::lock_guard<std::mutex> lock(callbacks_mutex_);
      if (vda5050_callbacks_.on_factsheet) {
        vda5050_callbacks_.on_factsheet(agv_id, msg);
      }
    }
    catch (const std::exception& e) {
      VDA5050_WARN(
        "[VDA5050Manager] Failed to parse factsheet message from " + agv_id +
        ": " + std::string(e.what()));
    }
  }

  void handle_visualization_message(
    const std::string& agv_id, const std::string& payload)
  {
    try {
      auto json_msg = nlohmann::json::parse(payload);
      vda5050_msgs::msg::Visualization msg;
      vda5050_msgs::msg::from_json(json_msg, msg);

      std::lock_guard<std::mutex> lock(callbacks_mutex_);
      if (vda5050_callbacks_.on_visualization) {
        vda5050_callbacks_.on_visualization(agv_id, msg);
      }
    }
    catch (const std::exception& e) {
      VDA5050_WARN(
        "[VDA5050Manager] Failed to parse visualization message from " +
        agv_id + ": " + std::string(e.what()));
    }
  }

  std::unique_ptr<ICommunicationStrategy> communication_;

  // Shared callbacks for all AGVs
  std::mutex callbacks_mutex_;
  VDA5050Callbacks vda5050_callbacks_;

  // Registry of AGV info (AGV ID -> AGVInfo)
  std::mutex agv_registry_mutex_;
  std::unordered_map<std::string, AGVInfo> agv_registry_;
};

#endif  // VDA5050_MASTER__COMMUNICATION__VDA5050_MANAGER_HPP_
