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

#include "vda5050_core/adapter/runtime.hpp"

#include <utility>

#include "vda5050_core/execution/protocol_adapter.hpp"
#include "vda5050_core/transport/mqtt_client_interface.hpp"

namespace vda5050_core {

namespace adapter {

std::shared_ptr<RobotRuntime> RobotRuntime::make(
  const std::string& broker_address, const std::string& client_id,
  const std::string& manufacturer, const std::string& serial_number,
  const std::string& interface, const std::string& version)
{
  std::shared_ptr<transport::MqttClientInterface> mqtt_client(
    transport::create_default_client_unique(broker_address, client_id));

  auto protocol = execution::ProtocolAdapter::make(
    mqtt_client, interface, version, manufacturer, serial_number);

  return std::shared_ptr<RobotRuntime>(
    new RobotRuntime(Adapter::make(protocol)));
}

RobotRuntime::RobotRuntime(std::shared_ptr<Adapter> adapter)
: adapter_(std::move(adapter))
{
}

void RobotRuntime::on_navigate(NavigateCallback callback)
{
  adapter_->on_navigate(std::move(callback));
}

void RobotRuntime::on_base(BaseCallback callback)
{
  adapter_->on_base(std::move(callback));
}

std::shared_ptr<Reporter> RobotRuntime::reporter()
{
  return adapter_->reporter();
}

void RobotRuntime::start()
{
  adapter_->start();
}

void RobotRuntime::stop()
{
  adapter_->stop();
}

}  // namespace adapter
}  // namespace vda5050_core
