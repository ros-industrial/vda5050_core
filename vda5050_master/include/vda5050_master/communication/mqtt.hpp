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

#include <memory>
#include <mutex>
#include <string>

#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"
#include "vda5050_master/communication/communication.hpp"

class MqttCommunication : public ICommunicationStrategy
{
public:
  MqttCommunication(const std::string& endpoint, const std::string& id);

  void subscribe(
    const std::string& topic, MessageCallback callback,
    const int qos = 0) override;

  void unsubscribe(const std::string& topic) override;

  void connect() override;

  void disconnect() override;

  ConnectionState get_state() const override;

  void send_message(
    const std::string& topic, const std::string& message,
    const int qos = 0) override;

private:
  std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface> mqtt_client_;
  std::string endpoint_;
  std::string id_;
  ConnectionState state_{ConnectionState::DISCONNECTED};
  mutable std::mutex state_mutex_;
};

#endif  // VDA5050_MASTER__COMMUNICATION__MQTT_HPP_
