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

#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"
#include "vda5050_master/agv/agv.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

/**
 * @brief Factory function type for creating MQTT clients
 *
 * The factory takes an AGV identifier (used for client naming) and returns
 * a new MqttClientInterface instance.
 */
using MqttClientFactory =
  std::function<std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface>(
    const std::string& agv_id)>;

/**
 * @brief VDA5050 Master for multi-AGV fleet management
 *
 * This abstract base class manages VDA5050 communication for multiple AGVs.
 * Each registered AGV gets its own communication instance. Users should
 * inherit from this class and override the virtual callback methods to
 * handle incoming VDA5050 messages.
 *
 * Features:
 * - AGV registration with automatic communication setup
 * - Per-AGV communication isolation
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
   * @brief Construct a VDA5050 master with an MQTT client factory
   * @param factory Factory function that creates MqttClientInterface instances
   *
   * The factory is called for each AGV when register_agv() is invoked.
   *
   * Example usage:
   * @code
   * auto factory = [broker](const std::string& agv_id) {
   *   return vda5050_core::mqtt_client::create_default_client(broker, agv_id);
   * };
   * MyMaster master(factory);
   * @endcode
   */
  explicit VDA5050Master(MqttClientFactory factory);

  /**
   * @brief Virtual destructor for inheritance
   */
  virtual ~VDA5050Master() = default;

  /**
   * @brief Register an AGV for VDA5050 communication
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param max_queue_size Maximum number of outgoing messages to queue (default: 10)
   * @param drop_oldest If true, drop oldest message when queue full; if false, reject new message (default: true)
   *
   * Creates a dedicated communication instance for this AGV and subscribes
   * to all VDA5050 topics with auto-constructed topic paths per VDA5050 spec.
   */
  void register_agv(
    const std::string& manufacturer, const std::string& serial_number,
    size_t max_queue_size = AGV::DEFAULT_MAX_QUEUE_SIZE,
    bool drop_oldest = true);

  /**
   * @brief Unregister an AGV
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   *
   * Disconnects and removes the AGV's communication instance.
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
   * @return true if queued successfully, false if queue is full
   * @throws std::runtime_error if AGV is not registered
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
   * @throws std::runtime_error if AGV is not registered
   */
  bool publish_instant_actions(
    const std::string& manufacturer, const std::string& serial_number,
    const vda5050_types::InstantActions& actions);

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
    const std::string& agv_id, const vda5050_types::Connection& msg) = 0;

  /**
   * @brief Called when a state message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The state message
   *
   * This is a pure virtual method - derived classes must implement it.
   */
  virtual void on_state(
    const std::string& agv_id, const vda5050_types::State& msg) = 0;

  /**
   * @brief Called when a factsheet message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The factsheet message
   *
   * Default implementation logs a warning. Override to handle factsheets.
   */
  virtual void on_factsheet(
    const std::string& agv_id, const vda5050_types::Factsheet& msg);

  /**
   * @brief Called when a visualization message is received from an AGV
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @param msg The visualization message
   *
   * Default implementation logs a warning. Override to handle visualizations.
   */
  virtual void on_visualization(
    const std::string& agv_id, const vda5050_types::Visualization& msg);

private:
  /**
   * @brief Internal AGV lookup (must be called with agv_mutex_ held)
   * @param agv_id The AGV identifier (manufacturer/serial_number)
   * @return Shared pointer to AGV, or nullptr if not found
   */
  std::shared_ptr<AGV> get_agv(const std::string& agv_id) const;

  // Factory for creating MQTT clients
  MqttClientFactory mqtt_client_factory_;

  // Registered AGVs (shared_ptr allows safe access from get_agv())
  mutable std::mutex agv_mutex_;
  std::unordered_map<std::string, std::shared_ptr<AGV>> agvs_;
};

#endif  // VDA5050_MASTER__VDA5050_MASTER__MASTER_HPP_
