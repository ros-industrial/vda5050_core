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

#include <functional>
#include <memory>

#include "vda5050_core/adapter/navigation_manager.hpp"
#include "vda5050_core/execution/protocol_adapter.hpp"
#include "vda5050_core/types/node.hpp"

namespace vda5050_core {

namespace adapter {

/// \brief AGV-side integration facade over the vda5050_core execution framework.
///
/// Wires a `ProtocolAdapter` to the client Context+Strategy pipeline
/// (`AGVContext` + OrderAcceptance / OrderTraversal / OrderActions /
/// StateReporting): subscribes to Order, dispatches per-node navigation to the
/// `on_navigate` callback, publishes State (on change plus a 1 Hz heartbeat),
/// and manages the MQTT connection lifecycle.
///
/// The Python bindings expose exactly this surface, so the public API mirrors
/// the GameTL reference adapter — only the implementation differs (it drives
/// the real strategies instead of an inline engine).
class Adapter : public std::enable_shared_from_this<Adapter>
{
public:
  /// \brief Create an Adapter from a configured ProtocolAdapter.
  static std::shared_ptr<Adapter> make(
    std::shared_ptr<execution::ProtocolAdapter> protocol_adapter);

  /// \brief Register the per-node navigation callback.
  ///
  /// Invoked on the spin thread when the order traversal dispatches a node
  /// (`NavigateToNodeEvent`). The callback receives the full order `Node`
  /// (actions + position). Long-running work (HTTP, polling) must be moved to a
  /// worker thread so the spin loop can receive `node_reached` and advance.
  void on_navigate(std::function<void(types::Node)> callback);

  /// \brief Access the NavigationManager for arrival + state reporting.
  std::shared_ptr<NavigationManager> navigation_manager();

  /// \brief Connect MQTT, subscribe to Order, publish ONLINE, start the loop.
  void start();

  /// \brief Stop the loop, publish OFFLINE, disconnect MQTT.
  void stop();

  ~Adapter();

private:
  Adapter(
    std::shared_ptr<execution::ProtocolAdapter> protocol_adapter,
    std::shared_ptr<NavigationManager> navigation_manager);

  class Implementation;
  std::unique_ptr<Implementation> pimpl_;
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__ADAPTER_HPP_
