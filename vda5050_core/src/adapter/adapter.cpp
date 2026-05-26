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

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "vda5050_core/adapter/adapter.hpp"
#include "vda5050_core/logger/logger.hpp"

#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/handler.hpp"
#include "vda5050_core/execution/strategy_interface.hpp"
#include "vda5050_core/types/connection.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"

#include "adapter_internal.hpp"

namespace vda5050_core {

namespace adapter {

namespace {

void apply_order_to_agv_state(const types::Order& order, AgvState& agv_state)
{
  auto& s = agv_state.state;

  s.order_id = order.order_id;
  s.order_update_id = order.order_update_id;
  s.zone_set_id = order.zone_set_id;
  s.node_states.clear();
  s.edge_states.clear();

  for (const auto& node : order.nodes)
  {
    types::NodeState node_state;
    node_state.node_id = node.node_id;
    node_state.sequence_id = node.sequence_id;
    node_state.released = node.released;
    node_state.node_position = node.node_position;
    node_state.node_description = node.node_description;
    agv_state.state.node_states.push_back(std::move(node_state));
  }

  for (const auto& edge : order.edges)
  {
    types::EdgeState edge_state;
    edge_state.edge_id = edge.edge_id;
    edge_state.sequence_id = edge.sequence_id;
    edge_state.released = edge.released;
    edge_state.edge_description = edge.edge_description;
    edge_state.trajectory = edge.trajectory;
    agv_state.state.edge_states.push_back(std::move(edge_state));
  }
}

types::State build_state_from_agv_state(const AgvState& agv_state)
{
  return agv_state.state;
}

class AgvContext : public execution::ContextInterface,
                   public std::enable_shared_from_this<AgvContext>
{
public:
  explicit AgvContext(std::shared_ptr<AgvState> agv_state)
  : agv_state_(std::move(agv_state))
  {
    // Nothing to do here ...
  }

  void init() override
  {
    provider()->on<OrderUpdate>([w = weak_from_this()](auto update) {
      if (auto context = w.lock())
      {
        {
          std::lock_guard<std::mutex> lock(context->mutex_);
          context->updates_[update->get_type()] = update;
        }

        if (context->agv_state_)
        {
          std::lock_guard<std::mutex> state_lock(context->agv_state_->mutex);
          apply_order_to_agv_state(update->order, *context->agv_state_);
        }

        context->notify_on_change();
      }
    });
  }

protected:
  std::shared_ptr<execution::UpdateBase> get_update_raw(
    std::type_index type) const override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = updates_.find(type);
    if (it != updates_.end()) return it->second;
    return nullptr;
  }

  std::shared_ptr<execution::ResourceBase> get_resource_raw(
    std::type_index /*type*/) const override
  {
    return nullptr;
  }

private:
  std::shared_ptr<AgvState> agv_state_;
  std::unordered_map<std::type_index, std::shared_ptr<execution::UpdateBase>>
    updates_;
  mutable std::mutex mutex_;
};

class NavigationStrategy : public execution::StrategyInterface
{
public:
  void set_navigate_callback(std::function<void(types::Node)> callback)
  {
    navigate_callback_ = std::move(callback);
  }

  void init(std::shared_ptr<execution::ContextInterface> context) override
  {
    engine()->on<NodeDispatchEvent>([this](auto event) {
      if (agv_state_)
      {
        std::lock_guard<std::mutex> lock(agv_state_->mutex);
        agv_state_->state.driving = true;
      }

      if (navigate_callback_)
      {
        navigate_callback_(event->node);
      }
    });

    context->provider()->on<NodeAckUpdate>(
      [w = std::weak_ptr(engine())](auto update) {
        if (auto engine = w.lock()) engine->notify(update);
      });

    context->provider()->on<OrderUpdate>([this](auto update) {
      std::lock_guard<std::mutex> lock(mutex_);
      nodes_ = update->order.nodes;
      current_idx_ = 0;
      order_loaded_ = !nodes_.empty();
    });
  }

  void step(std::shared_ptr<execution::ContextInterface> /*context*/) override
  {
    if (engine()->waiting()) return;

    types::Node target;
    {
      // Single lock section: eliminates the TOCTOU window that existed when the
      // bounds check and the actual nodes_[current_idx_] read were in separate
      // critical sections, leaving a gap where an OrderUpdate could reset the
      // vector between the two acquisitions.
      std::lock_guard<std::mutex> lock(mutex_);
      if (!order_loaded_ || current_idx_ >= nodes_.size()) return;
      target = nodes_[current_idx_++];
      if (current_idx_ >= nodes_.size()) order_loaded_ = false;
    }

    const auto sequence_id = target.sequence_id;
    engine()->emit<NodeDispatchEvent>(
      execution::Priority::NORMAL, std::move(target));
    engine()->step();
    engine()->suspend_for<NodeAckUpdate>(
      std::chrono::seconds(30),
      [sequence_id](auto update) -> bool {
        return update->sequence_id == sequence_id;
      });
  }

  void set_agv_state(std::shared_ptr<AgvState> agv_state)
  {
    agv_state_ = std::move(agv_state);
  }

private:
  std::function<void(types::Node)> navigate_callback_;
  std::shared_ptr<AgvState> agv_state_;
  std::vector<types::Node> nodes_;
  size_t current_idx_ = 0;
  bool order_loaded_ = false;
  std::mutex mutex_;
};

class StateStrategy : public execution::StrategyInterface
{
public:
  StateStrategy(
    std::shared_ptr<execution::ProtocolAdapter> protocol_adapter,
    std::shared_ptr<AgvState> agv_state)
  : protocol_adapter_(std::move(protocol_adapter)),
    agv_state_(std::move(agv_state)),
    last_pub_time_(std::chrono::steady_clock::now())
  {
    // Nothing to do here ...
  }

