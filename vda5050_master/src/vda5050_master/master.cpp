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

#include <sstream>
#include <utility>
#include <vector>

#include "nlohmann/json.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_json_utils/serialization.hpp"
#include "vda5050_master/standard_names.hpp"

namespace vda5050_master {

// ============================================================================
// Constructor / Destructor
// ============================================================================

VDA5050Master::VDA5050Master(
  std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface> mqtt_client,
  const std::string& broker_address)
: mqtt_client_(std::move(mqtt_client)), broker_address_(broker_address)
{
  VDA5050_INFO("[VDA5050Master] Created VDA5050Master instance");
}

VDA5050Master::~VDA5050Master()
{
  VDA5050_INFO("[VDA5050Master] Destroying VDA5050Master instance");
  disconnect();
  VDA5050_INFO("[VDA5050Master] VDA5050Master instance destroyed");
}

// ============================================================================
// Connection Management
// ============================================================================

void VDA5050Master::connect()
{
  if (!mqtt_client_)
  {
    VDA5050_WARN("[VDA5050Master] Cannot connect: no MQTT client");
    return;
  }

  if (mqtt_client_->connected())
  {
    VDA5050_WARN("[VDA5050Master] Already connected");
    return;
  }

  VDA5050_INFO("[VDA5050Master] Connecting MQTT client");
  mqtt_client_->connect();
  setup_subscriptions();
  VDA5050_INFO("[VDA5050Master] Connected and subscribed to wildcard topics");
}

void VDA5050Master::disconnect()
{
  if (!mqtt_client_)
  {
    return;
  }

  if (!mqtt_client_->connected())
  {
    return;
  }

  VDA5050_INFO("[VDA5050Master] Disconnecting MQTT client");
  cleanup_subscriptions();
  mqtt_client_->disconnect();
  VDA5050_INFO("[VDA5050Master] Disconnected");
}

bool VDA5050Master::is_connected() const
{
  return mqtt_client_ && mqtt_client_->connected();
}

// ============================================================================
// Subscription Management
// ============================================================================

void VDA5050Master::setup_subscriptions()
{
  if (!mqtt_client_)
  {
    return;
  }

  // Subscribe to connection topic with wildcard
  std::string connection_topic = build_wildcard_topic(ConnectionTopic);
  mqtt_client_->subscribe(
    connection_topic,
    [this](const std::string& topic, const std::string& payload) {
      handle_connection_message(topic, payload);
    },
    ConnectionQos);
  VDA5050_INFO("[VDA5050Master] Subscribed to: {}", connection_topic);

  // Subscribe to state topic with wildcard
  std::string state_topic = build_wildcard_topic(StateTopic);
  mqtt_client_->subscribe(
    state_topic,
    [this](const std::string& topic, const std::string& payload) {
      handle_state_message(topic, payload);
    },
    StateQos);
  VDA5050_INFO("[VDA5050Master] Subscribed to: {}", state_topic);

  // Subscribe to factsheet topic with wildcard
  std::string factsheet_topic = build_wildcard_topic(FactsheetTopic);
  mqtt_client_->subscribe(
    factsheet_topic,
    [this](const std::string& topic, const std::string& payload) {
      handle_factsheet_message(topic, payload);
    },
    FactsheetQos);
  VDA5050_INFO("[VDA5050Master] Subscribed to: {}", factsheet_topic);

  // Subscribe to visualization topic with wildcard
  std::string visualization_topic = build_wildcard_topic(VisualizationTopic);
  mqtt_client_->subscribe(
    visualization_topic,
    [this](const std::string& topic, const std::string& payload) {
      handle_visualization_message(topic, payload);
    },
    VisualizationQos);
  VDA5050_INFO("[VDA5050Master] Subscribed to: {}", visualization_topic);
}

void VDA5050Master::cleanup_subscriptions()
{
  if (!mqtt_client_)
  {
    return;
  }

  mqtt_client_->unsubscribe(build_wildcard_topic(ConnectionTopic));
  mqtt_client_->unsubscribe(build_wildcard_topic(StateTopic));
  mqtt_client_->unsubscribe(build_wildcard_topic(FactsheetTopic));
  mqtt_client_->unsubscribe(build_wildcard_topic(VisualizationTopic));

  VDA5050_INFO("[VDA5050Master] Unsubscribed from wildcard topics");
}

// ============================================================================
// Topic Utilities
// ============================================================================

std::string VDA5050Master::build_wildcard_topic(const std::string& topic_name)
{
  return InterfaceName + "/" + Version + "/+/+/" + topic_name;
}

std::pair<std::string, std::string> VDA5050Master::parse_topic(
  const std::string& topic)
{
  // Topic structure: {interfaceName}/{version}/{manufacturer}/{serialNumber}/{topic}
  // Example: "rmf2/v2/MyManufacturer/AGV001/state"
  // Parts:    [0]   [1]     [2]          [3]     [4]

  std::vector<std::string> parts;
  std::istringstream stream(topic);
  std::string part;

  while (std::getline(stream, part, '/'))
  {
    parts.push_back(part);
  }

  // We need at least 5 parts
  if (parts.size() < 5)
  {
    return {"", ""};
  }

  // Return manufacturer (index 2) and serial number (index 3)
  return {parts[2], parts[3]};
}

// ============================================================================
// AGV Onboarding/Offboarding
// ============================================================================

void VDA5050Master::onboard_agv(
  const std::string& manufacturer, const std::string& serial_number,
  size_t max_queue_size, bool drop_oldest)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::lock_guard<std::mutex> lock(agv_mutex_);

