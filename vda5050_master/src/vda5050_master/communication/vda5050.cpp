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

#include "vda5050_master/communication/vda5050.hpp"

Vda5050Communication::Vda5050Communication(
  std::unique_ptr<ICommunicationStrategy> comm_client,
  VDA5050TopicConfig config, VDA5050Handlers handlers)
: comm_client_(std::move(comm_client)),
  config_(std::move(config)),
  handlers_(std::move(handlers))
{
  config_.validate();
  handlers_.validate();
}

void Vda5050Communication::start_communication()
{
  comm_client_->connect();

  // Subscribe to required topics
  subscribe_to_topic<vda5050_msgs::msg::Connection>(
    config_.connection_topic, handlers_.on_connection,
    vda5050_master::ConnectionQos);

  subscribe_to_topic<vda5050_msgs::msg::State>(
    config_.state_topic, handlers_.on_state, vda5050_master::StateQos);

  // Subscribe to optional topics if configured (handlers always exist)
  if (config_.has_factsheet())
  {
    subscribe_to_topic<vda5050_msgs::msg::Factsheet>(
      config_.factsheet_topic.value(), handlers_.on_factsheet,
      vda5050_master::FactsheetQos);
  }

  if (config_.has_visualization())
  {
    subscribe_to_topic<vda5050_msgs::msg::Visualization>(
      config_.visualization_topic.value(), handlers_.on_visualization,
      vda5050_master::VisualizationQos);
  }
}

void Vda5050Communication::stop_communication()
{
  comm_client_->disconnect();
}

void Vda5050Communication::publish_order(const vda5050_msgs::msg::Order& order)
{
  if (!config_.has_order())
  {
    throw std::runtime_error("Order topic not configured");
  }

  nlohmann::json j;
  vda5050_msgs::msg::to_json(j, order);
  comm_client_->send_message(
    config_.order_topic.value(), j.dump(), vda5050_master::OrderQos);
}

void Vda5050Communication::publish_instant_actions(
  const vda5050_msgs::msg::InstantActions& actions)
{
  if (!config_.has_instant_actions())
  {
    throw std::runtime_error("Instant actions topic not configured");
  }

  nlohmann::json j;
  vda5050_msgs::msg::to_json(j, actions);
  comm_client_->send_message(
    config_.instant_actions_topic.value(), j.dump(),
    vda5050_master::InstantActionsQos);
}
