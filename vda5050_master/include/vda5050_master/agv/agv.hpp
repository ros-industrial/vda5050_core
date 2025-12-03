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

#ifndef VDA5050_MASTER__AGV__AGV_HPP_
#define VDA5050_MASTER__AGV__AGV_HPP_

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "vda5050_master/communication/communication.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

// Forward declaration
class VDA5050Master;

/**
 * @brief Represents an individual AGV managed by VDA5050Master
 *
 * This class encapsulates all per-AGV data including:
 * - Identity information (manufacturer, serial number)
 * - Communication instance
 * - Cached VDA5050 messages (connection, state, factsheet, visualization)
 * - Connection status and timestamps
 *
 * Thread safety: Methods are thread-safe. Cached data access is protected
 * by a mutex.
 */
class AGV
{
public:
  // Type aliases
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;

  // Callback types for VDA5050 message handling
  using ConnectionCallback = std::function<void(
    const std::string&, const vda5050_msgs::msg::Connection&)>;
  using StateCallback =
    std::function<void(const std::string&, const vda5050_msgs::msg::State&)>;
  using FactsheetCallback = std::function<void(
    const std::string&, const vda5050_msgs::msg::Factsheet&)>;
  using VisualizationCallback = std::function<void(
    const std::string&, const vda5050_msgs::msg::Visualization&)>;

  /**
   * @brief Construct an AGV instance
   * @param manufacturer Manufacturer name
   * @param serial_number Serial number
   * @param communication Communication strategy for this AGV
   */
  AGV(
    const std::string& manufacturer, const std::string& serial_number,
    std::unique_ptr<ICommunicationStrategy> communication);

  ~AGV() = default;

  // Non-copyable, movable
  AGV(const AGV&) = delete;
  AGV& operator=(const AGV&) = delete;
  AGV(AGV&&) = default;
  AGV& operator=(AGV&&) = default;

  // ============================================================================
  // Identity
  // ============================================================================

  /**
   * @brief Get the manufacturer name
   */
  const std::string& get_manufacturer() const
  {
    return manufacturer_;
  }

  /**
   * @brief Get the serial number
   */
  const std::string& get_serial_number() const
  {
    return serial_number_;
  }

  /**
   * @brief Get the AGV ID (manufacturer/serial_number)
   */
  const std::string& get_agv_id() const
  {
    return agv_id_;
  }

  // ============================================================================
  // Communication
  // ============================================================================

  /**
   * @brief Connect the AGV's communication
   */
  void connect();

  /**
   * @brief Disconnect the AGV's communication
   */
  void disconnect();

  /**
   * @brief Check if the AGV is connected
   */
  bool is_connected() const;

  /**
   * @brief Setup VDA5050 topic subscriptions for this AGV
   * @param on_connection Callback for connection messages
   * @param on_state Callback for state messages
   * @param on_factsheet Callback for factsheet messages (optional, cache always updated)
   * @param on_visualization Callback for visualization messages (optional, cache always updated)
   *
   * Sets up subscriptions to all standard VDA5050 topics using the AGV's
   * manufacturer and serial number. Received messages are parsed from JSON
   * and cached in the AGV. User callbacks are invoked if provided.
   */
  void setup_subscriptions(
    ConnectionCallback on_connection, StateCallback on_state,
    FactsheetCallback on_factsheet = nullptr,
    VisualizationCallback on_visualization = nullptr);

  // ============================================================================
  // Cached Messages (read-only access)
  // ============================================================================

  /**
   * @brief Get the last received connection message
   * @return Optional containing the message if received, nullopt otherwise
   */
  std::optional<vda5050_msgs::msg::Connection> get_last_connection() const;

  /**
   * @brief Get the last received state message
   * @return Optional containing the message if received, nullopt otherwise
   */
  std::optional<vda5050_msgs::msg::State> get_last_state() const;

