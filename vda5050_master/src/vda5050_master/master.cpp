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

#include "vda5050_master/vda5050_master/master.hpp"

#include "nlohmann/json.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_master/standard_names.hpp"

// ============================================================================
// VDA5050Master Implementation
// ============================================================================

VDA5050Master::VDA5050Master(
  std::unique_ptr<ICommunicationStrategy> communication)
: communication_(std::move(communication))
{
}

void VDA5050Master::connect()
{
  communication_->connect();
  VDA5050_INFO("[VDA5050Master] Connected to communication endpoint");
}

void VDA5050Master::disconnect()
{
  communication_->disconnect();
  VDA5050_INFO("[VDA5050Master] Disconnected from communication endpoint");
}

void VDA5050Master::register_agv(const AGVInfo& agv_info)
{
  std::string agv_id = agv_info.get_agv_id();

  {
    std::lock_guard<std::mutex> lock(agv_registry_mutex_);

    if (agv_registry_.find(agv_id) != agv_registry_.end())
    {
      VDA5050_WARN("[VDA5050Master] AGV already registered: {}", agv_id);
      return;
    }

    agv_registry_[agv_id] = agv_info;
  }

  // Build topic configuration for this AGV
  auto topic_config = build_topic_config(agv_info);

  // Subscribe to connection topic
  communication_->subscribe(
    topic_config.connection_topic,
    [this, agv_id](const std::string& /*topic*/, const std::string& payload) {
      handle_connection_message(agv_id, payload);
    },
    vda5050_master::ConnectionQos);

  // Subscribe to state topic
  communication_->subscribe(
    topic_config.state_topic,
    [this, agv_id](const std::string& /*topic*/, const std::string& payload) {
      handle_state_message(agv_id, payload);
    },
    vda5050_master::StateQos);

  // Subscribe to factsheet topic
  if (topic_config.has_factsheet())
  {
    communication_->subscribe(
      topic_config.factsheet_topic.value(),
      [this, agv_id](const std::string& /*topic*/, const std::string& payload) {
        handle_factsheet_message(agv_id, payload);
      },
      vda5050_master::FactsheetQos);
  }

  // Subscribe to visualization topic
  if (topic_config.has_visualization())
  {
    communication_->subscribe(
      topic_config.visualization_topic.value(),
      [this, agv_id](const std::string& /*topic*/, const std::string& payload) {
        handle_visualization_message(agv_id, payload);
      },
      vda5050_master::VisualizationQos);
  }

  VDA5050_INFO("[VDA5050Master] Registered AGV: {}", agv_id);
}

void VDA5050Master::unregister_agv(
  const std::string& manufacturer, const std::string& serial_number)
{
  std::lock_guard<std::mutex> lock(agv_registry_mutex_);
  std::string agv_id = manufacturer + "/" + serial_number;
  agv_registry_.erase(agv_id);
  VDA5050_INFO("[VDA5050Master] Unregistered AGV: {}", agv_id);
}

bool VDA5050Master::is_agv_registered(
  const std::string& manufacturer, const std::string& serial_number) const
{
  std::lock_guard<std::mutex> lock(agv_registry_mutex_);
  std::string agv_id = manufacturer + "/" + serial_number;
  return agv_registry_.find(agv_id) != agv_registry_.end();
}

void VDA5050Master::publish_order(
  const std::string& manufacturer, const std::string& serial_number,
  const vda5050_msgs::msg::Order& order)
{
  if (!is_agv_registered(manufacturer, serial_number))
  {
    throw std::runtime_error(
      "Cannot publish order: AGV not registered: " + manufacturer + "/" +
      serial_number);
  }

  std::string topic =
    build_topic(manufacturer, serial_number, vda5050_master::OrderTopic);
  nlohmann::json j;
  vda5050_msgs::msg::to_json(j, order);
  communication_->send_message(topic, j.dump(), vda5050_master::OrderQos);

  VDA5050_INFO(
    "[VDA5050Master] Published order to AGV: {}/{}", manufacturer,
    serial_number);
}

