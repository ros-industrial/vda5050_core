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

#ifndef VDA5050_MASTER__COMMUNICATION__VDA5050_HPP_
#define VDA5050_MASTER__COMMUNICATION__VDA5050_HPP_

#include <memory>
#include <string>
#include <utility>

#include "nlohmann/json.hpp"
#include "vda5050_master/communication/communication.hpp"
#include "vda5050_master/communication/vda5050_handlers.hpp"
#include "vda5050_master/communication/vda5050_topic_config.hpp"
#include "vda5050_master/standard_names.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

/**
 * @brief VDA5050 communication manager using callback injection
 *
 * This class handles VDA5050 protocol communication in a protocol-agnostic way.
 * It accepts an ICommunicationStrategy (MQTT, ROS2, etc.) and invokes user-provided
 * callbacks when messages are received.
 *
 * Thread safety: Callbacks are invoked on the communication thread. Users are
 * responsible for thread synchronization in their callback implementations.
 */
class Vda5050Communication
{
public:
  /**
   * @brief Construct a VDA5050 communication manager
   * @param comm_client The communication strategy (MQTT, ROS2, etc.)
   * @param config Topic configuration
   * @param handlers Message callbacks
   * @throws std::runtime_error if config or handlers validation fails
   */
  Vda5050Communication(
    std::unique_ptr<ICommunicationStrategy> comm_client,
    VDA5050TopicConfig config, VDA5050Handlers handlers);

  /**
   * @brief Start communication and subscribe to topics
   */
  void start_communication();

  /**
   * @brief Stop communication and disconnect
   */
  void stop_communication();

  /**
   * @brief Publish an order to the AGV
   * @param order The order message
   * @throws std::runtime_error if order topic not configured
   */
  void publish_order(const vda5050_msgs::msg::Order& order);

  /**
   * @brief Publish instant actions to the AGV
   * @param actions The instant actions message
   * @throws std::runtime_error if instant actions topic not configured
   */
  void publish_instant_actions(
    const vda5050_msgs::msg::InstantActions& actions);

private:
  /**
   * @brief Subscribe to a topic with JSON deserialization and typed callback
   * @tparam MsgType The VDA5050 message type
   * @tparam Handler The callback type
   * @param topic The topic to subscribe to
   * @param handler The callback to invoke on message receipt
   * @param qos The QoS level for the subscription
   */
  template <typename MsgType, typename Handler>
  void subscribe_to_topic(
    const std::string& topic, const Handler& handler, int qos)
  {
    comm_client_->subscribe(
      topic,
      [handler](const std::string& /*topic*/, const std::string& payload) {
        if (!handler)
        {
          return;
        }

        try
        {
          auto json_msg = nlohmann::json::parse(payload);
          MsgType msg;
          vda5050_msgs::msg::from_json(json_msg, msg);
          handler(msg);
        }
        catch (const std::exception& e)
        {
          // TODO(tanjpg): Add proper error handling/logging
          // Consider adding an error callback to VDA5050Handlers
        }
      },
      qos);
  }

  std::unique_ptr<ICommunicationStrategy> comm_client_;
  VDA5050TopicConfig config_;
  VDA5050Handlers handlers_;
};

#endif  // VDA5050_MASTER__COMMUNICATION__VDA5050_HPP_
