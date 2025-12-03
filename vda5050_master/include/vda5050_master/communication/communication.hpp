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

#ifndef VDA5050_MASTER__COMMUNICATION__COMMUNICATION_HPP_
#define VDA5050_MASTER__COMMUNICATION__COMMUNICATION_HPP_

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Protocol-agnostic callback signature
 * All protocols (MQTT, ROS2, etc.) must adapt their native callbacks to this format
 * @param topic The topic the message was received on
 * @param payload The message payload as a JSON string
 */
using MessageCallback =
  std::function<void(const std::string& topic, const std::string& payload)>;

/**
 * @brief Connection lifecycle states
 *
 * State transitions:
 *   DISCONNECTED -> CONNECTED (via connect())
 *   CONNECTED -> DISCONNECTING (via disconnect())
 *   DISCONNECTING -> DISCONNECTED (when cleanup completes)
 */
enum class ConnectionState
{
  DISCONNECTED,  // Not connected, safe to destroy or reconnect
  CONNECTED,     // Actively connected, can send/receive messages
  DISCONNECTING  // Disconnect in progress, cleanup ongoing
};

class ICommunicationStrategy
{
public:
  virtual ~ICommunicationStrategy() = default;

  /**
   * @brief Subscribe to a topic with a callback
   * @param topic Topic to subscribe to
   * @param callback Function to call when messages arrive
   * @param qos Quality of Service level
   *
   * The callback will be invoked immediately when messages arrive.
   * Protocol implementations must adapt their native callbacks to match
   * the MessageCallback signature.
   */
  virtual void subscribe(
    const std::string& topic, MessageCallback callback, const int qos = 0) = 0;

  /**
   * @brief Unsubscribe from a topic
   * @param topic Topic to unsubscribe from
   *
   * Removes the subscription and associated callback for the given topic.
   */
  virtual void unsubscribe(const std::string& topic) = 0;

  virtual void connect() = 0;
  virtual void disconnect() = 0;
  virtual void send_message(
    const std::string& topic, const std::string& message,
    const int qos = 0) = 0;

  /**
   * @brief Get the current connection state
   * @return ConnectionState enum value
   */
  virtual ConnectionState get_state() const = 0;
};
#endif  // VDA5050_MASTER__COMMUNICATION__COMMUNICATION_HPP_