  if (get_agv_by_id(agv_id))
  {
    VDA5050_WARN("[VDA5050Master] AGV already onboarded: {}", agv_id);
    return;
  }

  // Create AGV instance with broker address for transient publishing
  auto agv = std::make_shared<AGV>(
    manufacturer, serial_number, broker_address_, max_queue_size, drop_oldest);

  agvs_[agv_id] = std::move(agv);

  VDA5050_INFO("[VDA5050Master] Onboarded AGV: {}", agv_id);
}

void VDA5050Master::offboard_agv(
  const std::string& manufacturer, const std::string& serial_number)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::shared_ptr<AGV> agv;
  {
    std::lock_guard<std::mutex> lock(agv_mutex_);
    auto it = agvs_.find(agv_id);
    if (it == agvs_.end())
    {
      VDA5050_WARN(
        "[VDA5050Master] Cannot offboard: AGV not found: {}", agv_id);
      return;
    }
    agv = std::move(it->second);
    agvs_.erase(it);
  }

  // Stop AGV after removing from map
  agv->stop();

  VDA5050_INFO("[VDA5050Master] Offboarded AGV: {}", agv_id);
}

bool VDA5050Master::is_agv_onboarded(
  const std::string& manufacturer, const std::string& serial_number) const
{
  std::lock_guard<std::mutex> lock(agv_mutex_);
  std::string agv_id = manufacturer + "/" + serial_number;
  return get_agv_by_id(agv_id) != nullptr;
}

// ============================================================================
// AGV Access
// ============================================================================

std::shared_ptr<AGV> VDA5050Master::get_agv(
  const std::string& manufacturer, const std::string& serial_number) const
{
  std::lock_guard<std::mutex> lock(agv_mutex_);
  std::string agv_id = manufacturer + "/" + serial_number;
  return get_agv_by_id(agv_id);
}

std::shared_ptr<AGV> VDA5050Master::get_agv_by_id(
  const std::string& agv_id) const
{
  auto it = agvs_.find(agv_id);
  return (it != agvs_.end()) ? it->second : nullptr;
}

std::pair<std::string, std::shared_ptr<AGV>> VDA5050Master::get_agv_from_topic(
  const std::string& topic, const std::string& message_type)
{
  auto [manufacturer, serial_number] = parse_topic(topic);
  if (manufacturer.empty() || serial_number.empty())
  {
    VDA5050_WARN(
      "[VDA5050Master] Ignoring {} message: failed to parse topic: {}",
      message_type, topic);
    return {"", nullptr};
  }

  std::string agv_id = manufacturer + "/" + serial_number;

  std::shared_ptr<AGV> agv;
  {
    std::lock_guard<std::mutex> lock(agv_mutex_);
    agv = get_agv_by_id(agv_id);
  }

  if (!agv)
  {
    VDA5050_WARN(
      "[VDA5050Master] Ignoring {} message from AGV not onboarded: {}",
      message_type, agv_id);
    return {agv_id, nullptr};
  }

  return {agv_id, agv};
}

