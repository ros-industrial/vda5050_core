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

/// \brief AGV-side integration facade over the vda5050_core execution framework
class Adapter : public std::enable_shared_from_this<Adapter>
{
public:
  /// \brief Create an Adapter instance
  ///
  /// \param protocol_adapter Configured protocol adapter for MQTT messaging
  /// \return Shared pointer to the Adapter
  static std::shared_ptr<Adapter> make(
    std::shared_ptr<execution::ProtocolAdapter> protocol_adapter);

  /// \brief Register callback invoked when the AGV should navigate to a node
  void on_navigate(std::function<void(types::Node)> callback);

  // on_cancel() is reserved for VDA5050 §6.10.2 cancel instant action support.
  // Wiring it requires subscribing to an InstantAction topic and dispatching
  // the cancel through the execution engine — not yet implemented.
  // void on_cancel(std::function<void()> callback);

  /// \brief Access the navigation manager for status reporting and state updates
  std::shared_ptr<NavigationManager> navigation_manager();

  /// \brief Connect MQTT, subscribe to orders, and start the execution loop
  void start();

  /// \brief Stop the execution loop and publish OFFLINE connection state
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
