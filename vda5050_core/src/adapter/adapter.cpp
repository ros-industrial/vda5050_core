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

#include "vda5050_core/logger/logger.hpp"

#include "vda5050_core/adapter/adapter.hpp"

#include "vda5050_core/types/connection.hpp"
#include "vda5050_core/types/instant_actions.hpp"
#include "vda5050_core/types/order.hpp"

#include "adapter_impl.hpp"

namespace vda5050_core {

namespace adapter {

//=============================================================================
void Adapter::Implementation::subscribe_orders()
{
  protocol_adapter->subscribe<types::Order>(
    [this](types::Order order, auto error) {
      if (error.has_value())
      {
      }
    },
    0);
}

//=============================================================================
void Adapter::Implementation::subscribe_instant_actions() {}

//=============================================================================
void Adapter::Implementation::start_dispatch_thread() {}

//=============================================================================
void Adapter::Implementation::start_state_thread() {}

//=============================================================================
void Adapter::Implementation::set_connection_will()
{
  types::Connection connection_will;
  connection_will.connection_state = types::ConnectionState::CONNECTIONBROKEN;
  protocol_adapter->set_will<types::Connection>(connection_will, 1, true);
}

//=============================================================================
void Adapter::Implementation::publish_connection_online()
{
  types::Connection connection_online;
  connection_online.connection_state = types::ConnectionState::ONLINE;
  protocol_adapter->publish<types::Connection>(connection_online, 1, true);
}

//=============================================================================
void Adapter::Implementation::publish_connection_offline()
{
  types::Connection connection_offline;
  connection_offline.connection_state = types::ConnectionState::OFFLINE;
  protocol_adapter->publish<types::Connection>(connection_offline, 1, true);
}

//=============================================================================
void Adapter::Implementation::process_navigation() {}

//=============================================================================
void Adapter::Implementation::publish_factsheet() {}

//=============================================================================
void Adapter::Implementation::publish_state() {}

//=============================================================================
Adapter::~Adapter()
{
  stop();
}

//=============================================================================
std::shared_ptr<Adapter> Adapter::make(
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter)
{
  auto adapter =
    std::shared_ptr<Adapter>(new Adapter(std::move(protocol_adapter)));
  return adapter;
}

//=============================================================================
void Adapter::on_navigate(
  std::function<void(NavigationRequest, std::shared_ptr<Execution>)> callback)
{
  pimpl_->navigation_callback = std::move(callback);
}

//=============================================================================
void Adapter::on_action(
  std::function<void(ActionRequest, std::shared_ptr<Execution>)> callback)
{
  pimpl_->action_callback = std::move(callback);
}

//=============================================================================
std::shared_ptr<StateManager> Adapter::state_manager()
{
  return pimpl_->state_manager;
}

//=============================================================================
void Adapter::start()
{
  if (pimpl_->running) return;

  pimpl_->set_connection_will();
  pimpl_->protocol_adapter->connect();

  if (!pimpl_->protocol_adapter->connected())
  {
    VDA5050_WARN(
      "Adapter started but MQTT broker is not connected. "
      "Messages will be dropped until a connection is established");
  }

  pimpl_->subscribe_orders();

  pimpl_->subscribe_instant_actions();

  pimpl_->publish_connection_online();

  pimpl_->start_dispatch_thread();

  pimpl_->start_state_thread();

  pimpl_->running = true;
}

//=============================================================================
void Adapter::stop()
{
  if (!pimpl_->running) return;

  pimpl_->running = false;

  if (pimpl_->protocol_adapter)
  {
    pimpl_->protocol_adapter->unsubscribe<types::Order>();
    pimpl_->protocol_adapter->unsubscribe<types::InstantActions>();

    pimpl_->publish_connection_offline();

    pimpl_->protocol_adapter->disconnect();
  }
}

//=============================================================================
Adapter::Adapter(std::shared_ptr<execution::ProtocolAdapter> protocol_adapter)
: pimpl_(std::make_unique<Implementation>())
{
  pimpl_->protocol_adapter = std::move(protocol_adapter);

  pimpl_->state_manager = StateManager::make();
}

}  // namespace adapter
}  // namespace vda5050_core
