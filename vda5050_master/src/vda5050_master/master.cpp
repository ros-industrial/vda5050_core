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

#include <utility>

#include "vda5050_core/logger/logger.hpp"

// ============================================================================
// VDA5050Master Implementation
// ============================================================================

VDA5050Master::VDA5050Master(CommunicationFactory factory)
: communication_factory_(std::move(factory))
{
}

void VDA5050Master::register_agv(
  const std::string& manufacturer, const std::string& serial_number)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::lock_guard<std::mutex> lock(agv_mutex_);

  if (get_agv(agv_id))
  {
    VDA5050_WARN("[VDA5050Master] AGV already registered: {}", agv_id);
    return;
  }

  // Create communication instance for this AGV using the factory
  auto communication = communication_factory_(agv_id);

  // Create AGV object
  auto agv = std::make_shared<AGV>(
    manufacturer, serial_number, std::move(communication));

  // Connect the AGV's communication
  agv->connect();

  // Setup subscriptions for this AGV, routing callbacks to virtual methods
  agv->setup_subscriptions(
    [this](const std::string& id, const vda5050_msgs::msg::Connection& msg) {
      on_connection(id, msg);
    },
    [this](const std::string& id, const vda5050_msgs::msg::State& msg) {
      on_state(id, msg);
    },
    [this](const std::string& id, const vda5050_msgs::msg::Factsheet& msg) {
      on_factsheet(id, msg);
    },
    [this](const std::string& id, const vda5050_msgs::msg::Visualization& msg) {
      on_visualization(id, msg);
    });

  // Store the AGV
  agvs_[agv_id] = std::move(agv);

  VDA5050_INFO("[VDA5050Master] Registered AGV: {}", agv_id);
}

void VDA5050Master::unregister_agv(
  const std::string& manufacturer, const std::string& serial_number)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::lock_guard<std::mutex> lock(agv_mutex_);

  auto agv = get_agv(agv_id);
  if (!agv)
  {
    VDA5050_WARN(
      "[VDA5050Master] Cannot unregister: AGV not found: {}", agv_id);
    return;
  }

  // Disconnect the AGV's communication
  agv->disconnect();

  // Remove from map
  agvs_.erase(agv_id);

  VDA5050_INFO("[VDA5050Master] Unregistered AGV: {}", agv_id);
}

bool VDA5050Master::is_agv_registered(
  const std::string& manufacturer, const std::string& serial_number) const
{
  std::lock_guard<std::mutex> lock(agv_mutex_);
  std::string agv_id = manufacturer + "/" + serial_number;
  return get_agv(agv_id) != nullptr;
}

std::shared_ptr<AGV> VDA5050Master::get_agv(const std::string& agv_id) const
{
  auto it = agvs_.find(agv_id);
  return (it != agvs_.end()) ? it->second : nullptr;
}

void VDA5050Master::publish_order(
  const std::string& manufacturer, const std::string& serial_number,
  const vda5050_msgs::msg::Order& order)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::lock_guard<std::mutex> lock(agv_mutex_);

  auto agv = get_agv(agv_id);
  if (!agv)
  {
    throw std::runtime_error(
      "Cannot publish order: AGV not registered: " + agv_id);
  }

  agv->send_order(order);
}

void VDA5050Master::publish_instant_actions(
  const std::string& manufacturer, const std::string& serial_number,
  const vda5050_msgs::msg::InstantActions& actions)
{
  std::string agv_id = manufacturer + "/" + serial_number;

  std::lock_guard<std::mutex> lock(agv_mutex_);

  auto agv = get_agv(agv_id);
  if (!agv)
  {
    throw std::runtime_error(
      "Cannot publish instant actions: AGV not registered: " + agv_id);
  }

  agv->send_instant_actions(actions);
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