  void init(std::shared_ptr<execution::ContextInterface> context) override
  {
    auto publish = [this]() { publish_state(); };

    context->provider()->on<OrderUpdate>(
      [publish](auto /*update*/) { publish(); });

    context->provider()->on<NodeAckUpdate>(
      [publish](auto /*update*/) { publish(); });
  }

  void step(std::shared_ptr<execution::ContextInterface> /*context*/) override
  {
    const auto now = std::chrono::steady_clock::now();
    if (now - last_pub_time_ >= std::chrono::seconds(1))
    {
      publish_state();
    }
  }

private:
  void publish_state()
  {
    if (!protocol_adapter_ || !agv_state_) return;

    types::State state;
    {
      std::lock_guard<std::mutex> lock(agv_state_->mutex);
      state = build_state_from_agv_state(*agv_state_);
    }

    protocol_adapter_->publish<types::State>(state, 0);
    last_pub_time_ = std::chrono::steady_clock::now();
  }

  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter_;
  std::shared_ptr<AgvState> agv_state_;
  std::chrono::steady_clock::time_point last_pub_time_;
};

}  // namespace

class Adapter::Implementation
{
public:
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter;
  std::shared_ptr<AgvState> agv_state;
  std::shared_ptr<AgvContext> context;
  std::shared_ptr<NavigationStrategy> navigation_strategy;
  std::shared_ptr<StateStrategy> state_strategy;
  std::shared_ptr<execution::Handler> handler;
  std::shared_ptr<NavigationManager> navigation_manager;
  std::thread spin_thread;
  std::atomic_bool started{false};
  // Reserved for on_cancel() — see adapter.hpp for rationale.
  // std::function<void()> cancel_callback;
};

Adapter::Adapter(
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter,
  std::shared_ptr<NavigationManager> navigation_manager)
: pimpl_(std::make_unique<Implementation>())
{
  pimpl_->protocol_adapter = std::move(protocol_adapter);
  pimpl_->agv_state = std::make_shared<AgvState>();
  pimpl_->context = std::make_shared<AgvContext>(pimpl_->agv_state);
  pimpl_->navigation_strategy = std::make_shared<NavigationStrategy>();
  pimpl_->navigation_strategy->set_agv_state(pimpl_->agv_state);
  pimpl_->state_strategy = std::make_shared<StateStrategy>(
    pimpl_->protocol_adapter, pimpl_->agv_state);
  pimpl_->handler = execution::Handler::make(
    pimpl_->context,
    {pimpl_->navigation_strategy, pimpl_->state_strategy});
  pimpl_->navigation_manager = std::move(navigation_manager);
  pimpl_->navigation_manager->bind_internals(
    pimpl_->agv_state, pimpl_->context->provider(), pimpl_->context);
}

Adapter::~Adapter()
{
  stop();
}

std::shared_ptr<Adapter> Adapter::make(
  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter)
{
  auto navigation_manager = std::shared_ptr<NavigationManager>(
    new NavigationManager());
  return std::shared_ptr<Adapter>(
    new Adapter(std::move(protocol_adapter), navigation_manager));
}

void Adapter::on_navigate(std::function<void(types::Node)> callback)
{
  pimpl_->navigation_strategy->set_navigate_callback(std::move(callback));
}

// on_cancel() is reserved for future VDA5050 §6.10.2 cancel instant action
// support. The callback storage and dispatch logic are not yet wired — enable
// once InstantAction topic subscription is implemented.
// void Adapter::on_cancel(std::function<void()> callback)
// {
//   pimpl_->cancel_callback = std::move(callback);
// }

std::shared_ptr<NavigationManager> Adapter::navigation_manager()
{
  return pimpl_->navigation_manager;
}

void Adapter::start()
{
  if (pimpl_->started.exchange(true)) return;

  pimpl_->protocol_adapter->connect();

  // connect() swallows exceptions internally, so verify the result explicitly.
  // Subscribing and publishing below will silently no-op until the broker is
  // reachable, so surface the problem early rather than failing silently.
  if (!pimpl_->protocol_adapter->connected())
  {
    VDA5050_WARN("Adapter started but MQTT broker is not connected — "
                 "messages will be dropped until a connection is established");
  }

  pimpl_->protocol_adapter->subscribe<types::Order>(
    [w = std::weak_ptr<AgvContext>(pimpl_->context)](
      auto order, auto error) {
      if (error.has_value()) return;

      if (auto context = w.lock())
      {
        VDA5050_INFO("Received order with order_id: {}", order.order_id);
        context->provider()->push<OrderUpdate>(std::move(order));
      }
    },
    0);

  types::Connection connection_online;
  connection_online.connection_state = types::ConnectionState::ONLINE;
  pimpl_->protocol_adapter->publish<types::Connection>(connection_online, 1, true);

  pimpl_->spin_thread = std::thread([handler = pimpl_->handler]() {
    handler->spin();
  });
}

void Adapter::stop()
{
  if (!pimpl_->started.exchange(false)) return;

  if (pimpl_->handler) pimpl_->handler->stop();

  if (pimpl_->spin_thread.joinable()) pimpl_->spin_thread.join();

  if (pimpl_->protocol_adapter)
  {
    types::Connection connection_offline;
    connection_offline.connection_state = types::ConnectionState::OFFLINE;
    pimpl_->protocol_adapter->publish<types::Connection>(
      connection_offline, 1, true);
    pimpl_->protocol_adapter->disconnect();
  }
}

}  // namespace adapter
}  // namespace vda5050_core