  /**
   * @brief Get the last received factsheet message
   * @return Optional containing the message if received, nullopt otherwise
   */
  std::optional<vda5050_msgs::msg::Factsheet> get_last_factsheet() const;

  /**
   * @brief Get the last received visualization message
   * @return Optional containing the message if received, nullopt otherwise
   */
  std::optional<vda5050_msgs::msg::Visualization> get_last_visualization()
    const;

  // ============================================================================
  // Timestamps
  // ============================================================================

  /**
   * @brief Get the time when the AGV was registered
   */
  TimePoint get_registered_time() const
  {
    return registered_time_;
  }

  /**
   * @brief Get the time of the last received connection message
   * @return Optional containing the timestamp if received, nullopt otherwise
   */
  std::optional<TimePoint> get_last_connection_time() const;

  /**
   * @brief Get the time of the last received state message
   * @return Optional containing the timestamp if received, nullopt otherwise
   */
  std::optional<TimePoint> get_last_state_time() const;

  /**
   * @brief Get the time of the last received factsheet message
   * @return Optional containing the timestamp if received, nullopt otherwise
   */
  std::optional<TimePoint> get_last_factsheet_time() const;

  /**
   * @brief Get the time of the last received visualization message
   * @return Optional containing the timestamp if received, nullopt otherwise
   */
  std::optional<TimePoint> get_last_visualization_time() const;

private:
  // VDA5050Master needs access to send/update methods
  friend class VDA5050Master;

  // ============================================================================
  // Master-only methods (accessed via friend class)
  // ============================================================================

  /**
   * @brief Send an order to this AGV
   * @param order The order message
   */
  void send_order(const vda5050_msgs::msg::Order& order);

  /**
   * @brief Send instant actions to this AGV
   * @param actions The instant actions message
   */
  void send_instant_actions(const vda5050_msgs::msg::InstantActions& actions);

  /**
   * @brief Update the cached connection message
   * @param msg The connection message
   */
  void update_connection(const vda5050_msgs::msg::Connection& msg);

  /**
   * @brief Update the cached state message
   * @param msg The state message
   */
  void update_state(const vda5050_msgs::msg::State& msg);

  /**
   * @brief Update the cached factsheet message
   * @param msg The factsheet message
   */
  void update_factsheet(const vda5050_msgs::msg::Factsheet& msg);

  /**
   * @brief Update the cached visualization message
   * @param msg The visualization message
   */
  void update_visualization(const vda5050_msgs::msg::Visualization& msg);

  // ============================================================================
  // Helper methods
  // ============================================================================

  // Helper methods for building VDA5050 topic paths
  std::string build_topic(const std::string& topic_name) const;

  // Internal message handlers that parse JSON, update cache, and invoke callbacks
  void handle_connection_message(
    const std::string& payload, const ConnectionCallback& callback);
  void handle_state_message(
    const std::string& payload, const StateCallback& callback);
  void handle_factsheet_message(
    const std::string& payload, const FactsheetCallback& callback);
  void handle_visualization_message(
    const std::string& payload, const VisualizationCallback& callback);

  // Identity
  std::string manufacturer_;
  std::string serial_number_;
  std::string agv_id_;

  // Communication
  std::unique_ptr<ICommunicationStrategy> communication_;

  // Timestamps
  TimePoint registered_time_;

  // Cached messages and timestamps (protected by mutex)
  mutable std::mutex data_mutex_;

  std::optional<vda5050_msgs::msg::Connection> last_connection_;
  std::optional<TimePoint> last_connection_time_;

  std::optional<vda5050_msgs::msg::State> last_state_;
  std::optional<TimePoint> last_state_time_;

  std::optional<vda5050_msgs::msg::Factsheet> last_factsheet_;
  std::optional<TimePoint> last_factsheet_time_;

  std::optional<vda5050_msgs::msg::Visualization> last_visualization_;
  std::optional<TimePoint> last_visualization_time_;
};

#endif  // VDA5050_MASTER__AGV__AGV_HPP_
