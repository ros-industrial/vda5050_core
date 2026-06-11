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

#include "vda5050_core/adapter/adapter.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "vda5050_core/logger/logger.hpp"

#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/client/events/navigate_to_node.hpp"
#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/strategies/order_acceptance.hpp"
#include "vda5050_core/client/strategies/order_actions.hpp"
#include "vda5050_core/client/strategies/order_traversal.hpp"
#include "vda5050_core/client/strategies/state_reporting.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/execution/handler.hpp"
#include "vda5050_core/types/connection.hpp"
#include "vda5050_core/types/error.hpp"
#include "vda5050_core/types/node.hpp"
#include "vda5050_core/types/node_state.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"

#include "adapter_internal.hpp"

namespace vda5050_core {

namespace adapter {

class Adapter::Implementation
{
public:
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter;
  std::shared_ptr<RobotIoState> robot_io;

  std::shared_ptr<client::AGVContext> context;
  std::shared_ptr<client::OrderExecutionResource> execution;

  std::shared_ptr<client::OrderAcceptance> acceptance;
  std::shared_ptr<client::OrderTraversal> traversal;
  std::shared_ptr<client::OrderActions> actions;
  std::shared_ptr<client::StateReporting> reporting;

  std::shared_ptr<execution::Handler> handler;
  std::shared_ptr<NavigationManager> navigation_manager;

  std::function<void(types::Node)> navigate_callback;
  std::mutex callback_mutex;

  // Serialises State publishes from the reporter (spin thread) and the
  // 1 Hz heartbeat thread.
  std::mutex publish_mutex;

  std::thread spin_thread;
  std::thread heartbeat_thread;
  std::atomic_bool started{false};

  bool heartbeat_running = false;
  std::mutex heartbeat_mutex;
  std::condition_variable heartbeat_cv;

  // Overlay robot-owned fields onto the strategy-produced State, then publish.
  void publish_state(const types::State& order_state)
  {
    if (!protocol_adapter || !robot_io) return;

    types::State merged = merge_robot_io(order_state, *robot_io);

    std::lock_guard<std::mutex> lock(publish_mutex);
    protocol_adapter->publish<types::State>(merged, 0);
  }

  // NavigateToNodeEvent carries a NodeState; resolve the full order Node
  // (actions + position) from the persisted active order, falling back to a
  // minimal Node built from the NodeState when no match is found.
  types::Node resolve_node(const types::NodeState& target)
  {
    if (execution)
    {
      const types::Order order = execution->get_active_order();
      for (const auto& node : order.nodes)
      {
        if (
          node.node_id == target.node_id &&
          node.sequence_id == target.sequence_id)
        {
          return node;
        }
      }
    }

    types::Node node;
    node.node_id = target.node_id;
    node.sequence_id = target.sequence_id;
    node.released = target.released;
    node.node_position = target.node_position;
    node.node_description = target.node_description;
    return node;
  }

