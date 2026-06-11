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

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "vda5050_core/logger/logger.hpp"

#include "vda5050_core/client/events/navigate_to_node.hpp"
#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/strategies/order_acceptance.hpp"
#include "vda5050_core/client/strategies/order_actions.hpp"
#include "vda5050_core/client/strategies/order_traversal.hpp"
#include "vda5050_core/client/strategies/state_reporting.hpp"
#include "vda5050_core/client/updates/node_reached.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/execution/strategy_interface.hpp"
#include "vda5050_core/types/connection.hpp"
#include "vda5050_core/types/edge_state.hpp"
#include "vda5050_core/types/error.hpp"

#include "adapter_internal.hpp"

namespace vda5050_core {

namespace adapter {

namespace {

// Reports the order's first node as reached the moment a new order is accepted,
// and announces the whole base. OrderTraversal dispatches only SUCCESSOR nodes
// and advances on node-reached signals; it treats node[0] as the AGV's current
// position. VDA5050 requires the first node of a new order to be where the AGV
// already is, so it is "reached" on acceptance. Without this kick, a freshly
// accepted order would sit idle. Runs after OrderAcceptance in the pipeline.
class AutoStartKick : public execution::StrategyInterface
{
public:
  // Invoked once per newly accepted order with the full active order (the base).
  void set_on_new_order(std::function<void(const types::Order&)> callback)
  {
    on_new_order_ = std::move(callback);
  }

  void init(std::shared_ptr<execution::ContextInterface> /*context*/) override
  {
  }