// ============================================================================
// Outgoing Messages
// ============================================================================

bool VDA5050Master::publish_order(
  const std::string& manufacturer, const std::string& serial_number,
  const vda5050_types::Order& order)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::shared_ptr<AGV> agv;
  {
    std::lock_guard<std::mutex> lock(agv_mutex_);
    agv = get_agv_by_id(agv_id);
  }

  if (!agv)
  {
    throw std::runtime_error(
      "Cannot publish order: AGV not onboarded: " + agv_id);
  }

  return agv->send_order(order);
}

bool VDA5050Master::publish_instant_actions(
  const std::string& manufacturer, const std::string& serial_number,
  const vda5050_types::InstantActions& actions)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::shared_ptr<AGV> agv;
  {
    std::lock_guard<std::mutex> lock(agv_mutex_);
    agv = get_agv_by_id(agv_id);
  }

  if (!agv)
  {
    throw std::runtime_error(
      "Cannot publish instant actions: AGV not onboarded: " + agv_id);
  }

  return agv->send_instant_actions(actions);
}

// ============================================================================
// Message Handlers
// ============================================================================

template <typename MsgType>
void VDA5050Master::handle_message(
  const std::string& topic, const std::string& payload,
  const std::string& message_type, void (AGV::*agv_handler)(const MsgType&),
  void (VDA5050Master::*callback)(const std::string&, const MsgType&))
{
  auto [agv_id, agv] = get_agv_from_topic(topic, message_type);
  if (!agv)
  {
    return;
  }

  try
  {
    nlohmann::json j = nlohmann::json::parse(payload);
    MsgType msg;
    vda5050_types::from_json(j, msg);

    (agv.get()->*agv_handler)(msg);
    (this->*callback)(agv_id, msg);
  }
  catch (const std::exception& e)
  {
    VDA5050_WARN(
      "[VDA5050Master] Failed to parse {} message from {}: {}", message_type,
      agv_id, e.what());
  }
}

void VDA5050Master::handle_connection_message(
  const std::string& topic, const std::string& payload)
{
  handle_message<vda5050_types::Connection>(
    topic, payload, "connection", &AGV::handle_connection,
    &VDA5050Master::on_connection);
}

void VDA5050Master::handle_state_message(
  const std::string& topic, const std::string& payload)
{
  handle_message<vda5050_types::State>(
    topic, payload, "state", &AGV::handle_state, &VDA5050Master::on_state);
}

void VDA5050Master::handle_factsheet_message(
  const std::string& topic, const std::string& payload)
{
  handle_message<vda5050_types::Factsheet>(
    topic, payload, "factsheet", &AGV::handle_factsheet,
    &VDA5050Master::on_factsheet);
}

void VDA5050Master::handle_visualization_message(
  const std::string& topic, const std::string& payload)
{
  handle_message<vda5050_types::Visualization>(
    topic, payload, "visualization", &AGV::handle_visualization,
    &VDA5050Master::on_visualization);
}

// ============================================================================
// Default Virtual Callback Implementations
// ============================================================================

void VDA5050Master::on_connection(
  const std::string& agv_id, const vda5050_types::Connection& /*msg*/)
{
  VDA5050_WARN(
    "[VDA5050Master] on_connection not overridden. Received connection from "
    "AGV: {}",
    agv_id);
}

void VDA5050Master::on_state(
  const std::string& agv_id, const vda5050_types::State& /*msg*/)
{
  VDA5050_WARN(
    "[VDA5050Master] on_state not overridden. Received state from AGV: {}",
    agv_id);
}

void VDA5050Master::on_factsheet(
  const std::string& agv_id, const vda5050_types::Factsheet& /*msg*/)
{
  VDA5050_WARN(
    "[VDA5050Master] on_factsheet not overridden. Received factsheet from "
    "AGV: {}",
    agv_id);
}

void VDA5050Master::on_visualization(
  const std::string& agv_id, const vda5050_types::Visualization& /*msg*/)
{
  VDA5050_WARN(
    "[VDA5050Master] on_visualization not overridden. Received visualization "
    "from AGV: {}",
    agv_id);
}

}  // namespace vda5050_master
