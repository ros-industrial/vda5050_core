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

#ifndef VDA5050_CORE__ADAPTER__ADAPTER_HPP_
#define VDA5050_CORE__ADAPTER__ADAPTER_HPP_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include "vda5050_core/adapter/reporter.hpp"
#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/execution/handler.hpp"
#include "vda5050_core/execution/protocol_adapter.hpp"
#include "vda5050_core/types/edge.hpp"
#include "vda5050_core/types/node.hpp"
#include "vda5050_core/types/node_state.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"

namespace vda5050_core {

namespace adapter {

// Robot-owned State fields, defined in the implementation TU.
struct RobotIoState;

/// \brief Per-node navigation callback: drive to `node`, traversing `edge`.
///
/// `edge` carries the edge being entered to reach `node` (trajectory and
/// edge-specific motion properties); it is absent for the first dispatched node.
using NavigateCallback =
  std::function<void(types::Node, std::optional<types::Edge>)>;

/// \brief Whole-base callback: the full released base of a newly accepted order.
///
/// For robots that plan an entire route at once rather than node-by-node. Fires
/// once per accepted order, alongside (not instead of) the per-node callback.
using BaseCallback = std::function<void(types::Order)>;

/// \brief AGV-side integration facade over the client Context+Strategy pipeline.
///
/// Wires a `ProtocolAdapter` to `AGVContext` + the order strategies: subscribes
/// to Order, dispatches navigation to the registered callback(s), publishes
/// State (on change plus a 1 Hz heartbeat), and manages the MQTT connection.
///
/// Members are held directly (no PIMPL): this API is experimental and expected
/// to evolve with the execution layer, so ABI stability is not a concern.
class Adapter : public std::enable_shared_from_this<Adapter>
{
public:
  /// \brief Create an Adapter from a configured ProtocolAdapter.
  static std::shared_ptr<Adapter> make(
    std::shared_ptr<execution::ProtocolAdapter> protocol_adapter);

  /// \brief Register the per-node navigation callback (node + entering edge).
  void on_navigate(NavigateCallback callback);

  /// \brief Register the whole-base callback (full plan on order acceptance).
  void on_base(BaseCallback callback);

  /// \brief Access the Reporter for arrival + state reporting.
  std::shared_ptr<Reporter> reporter();

  /// \brief Connect MQTT, subscribe to Order, publish ONLINE, start the loop.
  void start();

  /// \brief Stop the loop, publish OFFLINE, disconnect MQTT.
  void stop();

  ~Adapter();

private:
  explicit Adapter(
    std::shared_ptr<execution::ProtocolAdapter> protocol_adapter);

  // Overlay robot-owned fields onto the strategy-produced State, then publish.
  void publish_state(const types::State& order_state);

  // Resolve the full order Node / entering Edge from the persisted active order.
  types::Node resolve_node(const types::NodeState& target);
  std::optional<types::Edge> resolve_edge(
    const std::optional<types::EdgeState>& target);

  void run_heartbeat();

  std::shared_ptr<execution::ProtocolAdapter> protocol_adapter_;
  std::shared_ptr<RobotIoState> robot_io_;

  std::shared_ptr<client::AGVContext> context_;
  std::shared_ptr<client::OrderExecutionResource> execution_;
  std::shared_ptr<execution::Handler> handler_;
  std::shared_ptr<Reporter> reporter_;

  NavigateCallback navigate_callback_;
  BaseCallback base_callback_;
  std::mutex callback_mutex_;

  // Serialises State publishes from the reporter (spin thread) and heartbeat.
  std::mutex publish_mutex_;

  std::thread spin_thread_;
  std::thread heartbeat_thread_;
  std::atomic_bool started_{false};

  bool heartbeat_running_ = false;
  std::mutex heartbeat_mutex_;
  std::condition_variable heartbeat_cv_;
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__ADAPTER_HPP_
