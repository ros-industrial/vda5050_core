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

#ifndef VDA5050_MASTER__MQTT__MQTT_HPP_
#define VDA5050_MASTER__MQTT__MQTT_HPP_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"
#include "vda5050_master/standard_names.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

using vda5050_core::mqtt_client::MqttClientInterface;

namespace vda5050_master {
namespace communication {

class AGVClient
{
public:
  AGVClient(
    std::unique_ptr<ICommunicationStrategy> connection_client,
    const std::string& broker_address, const std::string& manufacturer,
    const std::string& serial_number)
  : connection_client_(std::move(connection_client)),
    manufacturer_(manufacturer),
    serial_number_(serial_number)
  {
    std::string topic_prefix = vda5050_master::InterfaceName + "/" +
                               vda5050_master::Version + "/" + manufacturer +
                               "/" + serial_number + "/";

    connection_topic_ = topic_prefix + vda5050_master::ConnectionTopic;
    state_topic_ = topic_prefix + vda5050_master::StateTopic;
    factsheet_topic_ = topic_prefix + vda5050_master::FactsheetTopic;
    order_topic_ = topic_prefix + vda5050_master::OrderTopic;
    instant_actions_topic_ = topic_prefix + vda5050_master::InstantActionsTopic;
    visualization_topic_ = topic_prefix + vda5050_master::VisualizationTopic;

    // Setup connection heartbeat listener
    connection_heartbeat_listener_ =
      std::make_shared<ConnectionHeartbeatListener>(
        connection_topic_, vda5050_master::ConnectionHeartbeatInterval, [&]() {
          mqtt_client_->disconnect();
          VDA5050_WARN("Connection lost. Disconnected from MQTT broker");
        });
    connection_heartbeat_listener_->start_connection_heartbeat();

    communication_client_->connect();

    // Subsctibe to connection
    communication_client_->subscribe(
      connection_topic_,
      [&](const std::string& topic_, const std::string& payload_) {
        connection_success_callback();
      },
      vda5050_master::ConnectionQos);
  }

  void connection_success_callback()
  {
    connection_heartbeat_listener_->received_connection();
    setup_client();
  }

  ~AGVClient() {}

private:
  void setup_client()
  {
    // Subsctibe to state
    mqtt_client_->subscribe(
      state_topic_,
      [&](const std::string& topic_, const std::string& payload_) { return; },
      vda5050_master::StateQos);

    // Subsctibe to factsheet
    mqtt_client_->subscribe(
      factsheet_topic_,
      [&](const std::string& topic_, const std::string& payload_) { return; },
      vda5050_master::FactsheetQos);
  }

  std::shared_ptr<MqttClientInterface> mqtt_client_;

  std::shared_ptr<ConnectionHeartbeatListener> connection_heartbeat_listener_;

  std::unique_ptr<ICommunicationStrategy> communication_client_;

  std::string connection_topic_;

  std::string state_topic_;

  std::string factsheet_topic_;

  std::string order_topic_;

  std::string instant_actions_topic_;

  std::string visualization_topic_;

  std::string manufacturer_;

  std::string serial_number_;
};
}  // namespace communication
}  // namespace vda5050_master

#endif  // VDA5050_MASTER__MQTT__MQTT_HPP_
