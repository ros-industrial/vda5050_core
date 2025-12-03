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

#include "vda5050_master/communication/mqtt.hpp"

#include "vda5050_core/logger/logger.hpp"

MqttCommunication::MqttCommunication(
  const std::string& endpoint, const std::string& id)
: ICommunicationStrategy(), endpoint_(endpoint), id_(id)
{
  mqtt_client_ =
    vda5050_core::mqtt_client::create_default_client(endpoint_, id_);
}

void MqttCommunication::subscribe(
  const std::string& topic, MessageCallback callback, const int qos)
{
  mqtt_client_->subscribe(
    topic,
    [callback](const std::string& topic_, const std::string& payload_) {
      callback(topic_, payload_);
    },
    qos);
  VDA5050_INFO("[MQTT] Subscribing to topic at {}", topic);
}

void MqttCommunication::unsubscribe(const std::string& topic)
{
  mqtt_client_->unsubscribe(topic);
  VDA5050_INFO("[MQTT] Unsubscribed from topic: {}", topic);
}

void MqttCommunication::connect()
{
  mqtt_client_->connect();
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    state_ = ConnectionState::CONNECTED;
  }
  VDA5050_INFO("[MQTT] Connecting to broker at {}", endpoint_);
}

void MqttCommunication::disconnect()
{
  VDA5050_INFO("[MQTT] Disconnecting from MQTT broker");

  // Signal disconnecting state
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    state_ = ConnectionState::DISCONNECTING;
  }

  mqtt_client_->disconnect();

  // Mark as fully disconnected
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    state_ = ConnectionState::DISCONNECTED;
  }

  VDA5050_INFO("[MQTT] Disconnected from MQTT broker");
}

ConnectionState MqttCommunication::get_state() const
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  return state_;
}

void MqttCommunication::send_message(
  const std::string& topic, const std::string& message, const int qos)
{
  mqtt_client_->publish(topic, message, qos);
  VDA5050_INFO("[MQTT] Publishing message: {}", message);
}
