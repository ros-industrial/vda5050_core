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

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"
#include "vda5050_master/agv/agv.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

namespace vda5050_master {

/**
 * @brief VDA5050 Master for multi-AGV fleet management
 *
 * This abstract base class manages VDA5050 communication for multiple AGVs
 * using a single shared MQTT client with wildcard subscriptions.
 *
 * Features:
 * - Single shared MQTT client with wildcard subscriptions
 * - AGV onboarding/offboarding with allow list
 * - Message routing based on topic parsing
 * - Transient MQTT clients for publishing to AGVs
 *
 * Topic structure: {interfaceName}/{majorVersion}/{manufacturer}/{serialNumber}/{topic}
 * Example: rmf2/v2/MyManufacturer/AGV001/state
 *
 * Wildcard subscriptions: rmf2/v2/+/+/connection, rmf2/v2/+/+/state, etc.
 *
 * Thread safety note: Callbacks are invoked on the MQTT client thread.
 * If thread safety is required, the callback implementation should handle
 * synchronization (e.g., using a mutex).
 */
class VDA5050Master
{
public:
  /**
   * @brief Construct a VDA5050 master with shared MQTT client and broker address
   * @param mqtt_client Shared MQTT client for subscriptions
   * @param broker_address MQTT broker address for creating transient publish clients
   *
   * The mqtt_client is used for all wildcard subscriptions.
   * The broker_address is used to create transient MQTT clients for publishing
   * using vda5050_core::mqtt_client::create_default_client().
   *
   * Example usage:
   * @code
   * auto client = vda5050_core::mqtt_client::create_default_client(broker, "master");
   * MyMaster master(client, broker);
   * master.connect();
   * @endcode
   */
  VDA5050Master(
    std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface> mqtt_client,
    const std::string& broker_address);

  /**
   * @brief Virtual destructor - disconnects MQTT client
   */
  virtual ~VDA5050Master();

  // Non-copyable, non-movable
  VDA5050Master(const VDA5050Master&) = delete;
  VDA5050Master& operator=(const VDA5050Master&) = delete;
  VDA5050Master(VDA5050Master&&) = delete;
  VDA5050Master& operator=(VDA5050Master&&) = delete;

  // ============================================================================
  // Connection Management
  // ============================================================================

  /**
   * @brief Connect the MQTT client and setup wildcard subscriptions
   */
  void connect();

  /**
   * @brief Disconnect the MQTT client
   */
  void disconnect();

  /**
   * @brief Check if MQTT client is connected
   */
  bool is_connected() const;

  // ============================================================================
  // AGV Onboarding/Offboarding
  // ============================================================================

  /**
   * @brief Onboard an AGV to allow message routing
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param max_queue_size Maximum number of outgoing messages to queue (default: 10)
   * @param drop_oldest If true, drop oldest message when queue full; if false, reject new message (default: true)
   *
   * Creates an AGV instance in the allowed list. Messages from this AGV
   * will be routed to the appropriate handlers.
   */
  void onboard_agv(
    const std::string& manufacturer, const std::string& serial_number,
    size_t max_queue_size = 10, bool drop_oldest = true);

  /**
   * @brief Offboard an AGV to stop message routing
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   *
   * Removes the AGV from the allowed list. Messages from this AGV
   * will be ignored with a warning.
   */
  void offboard_agv(
    const std::string& manufacturer, const std::string& serial_number);

  /**
   * @brief Check if an AGV is onboarded
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @return true if AGV is onboarded
   */
  bool is_agv_onboarded(
    const std::string& manufacturer, const std::string& serial_number) const;

  // ============================================================================
  // AGV Access
  // ============================================================================

  /**
   * @brief Get a shared pointer to an onboarded AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @return Shared pointer to AGV, or nullptr if not onboarded
   */
  std::shared_ptr<AGV> get_agv(
    const std::string& manufacturer, const std::string& serial_number) const;

  // ============================================================================
  // Outgoing Messages
  // ============================================================================

  /**
   * @brief Publish an order to a specific AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param order The order message
   * @return true if queued successfully, false if queue is full
   * @throws std::runtime_error if AGV is not onboarded
   */
  bool publish_order(
    const std::string& manufacturer, const std::string& serial_number,
    const vda5050_types::Order& order);

