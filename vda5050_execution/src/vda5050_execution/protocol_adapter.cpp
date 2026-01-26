/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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

#include "vda5050_execution/protocol_adapter.hpp"

namespace vda5050_execution {

//=============================================================================
std::shared_ptr<ProtocolAdapter> ProtocolAdapter::make(
  std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface> mqtt_client,
  const std::string& interface, const std::string& version,
  const std::string& manufacturer, const std::string serial_number)
{
  auto adapter = std::shared_ptr<ProtocolAdapter>(new ProtocolAdapter(
    mqtt_client, interface, version, manufacturer, serial_number));
  return adapter;
}

//=============================================================================
ProtocolAdapter::ProtocolAdapter(
  std::shared_ptr<vda5050_core::mqtt_client::MqttClientInterface> mqtt_client,
  const std::string& interface, const std::string& version,
  const std::string& manufacturer, const std::string serial_number)
: mqtt_client_(mqtt_client),
  interface_(interface),
  version_(version),
  manufacturer_(manufacturer),
  serial_number_(serial_number)
{
  std::string topic_prefix = fmt::format(
    "{}/{}/{}/{}", interface_, version_, manufacturer_, serial_number_);

  topic_names_ = {
    {std::type_index(typeid(vda5050_types::Connection)),
     fmt::format("{}/connection", topic_prefix)},
    {std::type_index(typeid(vda5050_types::State)),
     fmt::format("{}/state", topic_prefix)},
    {std::type_index(typeid(vda5050_types::Order)),
     fmt::format("{}/order", topic_prefix)},
    {std::type_index(typeid(vda5050_types::InstantActions)),
     fmt::format("{}/instantActions", topic_prefix)},
    {std::type_index(typeid(vda5050_types::Factsheet)),
     fmt::format("{}/factsheet", topic_prefix)},
    {std::type_index(typeid(vda5050_types::Visualization)),
     fmt::format("{}/visualization", topic_prefix)}};

  header_ids_ = {
    {std::type_index(typeid(vda5050_types::Connection)), 0},
    {std::type_index(typeid(vda5050_types::State)), 0},
    {std::type_index(typeid(vda5050_types::Order)), 0},
    {std::type_index(typeid(vda5050_types::InstantActions)), 0},
    {std::type_index(typeid(vda5050_types::Factsheet)), 0},
    {std::type_index(typeid(vda5050_types::Visualization)), 0}};
}

}  // namespace vda5050_execution
