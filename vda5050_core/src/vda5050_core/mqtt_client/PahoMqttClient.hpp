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

#ifndef VDA5050_CORE__MQTT_CLIENT__PAHOMQTTCLIENT_HPP_
#define VDA5050_CORE__MQTT_CLIENT__PAHOMQTTCLIENT_HPP_

#include <mqtt/async_client.h>
#include <mqtt/callback.h>
#include <mqtt/connect_options.h>
#include <mqtt/iaction_listener.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "vda5050_core/mqtt_client/MqttClientInterface.hpp"

namespace vda5050_core {

namespace mqtt_client {

class PahoMqttClient;

//=============================================================================
class MqttActionListener : public mqtt::iaction_listener
{
public:
  MqttActionListener();

protected:
  void on_failure(const mqtt::token& tok) override;

  void on_success(const mqtt::token& tok) override;
};

//=============================================================================
class MqttCallback : public mqtt::callback
{
public:
  explicit MqttCallback(PahoMqttClient& parent);

protected:
  void connected(const std::string& cause) override;

  void connection_lost(const std::string& cause) override;

  void message_arrived(mqtt::const_message_ptr msg) override;

  void delivery_complete(mqtt::delivery_token_ptr tok) override;

private:
  PahoMqttClient& parent_;
};

//=============================================================================
class PahoMqttClient : public MqttClientInterface
{
public:
  static std::shared_ptr<PahoMqttClient> make(
    const std::string& broker_address, const std::string& client_id);

  ~PahoMqttClient();

  PahoMqttClient(const PahoMqttClient&) = delete;
  PahoMqttClient& operator=(const PahoMqttClient&) = delete;
  PahoMqttClient(PahoMqttClient&&) = delete;
  PahoMqttClient& operator=(PahoMqttClient&&) = delete;

  void connect() override;

  void disconnect() override;

  void publish(
    const std::string& topic, const std::string& message, int qos) override;

  void subscribe(
    const std::string& topic, MessageHandler handler, int qos) override;

  friend class MqttCallback;

private:
  PahoMqttClient(
    const std::string& broker_address, const std::string& client_id);

  std::unique_ptr<mqtt::async_client> client_;
  MqttActionListener action_listener_;
  MqttCallback callback_;

  std::unordered_map<std::string, MessageHandler> handlers_;
  std::mutex handler_mutex_;
};

}  // namespace mqtt_client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__MQTT_CLIENT__PAHOMQTTCLIENT_HPP_
