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

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "nlohmann/json.hpp"
#include "vda5050_master/communication/vda5050_config.hpp"
#include "vda5050_master/standard_names.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

class Vda5050Communication
{
public:
  Vda5050Communication(
    std::unique_ptr<ICommunicationStrategy> connection_client,
    std::unique_ptr<VDA5050Config> config)
  : communication_client_(std::move(connection_client)),
    config_(std::move(config))
  {
  }

  void start_communication()
  {
    communication_client_->connect();

    // Subscribe to connection topic with callback
    // Lambda captures 'this' to call the member function
    communication_client_->subscribe(
      config_->connection_topic,
      [this](const std::string& topic, const std::string& payload) {
        // Parse JSON and call the connection callback
        try {
          auto json_msg = nlohmann::json::parse(payload);
          this->connection_success_callback(json_msg);
        }
        catch (const std::exception& e) {
          // TODO(tanjpg): Add proper error handling/logging
          // For now, silently fail to maintain backwards compatibility
        }
      },
      vda5050_master::ConnectionQos);
  }

  void stop_communication()
  {
    communication_client_->disconnect();
  }

  void connection_success_callback(const nlohmann::json& connection_json)
  {
    // TODO(tanjpg): Store connection message if needed
    // vda5050_msgs::msg::Connection conn_msg;
    // vda5050_msgs::msg::from_json(connection_json, conn_msg);
    // connection_container_->push_back(conn_msg);

    // Subscribe to state topic with callback
    communication_client_->subscribe(
      config_->state_topic,
      [this](const std::string& topic, const std::string& payload) {
        try {
          auto json_msg = nlohmann::json::parse(payload);
          this->state_callback(json_msg);
        }
        catch (const std::exception& e) {
          // TODO(tanjpg): Error handling
        }
      },
      vda5050_master::StateQos);

    // Subscribe to factsheet topic with callback
    if (!config_->factsheet_topic.value().empty()) {
      communication_client_->subscribe(
        config_->factsheet_topic.value(),
        [this](const std::string& topic, const std::string& payload) {
          try {
            auto json_msg = nlohmann::json::parse(payload);
            this->factsheet_callback(json_msg);
          }
          catch (const std::exception& e) {
            // TODO(tanjpg): Error handling
          }
        },
        vda5050_master::FactsheetQos);
    }

    // Subscribe to visualization topic with callback
    if (!config_->visualization_topic.value().empty()) {
      communication_client_->subscribe(
        config_->visualization_topic.value(),
        [this](const std::string& topic, const std::string& payload) {
          try {
            auto json_msg = nlohmann::json::parse(payload);
            this->visualization_callback(json_msg);
          }
          catch (const std::exception& e) {
            // TODO(tanjpg): Error handling
          }
        },
        vda5050_master::VisualizationQos);
    }
  }

  void state_callback(const nlohmann::json& state_json)
  {
    // Deserialize JSON to typed message
    vda5050_msgs::msg::State state_msg;
    vda5050_msgs::msg::from_json(state_json, state_msg);

    // Store in container (optional - for history/debugging)
    if (config_->state_container) {
      config_->state_container->push_back(state_msg);
    }

    // TODO(tanjpg): Execute user-defined business logic here
    // e.g., update AGV state in a state manager, trigger workflows, etc.
  }

  void factsheet_callback(const nlohmann::json& factsheet_json)
  {
    // Deserialize and store
    vda5050_msgs::msg::Factsheet factsheet_msg;
    vda5050_msgs::msg::from_json(factsheet_json, factsheet_msg);

    if (config_->factsheet_container) {
      config_->factsheet_container->push_back(factsheet_msg);
    }

    // TODO(tanjpg): Process factsheet
  }

  void visualization_callback(const nlohmann::json& visualization_json)
  {
    // Deserialize and store
    vda5050_msgs::msg::Visualization viz_msg;
    vda5050_msgs::msg::from_json(visualization_json, viz_msg);

    if (config_->visualization_container) {
      config_->visualization_container->push_back(viz_msg);
    }

    // TODO(tanjpg): Process visualization
  }

  /**
   * @brief Publish an order to the AGV
   * @param order_json The order message as JSON
   */
  void publish_order(const nlohmann::json& order_json)
  {
    if (!config_->order_topic.has_value() || config_->order_topic->empty()) {
      throw std::runtime_error("Order topic not configured");
    }

    communication_client_->send_message(
      config_->order_topic.value(), order_json.dump(),
      vda5050_master::OrderQos);
  }

  /**
   * @brief Publish instant actions to the AGV
   * @param instant_actions_json The instant actions message as JSON
   */
  void publish_instant_actions(const nlohmann::json& instant_actions_json)
  {
    if (
      !config_->instant_actions_topic.has_value() ||
      config_->instant_actions_topic->empty()) {
      throw std::runtime_error("Instant actions topic not configured");
    }

    communication_client_->send_message(
      config_->instant_actions_topic.value(), instant_actions_json.dump(),
      vda5050_master::InstantActionsQos);
  }

private:
  std::unique_ptr<ICommunicationStrategy> communication_client_;
  std::unique_ptr<VDA5050Config> config_;
};

// ###########################
// IMPLEMENTATION EXAMPLE: How MQTT adapts to the common callback signature
// ###########################
/*
class MqttCommunication : public ICommunicationStrategy
{
public:
  void subscribe(const std::string& topic, MessageCallback callback,
                 const int qos = 0) override
  {
    // MQTT native callback already matches our signature!
    // Paho MQTT callback: void(const string& topic, const string& payload)

    mqtt_client_->subscribe(topic, qos,
      [callback](const std::string& mqtt_topic, const std::string& mqtt_payload) {
        // Direct pass-through - no adaptation needed
        callback(mqtt_topic, mqtt_payload);
      });
  }
};
*/
// ###########################

// ###########################
// IMPLEMENTATION EXAMPLE: How ROS2 adapts to the common callback signature
// ###########################
/*
class Ros2Communication : public ICommunicationStrategy
{
public:
  void subscribe(const std::string& topic, MessageCallback callback,
                 const int qos = 0) override
  {
    // ROS2 native callback: void(const MessageType::SharedPtr msg)
    // We need to adapt: structured message → JSON string → common callback

    auto ros2_callback = [topic, callback](const std_msgs::msg::String::SharedPtr msg) {
      // Adapter layer: Convert ROS2 message to JSON string
      std::string json_payload = msg->data;  // Assuming data is already JSON

      // Or if you need to serialize a structured message:
      // nlohmann::json j;
      // j["data"] = msg->data;
      // std::string json_payload = j.dump();

      // Now call the common callback
      callback(topic, json_payload);
    };

    // Create ROS2 subscription with our adapter callback
    ros2_subscription_ = node_->create_subscription<std_msgs::msg::String>(
      topic, qos, ros2_callback);
  }
};
*/
// ###########################

#endif  // VDA5050_MASTER__COMMUNICATION__VDA5050_HPP_
