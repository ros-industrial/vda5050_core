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

#ifndef VDA5050_MASTER__COMMUNICATION__MQTT_HPP_
#define VDA5050_MASTER__COMMUNICATION__MQTT_HPP_

#include <iostream>
#include <memory>
#include <string>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"
#include "vda5050_master/communication/communication.hpp"

using vda5050_core::mqtt_client::MqttClientInterface;

class MqttCommunication : public ICommunicationStrategy
{
public:
  MqttCommunication(const std::string& endpoint, const std::string& id)
  : ICommunicationStrategy(), endpoint_(endpoint), id_(id)
  {
    mqtt_client_ =
      vda5050_core::mqtt_client::create_default_client(endpoint_, id_);
  }

  void subscribe(
    const std::string& topic, MessageCallback callback,
    const int qos = 0) override
  {
    mqtt_client_->subscribe(
      topic,
      [callback](const std::string& topic_, const std::string& payload_) {
        callback(topic_, payload_);
      },
      qos);
    VDA5050_INFO("[MQTT] Subscribing to topic at " + topic);
  }

  void connect() override
  {
    mqtt_client_->connect();
    VDA5050_INFO("[MQTT] Connecting to broker at " + endpoint_);
  }

  void disconnect() override
  {
    mqtt_client_->disconnect();
    VDA5050_WARN("Disconnecting from MQTT broker");
  }

  void send_message(
    const std::string& topic, const std::string& message,
    const int qos = 0) override
  {
    mqtt_client_->publish(topic, message, qos);
    std::cout << "[MQTT] Publishing message: " << message << std::endl;
  }

private:
  std::shared_ptr<MqttClientInterface> mqtt_client_;
  std::string endpoint_;
  std::string id_;
};
#endif  // VDA5050_MASTER__COMMUNICATION__MQTT_HPP_