void VDA5050Master::publish_instant_actions(
  const std::string& manufacturer, const std::string& serial_number,
  const vda5050_msgs::msg::InstantActions& actions)
{
  if (!is_agv_registered(manufacturer, serial_number))
  {
    throw std::runtime_error(
      "Cannot publish instant actions: AGV not registered: " + manufacturer +
      "/" + serial_number);
  }

  std::string topic = build_topic(
    manufacturer, serial_number, vda5050_master::InstantActionsTopic);
  nlohmann::json j;
  vda5050_msgs::msg::to_json(j, actions);
  communication_->send_message(
    topic, j.dump(), vda5050_master::InstantActionsQos);

  VDA5050_INFO(
    "[VDA5050Master] Published instant actions to AGV: {}/{}", manufacturer,
    serial_number);
}

// ============================================================================
// Default implementations for optional virtual callbacks
// ============================================================================

void VDA5050Master::on_factsheet(
  const std::string& agv_id, const vda5050_msgs::msg::Factsheet& /*msg*/)
{
  VDA5050_WARN(
    "[VDA5050Master] on_factsheet not overridden. Received factsheet from "
    "AGV: {}",
    agv_id);
}

void VDA5050Master::on_visualization(
  const std::string& agv_id, const vda5050_msgs::msg::Visualization& /*msg*/)
{
  VDA5050_WARN(
    "[VDA5050Master] on_visualization not overridden. Received visualization "
    "from AGV: {}",
    agv_id);
}

// ============================================================================
// Private helper methods
// ============================================================================

VDA5050TopicConfig VDA5050Master::build_topic_config(
  const AGVInfo& agv_info) const
{
  VDA5050TopicConfig config;

  std::string topic_base =
    vda5050_master::InterfaceName + "/" + vda5050_master::Version + "/" +
    agv_info.manufacturer + "/" + agv_info.serial_number + "/";

  config.connection_topic = topic_base + vda5050_master::ConnectionTopic;
  config.state_topic = topic_base + vda5050_master::StateTopic;
  config.factsheet_topic = topic_base + vda5050_master::FactsheetTopic;
  config.visualization_topic = topic_base + vda5050_master::VisualizationTopic;
  config.order_topic = topic_base + vda5050_master::OrderTopic;
  config.instant_actions_topic =
    topic_base + vda5050_master::InstantActionsTopic;

  return config;
}

std::string VDA5050Master::build_topic(
  const std::string& manufacturer, const std::string& serial_number,
  const std::string& topic_name) const
{
  return vda5050_master::InterfaceName + "/" + vda5050_master::Version + "/" +
         manufacturer + "/" + serial_number + "/" + topic_name;
}

void VDA5050Master::handle_connection_message(
  const std::string& agv_id, const std::string& payload)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::Connection msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    on_connection(agv_id, msg);
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[VDA5050Master] Failed to parse connection message from {}: {}", agv_id,
      e.what());
  }
}

void VDA5050Master::handle_state_message(
  const std::string& agv_id, const std::string& payload)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::State msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    on_state(agv_id, msg);
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[VDA5050Master] Failed to parse state message from {}: {}", agv_id,
      e.what());
  }
}

void VDA5050Master::handle_factsheet_message(
  const std::string& agv_id, const std::string& payload)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::Factsheet msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    on_factsheet(agv_id, msg);
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[VDA5050Master] Failed to parse factsheet message from {}: {}", agv_id,
      e.what());
  }
}

void VDA5050Master::handle_visualization_message(
  const std::string& agv_id, const std::string& payload)
{
  try
  {
    auto json_msg = nlohmann::json::parse(payload);
    vda5050_msgs::msg::Visualization msg;
    vda5050_msgs::msg::from_json(json_msg, msg);

    on_visualization(agv_id, msg);
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[VDA5050Master] Failed to parse visualization message from {}: {}",
      agv_id, e.what());
  }
}
