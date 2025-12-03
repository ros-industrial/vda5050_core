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

#ifndef VDA5050_MASTER__VDA5050_MASTER__MASTER_HPP_
#define VDA5050_MASTER__VDA5050_MASTER__MASTER_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "vda5050_master/communication/communication.hpp"
#include "vda5050_master/communication/vda5050_topic_config.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

/**
 * @brief AGV registration info
 * AGV ID is derived as "manufacturer/serial_number" per VDA5050 standard
 */
struct AGVInfo
{
  std::string manufacturer;
  std::string serial_number;

  std::string get_agv_id() const
  {
    return manufacturer + "/" + serial_number;
  }
};

/**
 * @brief VDA5050 Master for multi-AGV fleet management
 *
 * This abstract base class manages VDA5050 communication for multiple AGVs.
 * Users should inherit from this class and override the virtual callback
 * methods to handle incoming VDA5050 messages.
 *
 * Features:
 * - AGV registration and topic auto-construction per VDA5050 spec
 * - Message routing with AGV identification
 * - Order and instant action publishing to specific AGVs
 *
 * Topic structure: {interfaceName}/{majorVersion}/{manufacturer}/{serialNumber}/{topic}
 * Example: rmf2/v2/MyManufacturer/AGV001/state
 *
 * Thread safety note: Callbacks are invoked on the communication thread.
 * If thread safety is required, the callback implementation should handle
 * synchronization (e.g., using a mutex).
 */
class VDA5050Master
{
public:
  /**
   * @brief Construct a VDA5050 master
   * @param communication The communication strategy (MQTT, ROS2, etc.)
   */
  explicit VDA5050Master(std::unique_ptr<ICommunicationStrategy> communication);

  /**
   * @brief Virtual destructor for inheritance
   */
  virtual ~VDA5050Master() = default;

  /**
   * @brief Connect to the communication endpoint
   */
  void connect();

  /**
   * @brief Disconnect from the communication endpoint
   */
  void disconnect();

  /**
   * @brief Register an AGV for VDA5050 communication
   * @param agv_info AGV identification (manufacturer, serial_number)
   *
   * Subscribes to all VDA5050 topics for this AGV with auto-constructed
   * topic paths per VDA5050 specification.
   */
  void register_agv(const AGVInfo& agv_info);

  /**
   * @brief Unregister an AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   */
  void unregister_agv(
    const std::string& manufacturer, const std::string& serial_number);

  /**
   * @brief Check if an AGV is registered
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @return true if AGV is registered
   */
  bool is_agv_registered(
    const std::string& manufacturer, const std::string& serial_number) const;

  /**
   * @brief Publish an order to a specific AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param order The order message
   * @throws std::runtime_error if AGV is not registered
   */
  void publish_order(
    const std::string& manufacturer, const std::string& serial_number,
    const vda5050_msgs::msg::Order& order);

  /**
   * @brief Publish instant actions to a specific AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param actions The instant actions message
   * @throws std::runtime_error if AGV is not registered
   */
  void publish_instant_actions(
    const std::string& manufacturer, const std::string& serial_number,
    const vda5050_msgs::msg::InstantActions& actions);

protected:
  // ============================================================================
  // Virtual callback methods - Override these in derived classes
  // ============================================================================

  /**
   * @brief Called when a connection message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The connection message
   *
   * This is a pure virtual method - derived classes must implement it.
   */
  virtual void on_connection(
    const std::string& agv_id, const vda5050_msgs::msg::Connection& msg) = 0;

  /**
   * @brief Called when a state message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The state message
   *
   * This is a pure virtual method - derived classes must implement it.
   */
  virtual void on_state(
    const std::string& agv_id, const vda5050_msgs::msg::State& msg) = 0;

  /**
   * @brief Called when a factsheet message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The factsheet message
   *
   * Default implementation logs a warning. Override to handle factsheets.
   */
  virtual void on_factsheet(
    const std::string& agv_id, const vda5050_msgs::msg::Factsheet& msg);

  /**
   * @brief Called when a visualization message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The visualization message
   *
   * Default implementation logs a warning. Override to handle visualizations.
   */
  virtual void on_visualization(
    const std::string& agv_id, const vda5050_msgs::msg::Visualization& msg);

private:
  /**
   * @brief Build VDA5050 topic configuration for an AGV
   */
  VDA5050TopicConfig build_topic_config(const AGVInfo& agv_info) const;

  /**
   * @brief Build a single VDA5050 topic path
   */
  std::string build_topic(
    const std::string& manufacturer, const std::string& serial_number,
    const std::string& topic_name) const;

  // Internal message handlers that parse JSON and invoke virtual callbacks
  void handle_connection_message(
    const std::string& agv_id, const std::string& payload);
  void handle_state_message(
    const std::string& agv_id, const std::string& payload);
  void handle_factsheet_message(
    const std::string& agv_id, const std::string& payload);
  void handle_visualization_message(
    const std::string& agv_id, const std::string& payload);

  std::unique_ptr<ICommunicationStrategy> communication_;

  mutable std::mutex agv_registry_mutex_;
  std::unordered_map<std::string, AGVInfo> agv_registry_;
};

#endif  // VDA5050_MASTER__VDA5050_MASTER__MASTER_HPP_