  /**
   * @brief Publish instant actions to a specific AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param actions The instant actions message
   * @return true if queued successfully, false if queue is full
   * @throws std::runtime_error if AGV is not onboarded
   */
  bool publish_instant_actions(
    const std::string& manufacturer, const std::string& serial_number,
    const vda5050_types::InstantActions& actions);

protected:
  // ============================================================================
  // Virtual callback methods - Override these in derived classes
  // ============================================================================

  /**
   * @brief Called when a connection message is received from an onboarded AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The connection message
   *
   * Default implementation logs a warning. Override to handle connections.
   */
  virtual void on_connection(
    const std::string& agv_id, const vda5050_types::Connection& msg);

  /**
   * @brief Called when a state message is received from an onboarded AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The state message
   *
   * Default implementation logs a warning. Override to handle states.
   */
  virtual void on_state(
    const std::string& agv_id, const vda5050_types::State& msg);

  /**
   * @brief Called when a factsheet message is received from an onboarded AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The factsheet message
   *
   * Default implementation logs a warning. Override to handle factsheets.
   */
  virtual void on_factsheet(
    const std::string& agv_id, const vda5050_types::Factsheet& msg);

  /**
   * @brief Called when a visualization message is received from an onboarded AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The visualization message
   *
   * Default implementation logs a warning. Override to handle visualizations.
   */
  virtual void on_visualization(
    const std::string& agv_id, const vda5050_types::Visualization& msg);

private:
  // ============================================================================
  // Subscription Management
  // ============================================================================

  void setup_subscriptions();
  void cleanup_subscriptions();

  // ============================================================================
  // Topic Utilities
  // ============================================================================

  /**
   * @brief Build a wildcard topic pattern for subscribing to all AGVs
   * @param topic_name The topic name (e.g., "connection", "state")
   * @return Wildcard topic pattern (e.g., "rmf2/v2/+/+/connection")
   */
  static std::string build_wildcard_topic(const std::string& topic_name);

  /**
   * @brief Parse manufacturer and serial number from a VDA5050 topic
   * @param topic The full topic string (e.g., "rmf2/v2/Manu/SN001/state")
   * @return Pair of (manufacturer, serial_number), or empty strings if parse fails
   *
   * Topic structure: {interfaceName}/{version}/{manufacturer}/{serialNumber}/{topic}
   */
  static std::pair<std::string, std::string> parse_topic(
    const std::string& topic);

  // ============================================================================
  // Message Handlers (from MQTT callbacks)
  // ============================================================================

  /**
   * @brief Generic message handler template
   * @tparam MsgType The VDA5050 message type
   * @param topic The MQTT topic
   * @param payload The JSON payload
   * @param message_type Type name for logging
   * @param agv_handler Pointer to AGV handler method
   * @param callback Pointer to callback method
   */
  template <typename MsgType>
  void handle_message(
    const std::string& topic, const std::string& payload,
    const std::string& message_type, void (AGV::*agv_handler)(const MsgType&),
    void (VDA5050Master::*callback)(const std::string&, const MsgType&));

  void handle_connection_message(
    const std::string& topic, const std::string& payload);
  void handle_state_message(
    const std::string& topic, const std::string& payload);
  void handle_factsheet_message(
    const std::string& topic, const std::string& payload);
  void handle_visualization_message(
    const std::string& topic, const std::string& payload);

  // ============================================================================
  // Internal AGV lookup
  // ============================================================================

  std::shared_ptr<AGV> get_agv_by_id(const std::string& agv_id) const;

  /**
   * @brief Get AGV from topic with validation and logging
   * @param topic The MQTT topic
   * @param message_type Type name for logging (e.g., "connection", "state")
   * @return Pair of (agv_id, AGV pointer), AGV is nullptr if not onboarded
   */
  std::pair<std::string, std::shared_ptr<AGV>> get_agv_from_topic(
    const std::string& topic, const std::string& message_type);

  // ============================================================================
  // Member Variables
  // ============================================================================

  // Shared MQTT client for subscriptions
  std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface> mqtt_client_;

  // Broker address for creating transient MQTT clients (passed to AGVs)
  std::string broker_address_;

  // Onboarded AGVs (shared_ptr allows safe access)
  mutable std::mutex agv_mutex_;
  std::unordered_map<std::string, std::shared_ptr<AGV>> agvs_;
};

}  // namespace vda5050_master

#endif  // VDA5050_MASTER__VDA5050_MASTER__MASTER_HPP_