  void run_heartbeat()
  {
    while (true)
    {
      std::unique_lock<std::mutex> lock(heartbeat_mutex);
      heartbeat_cv.wait_for(
        lock, std::chrono::seconds(1), [this] { return !heartbeat_running; });
      if (!heartbeat_running) break;
      lock.unlock();

      if (execution) publish_state(execution->get_state());
    }
  }
};

Adapter::Adapter(
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter,
  std::shared_ptr<NavigationManager> navigation_manager)
: pimpl_(std::make_unique<Implementation>())
{
  pimpl_->protocol_adapter = std::move(protocol_adapter);
  pimpl_->robot_io = std::make_shared<RobotIoState>();

  // No strategy reads HeaderConfigResource and the ProtocolAdapter stamps real
  // message headers on publish, so a placeholder identity is sufficient here.
  auto config = std::make_shared<client::HeaderConfigResource>(
    "uagv", "2.0.0", "Manufacturer", "S001");
  pimpl_->context = client::AGVContext::make(config);
  pimpl_->execution =
    pimpl_->context->get_resource<client::OrderExecutionResource>();

  pimpl_->acceptance = std::make_shared<client::OrderAcceptance>();
  pimpl_->traversal = std::make_shared<client::OrderTraversal>();
  // OrderActions consumes traversal's engine events (node reached / edge
  // entered / left), so it is constructed against the traversal engine.
  pimpl_->actions = client::OrderActions::make(pimpl_->traversal->engine());
  pimpl_->reporting = std::make_shared<client::StateReporting>();

  Implementation* self = pimpl_.get();

  // State out: StateReporting reports the assembled State on change.
  pimpl_->reporting->set_reporter(
    [self](const types::State& state) { self->publish_state(state); });

  // Navigation out: consume NavigateToNodeEvent from the traversal engine,
  // resolve the full Node, and dispatch it to the registered callback.
  pimpl_->traversal->engine()->on<client::NavigateToNodeEvent>(
    [self](std::shared_ptr<client::NavigateToNodeEvent> event) {
      {
        std::lock_guard<std::mutex> lock(self->robot_io->mutex);
        self->robot_io->driving = true;
      }

      types::Node node = self->resolve_node(event->target);

      std::function<void(types::Node)> callback;
      {
        std::lock_guard<std::mutex> lock(self->callback_mutex);
        callback = self->navigate_callback;
      }
      if (callback) callback(std::move(node));
    });

  // Handler::make() calls context->init() and strategy->init() for each.
  pimpl_->handler = execution::Handler::make(
    pimpl_->context, {pimpl_->acceptance, pimpl_->traversal, pimpl_->actions,
                      pimpl_->reporting});

  pimpl_->navigation_manager = std::move(navigation_manager);
  pimpl_->navigation_manager->bind_internals(
    pimpl_->robot_io, pimpl_->context->provider());
}

Adapter::~Adapter()
{
  stop();
}

std::shared_ptr<Adapter> Adapter::make(
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter)
{
  auto navigation_manager =
    std::shared_ptr<NavigationManager>(new NavigationManager());
  return std::shared_ptr<Adapter>(
    new Adapter(std::move(protocol_adapter), std::move(navigation_manager)));
}

void Adapter::on_navigate(std::function<void(types::Node)> callback)
{
  std::lock_guard<std::mutex> lock(pimpl_->callback_mutex);
  pimpl_->navigate_callback = std::move(callback);
}

std::shared_ptr<NavigationManager> Adapter::navigation_manager()
{
  return pimpl_->navigation_manager;
}

void Adapter::start()
{
  if (pimpl_->started.exchange(true)) return;

  pimpl_->protocol_adapter->connect();
  if (!pimpl_->protocol_adapter->connected())
  {
    VDA5050_WARN(
      "Adapter started but MQTT broker is not connected — messages will be "
      "dropped until a connection is established");
  }

  std::weak_ptr<execution::ContextInterface> weak_context = pimpl_->context;
  pimpl_->protocol_adapter->subscribe<types::Order>(
    [weak_context](types::Order order, std::optional<types::Error> error) {
      if (error.has_value()) return;
      if (auto context = weak_context.lock())
      {
        VDA5050_INFO(
          "Adapter received order with order_id: {}", order.order_id);
        context->provider()->push<client::OrderUpdate>(std::move(order));
      }
    },
    0);

  types::Connection online;
  online.connection_state = types::ConnectionState::ONLINE;
  pimpl_->protocol_adapter->publish<types::Connection>(online, 1, true);

  {
    std::lock_guard<std::mutex> lock(pimpl_->heartbeat_mutex);
    pimpl_->heartbeat_running = true;
  }
  pimpl_->spin_thread =
    std::thread([handler = pimpl_->handler] { handler->spin(); });
  pimpl_->heartbeat_thread =
    std::thread([self = pimpl_.get()] { self->run_heartbeat(); });
}

void Adapter::stop()
{
  if (!pimpl_->started.exchange(false)) return;

  if (pimpl_->protocol_adapter)
  {
    pimpl_->protocol_adapter->unsubscribe<types::Order>();
  }

  {
    std::lock_guard<std::mutex> lock(pimpl_->heartbeat_mutex);
    pimpl_->heartbeat_running = false;
  }
  pimpl_->heartbeat_cv.notify_all();

  if (pimpl_->handler) pimpl_->handler->stop();
  if (pimpl_->spin_thread.joinable()) pimpl_->spin_thread.join();
  if (pimpl_->heartbeat_thread.joinable()) pimpl_->heartbeat_thread.join();

  if (pimpl_->protocol_adapter)
  {
    if (pimpl_->protocol_adapter->connected())
    {
      types::Connection offline;
      offline.connection_state = types::ConnectionState::OFFLINE;
      pimpl_->protocol_adapter->publish<types::Connection>(offline, 1, true);
    }
    pimpl_->protocol_adapter->disconnect();
  }
}

}  // namespace adapter
}  // namespace vda5050_core