  void step(std::shared_ptr<execution::ContextInterface> context) override
  {
    auto execution = context->get_resource<client::OrderExecutionResource>();
    if (!execution || !execution->is_executing_order()) return;

    const types::Order order = execution->get_active_order();
    if (order.nodes.empty() || order.order_id == last_kicked_order_id_) return;

    // Only a brand-new order starts the AGV at node[0]; order updates extend the
    // horizon mid-route, so they must not re-report the start node.
    last_kicked_order_id_ = order.order_id;

    if (on_new_order_) on_new_order_(order);

    const types::Node& start = order.nodes.front();
    context->provider()->push<client::NodeReachedUpdate>(
      start.node_id, start.sequence_id);
  }

private:
  std::function<void(const types::Order&)> on_new_order_;
  std::string last_kicked_order_id_;
};

}  // namespace

Adapter::Adapter(std::shared_ptr<execution::ProtocolAdapter> protocol_adapter)
: protocol_adapter_(std::move(protocol_adapter)),
  robot_io_(std::make_shared<RobotIoState>())
{
  // No strategy reads HeaderConfigResource and the ProtocolAdapter stamps real
  // message headers on publish, so a placeholder identity is sufficient here.
  auto config = std::make_shared<client::HeaderConfigResource>(
    "uagv", "2.0.0", "Manufacturer", "S001");
  context_ = client::AGVContext::make(config);
  execution_ = context_->get_resource<client::OrderExecutionResource>();

  // Strategies are owned by the Handler once registered; only `traversal` is
  // needed here (for its engine), so the rest stay local to the constructor.
  auto acceptance = std::make_shared<client::OrderAcceptance>();
  auto auto_start_kick = std::make_shared<AutoStartKick>();
  auto traversal = std::make_shared<client::OrderTraversal>();
  auto actions = client::OrderActions::make(traversal->engine());
  auto reporting = std::make_shared<client::StateReporting>();

  // Whole-base callback fires from the start-kick, which already detects a
  // freshly accepted order.
  auto_start_kick->set_on_new_order([this](const types::Order& order) {
    BaseCallback callback;
    {
      std::lock_guard<std::mutex> lock(callback_mutex_);
      callback = base_callback_;
    }
    if (callback) callback(order);
  });

  // State out: StateReporting reports the assembled State on change.
  reporting->set_reporter(
    [this](const types::State& state) { publish_state(state); });

  // Navigation out: consume NavigateToNodeEvent from the traversal engine,
  // resolve the full Node + entering Edge, and dispatch to the callback.
  traversal->engine()->on<client::NavigateToNodeEvent>(
    [this](std::shared_ptr<client::NavigateToNodeEvent> event) {
      // `driving` is the robot's actual motion state, owned and reported by the
      // robot via Reporter::set_driving — the adapter does not fabricate it.
      types::Node node = resolve_node(event->target);
      std::optional<types::Edge> edge = resolve_edge(event->via_edge);

      NavigateCallback callback;
      {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback = navigate_callback_;
      }
      if (callback) callback(std::move(node), std::move(edge));
    });

  // Order matters: auto_start_kick runs after acceptance has applied the order
  // and before traversal, so the start-node reached signal it pushes is
  // consumed in the same handler pass.
  handler_ = execution::Handler::make(
    context_, {acceptance, auto_start_kick, traversal, actions, reporting});

  reporter_ = std::shared_ptr<Reporter>(new Reporter());
  reporter_->bind_internals(robot_io_, context_->provider());
}

Adapter::~Adapter()
{
  stop();
}

std::shared_ptr<Adapter> Adapter::make(
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter)
{
  return std::shared_ptr<Adapter>(new Adapter(std::move(protocol_adapter)));
}

void Adapter::on_navigate(NavigateCallback callback)
{
  std::lock_guard<std::mutex> lock(callback_mutex_);
  navigate_callback_ = std::move(callback);
}

void Adapter::on_base(BaseCallback callback)
{
  std::lock_guard<std::mutex> lock(callback_mutex_);
  base_callback_ = std::move(callback);
}

std::shared_ptr<Reporter> Adapter::reporter()
{
  return reporter_;
}

void Adapter::publish_state(const types::State& order_state)
{
  if (!protocol_adapter_ || !robot_io_) return;

  types::State merged = merge_robot_io(order_state, *robot_io_);

  std::lock_guard<std::mutex> lock(publish_mutex_);
  protocol_adapter_->publish<types::State>(merged, 0);
}

types::Node Adapter::resolve_node(const types::NodeState& target)
{
  if (execution_)
  {
    const types::Order order = execution_->get_active_order();
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

std::optional<types::Edge> Adapter::resolve_edge(
  const std::optional<types::EdgeState>& target)
{
  if (!target) return std::nullopt;

  if (execution_)
  {
    const types::Order order = execution_->get_active_order();
    for (const auto& edge : order.edges)
    {
      if (
        edge.edge_id == target->edge_id &&
        edge.sequence_id == target->sequence_id)
      {
        return edge;
      }
    }
  }

  // Fall back to a minimal Edge carrying the identity from the EdgeState.
  types::Edge edge;
  edge.edge_id = target->edge_id;
  edge.sequence_id = target->sequence_id;
  edge.released = target->released;
  return edge;
}

void Adapter::run_heartbeat()
{
  while (true)
  {
    std::unique_lock<std::mutex> lock(heartbeat_mutex_);
    heartbeat_cv_.wait_for(
      lock, std::chrono::seconds(1), [this] { return !heartbeat_running_; });
    if (!heartbeat_running_) break;
    lock.unlock();

    if (execution_) publish_state(execution_->get_state());
  }
}

void Adapter::start()
{
  if (started_.exchange(true)) return;

  protocol_adapter_->connect();
  if (!protocol_adapter_->connected())
  {
    VDA5050_WARN(
      "Adapter started but MQTT broker is not connected — messages will be "
      "dropped until a connection is established");
  }

  std::weak_ptr<execution::ContextInterface> weak_context = context_;
  protocol_adapter_->subscribe<types::Order>(
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
  protocol_adapter_->publish<types::Connection>(online, 1, true);

  {
    std::lock_guard<std::mutex> lock(heartbeat_mutex_);
    heartbeat_running_ = true;
  }
  spin_thread_ = std::thread([handler = handler_] { handler->spin(); });
  heartbeat_thread_ = std::thread([this] { run_heartbeat(); });
}

void Adapter::stop()
{
  if (!started_.exchange(false)) return;

  if (protocol_adapter_) protocol_adapter_->unsubscribe<types::Order>();

  {
    std::lock_guard<std::mutex> lock(heartbeat_mutex_);
    heartbeat_running_ = false;
  }
  heartbeat_cv_.notify_all();

  if (handler_) handler_->stop();
  if (spin_thread_.joinable()) spin_thread_.join();
  if (heartbeat_thread_.joinable()) heartbeat_thread_.join();

  if (protocol_adapter_)
  {
    if (protocol_adapter_->connected())
    {
      types::Connection offline;
      offline.connection_state = types::ConnectionState::OFFLINE;
      protocol_adapter_->publish<types::Connection>(offline, 1, true);
    }
    protocol_adapter_->disconnect();
  }
}

}  // namespace adapter
}  // namespace vda5050_core
