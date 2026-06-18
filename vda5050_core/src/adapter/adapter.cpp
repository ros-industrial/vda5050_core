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

#include <thread>
#include <utility>

#include "vda5050_core/errors/error_codes.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/order_utils/order_graph_validator.hpp"

#include "vda5050_core/types/connection.hpp"
#include "vda5050_core/types/instant_actions.hpp"
#include "vda5050_core/types/order.hpp"

#include "adapter_impl.hpp"
#include "vda5050_core/adapter/adapter.hpp"

namespace vda5050_core {

namespace adapter {

//=============================================================================
void Adapter::Implementation::subscribe_orders()
{
  protocol_adapter->subscribe<types::Order>(
    [this](types::Order order, auto error) {
      if (error.has_value())
      {
        state_manager->add_error(error.value());
        request_state_publish();
        return;
      }

      auto active = active_order.lock();

      if (!active->order.has_value())
      {
        auto result = order_utils::is_valid_graph(order);

        if (!result)
        {
          for (const auto& e : result.errors)
          {
            state_manager->add_error(e);
          }

          request_state_publish();
          return;
        }

        ActiveOrder incoming;
        incoming.order = std::move(order);

        for (std::size_t i = 0; i < order.nodes.size(); ++i)
        {
          incoming.node_lookup.emplace(order.nodes[i].sequence_id, i);
        }

        for (std::size_t i = 0; i < order.edges.size(); ++i)
        {
          incoming.edge_lookup.emplace(order.edges[i].sequence_id, i);
        }

        active->order = std::move(incoming);

        state_manager->set_order(active->order->order);
        request_state_publish();

        VDA5050_INFO("Accepted new order [{}]", active->order->order.order_id);

        return;
      }

      auto result = order_utils::is_valid_update(active->order->order, order);

      if (!result)
      {
        for (const auto& e : result.errors)
        {
          state_manager->add_error(e);
        }

        request_state_publish();
        return;
      }

      active->order->order = std::move(order);

      state_manager->set_order(active->order->order);
      request_state_publish();

      VDA5050_INFO(
        "Accepted order update for ID [{}]", active->order->order.order_id);
    },
    0);
}

//=============================================================================
void Adapter::Implementation::subscribe_instant_actions()
{
  protocol_adapter->subscribe<types::InstantActions>(
    [this](types::InstantActions actions, auto error) {
      if (error.has_value())
      {
        state_manager->add_error(error.value());
        return;
      }

      auto queue = instant_actions.lock();

      for (auto& action : actions.actions)
      {
        queue->push_back(std::move(action));
      }

      VDA5050_INFO("Received {} instant actions", actions.actions.size());
    },
    0);
}

//=============================================================================
void Adapter::Implementation::start_dispatch_thread()
{
  dispatch_thread = std::thread([this]() {
    while (running)
    {
      process_navigation();

      process_actions();
    }
  });
}

//=============================================================================
void Adapter::Implementation::start_state_thread()
{
  state_thread = std::thread([this]() {
    while (running)
    {
      std::unique_lock<std::mutex> lock(state_mutex);
      state_cv.wait_for(lock, std::chrono::seconds(30), [this]() {
        return !running || state_manager->consume_publish_requested();
      });

      if (!running) break;

      lock.unlock();

      publish_state();
    }
  });
}

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
void Adapter::Implementation::process_navigation()
{
  if (!navigation_callback) return;

  auto active = active_order.lock();

  if (!active->order.has_value()) return;

  auto& order_state = *active->order;
  if (order_state.executing) return;

  const auto& order = order_state.order;

  uint32_t next_node_seq;

  if (order_state.last_completed_node_sequence_id == 0)
  {
    next_node_seq = 0;
  }
  else
  {
    next_node_seq = order_state.last_completed_node_sequence_id;
  }

  auto node_it = order_state.node_lookup.find(next_node_seq);
  if (node_it == order_state.node_lookup.end()) return;

  const auto& next_node = order.nodes[node_it->second];

  uint32_t next_edge_seq = next_node_seq - 1;

  auto edge_it = order_state.edge_lookup.find(next_edge_seq);

  NavigationRequest request;
  request.destination = next_node;

  if (edge_it != order_state.edge_lookup.end())
  {
    request.approach_edge = order.edges[edge_it->second];
  }

  order_state.executing = true;

  VDA5050_INFO(
    "Dispatching node ID [{}] with sequence [{}]", next_node.node_id,
    next_node_seq);

  navigation_callback(
    std::move(request),
    Execution::make(
      [this, seq = next_node_seq]() {
        auto active = active_order.lock();
        if (!active->order.has_value()) return;

        auto& order_state = *active->order;

        order_state.executing = false;
        order_state.last_completed_node_sequence_id = seq;

        auto it = order_state.node_lookup.find(seq);
        if (it != order_state.node_lookup.end())
        {
          state_manager->node_reached(order_state.order.nodes[it->second]);
        }
        request_state_publish();
      },
      [this](const std::string& reason) {
        types::Error e;
        e.error_type = errors::NavigationFailedError;
        e.error_description = reason;

        state_manager->add_error(e);

        auto active = active_order.lock();
        if (active->order.has_value())
        {
          active->order->executing = false;
        }

        request_state_publish();
      }));
}

//=============================================================================
void Adapter::Implementation::publish_factsheet() {}

//=============================================================================
void Adapter::Implementation::publish_state()
{
  protocol_adapter->publish<types::State>(state_manager->state(), 0);
}

//=============================================================================
void Adapter::Implementation::request_state_publish()
{
  state_manager->mark_publish_requested();
  state_cv.notify_all();
}

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
