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

#include <utility>

#include "vda5050_core/execution/protocol_adapter.hpp"

namespace vda5050_core {

namespace execution {

//=============================================================================
std::shared_ptr<ProtocolAdapter> ProtocolAdapter::make(
  std::shared_ptr<transport::MqttClientInterface> mqtt_client,
  const std::string& interface, const std::string& version,
  const std::string& manufacturer, const std::string& serial_number)
{
  auto adapter = std::shared_ptr<ProtocolAdapter>(new ProtocolAdapter(
    std::move(mqtt_client), interface, version, manufacturer, serial_number));
  return adapter;
}

//=============================================================================
void ProtocolAdapter::connect()
{
  if (!mqtt_client_) return;

  try
  {
    mqtt_client_->connect();
  }
  catch (const std::exception& e)
  {
    VDA5050_ERROR("Unexpected error during client connection: {}", e.what());
  }
}

//=============================================================================
void ProtocolAdapter::disconnect()
{
  if (!mqtt_client_) return;

  try
  {
    mqtt_client_->disconnect();
  }
  catch (const std::exception& e)
  {
    VDA5050_ERROR("Unexpected error during client disconnection: {}", e.what());
  }
}

//=============================================================================
bool ProtocolAdapter::connected()
{
  if (mqtt_client_) return mqtt_client_->connected();
  return false;
}

//=============================================================================
std::string ProtocolAdapter::get_topic_version(const std::string& version)
{
  if (version.empty()) return version;

  // Already in topic form without patch/minor segments (e.g. "v2").
  if (version.front() == 'v' && version.find('.') == std::string::npos)
  {
    return version;
  }

  std::string major;
  if (version.front() == 'v')
  {
    // Semver with leading v (e.g. "v2.0.0").
    const auto start = std::size_t{1};
    const auto dot = version.find('.', start);
    major = (dot == std::string::npos) ? version.substr(start)
                                       : version.substr(start, dot - start);
  }
  else
  {
    // Semver or major-only (e.g. "2.0.0" or "2").
    const auto dot = version.find('.');
    major = (dot == std::string::npos) ? version : version.substr(0, dot);
  }

  return "v" + major;
}

//=============================================================================
ProtocolAdapter::ProtocolAdapter(
  std::shared_ptr<vda5050_core::transport::MqttClientInterface> mqtt_client,
  const std::string& interface, const std::string& version,
  const std::string& manufacturer, const std::string& serial_number)
: mqtt_client_(std::move(mqtt_client)),
  interface_(interface),
  version_(version),
  topic_version_(get_topic_version(version)),
  manufacturer_(manufacturer),
  serial_number_(serial_number)
{
  std::string topic_prefix = fmt::format(
    "{}/{}/{}/{}", interface_, topic_version_, manufacturer_, serial_number_);

  topic_names_ = {
    {std::type_index(typeid(vda5050_core::types::Connection)),
     fmt::format("{}/connection", topic_prefix)},
    {std::type_index(typeid(vda5050_core::types::State)),
     fmt::format("{}/state", topic_prefix)},
    {std::type_index(typeid(vda5050_core::types::Order)),
     fmt::format("{}/order", topic_prefix)},
    {std::type_index(typeid(vda5050_core::types::InstantActions)),
     fmt::format("{}/instantActions", topic_prefix)},
    {std::type_index(typeid(vda5050_core::types::Factsheet)),
     fmt::format("{}/factsheet", topic_prefix)},
    {std::type_index(typeid(vda5050_core::types::Visualization)),
     fmt::format("{}/visualization", topic_prefix)}};

  header_ids_ = {
    {std::type_index(typeid(vda5050_core::types::Connection)), 0},
    {std::type_index(typeid(vda5050_core::types::State)), 0},
    {std::type_index(typeid(vda5050_core::types::Order)), 0},
    {std::type_index(typeid(vda5050_core::types::InstantActions)), 0},
    {std::type_index(typeid(vda5050_core::types::Factsheet)), 0},
    {std::type_index(typeid(vda5050_core::types::Visualization)), 0}};
}

}  // namespace execution
}  // namespace vda5050_core
