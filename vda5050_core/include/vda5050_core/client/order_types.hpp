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

#ifndef VDA5050_CORE__CLIENT__ORDER_TYPES_HPP_
#define VDA5050_CORE__CLIENT__ORDER_TYPES_HPP_

#include <string>
#include <utility>

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/types/order.hpp"

namespace vda5050_core {

namespace client {

/// \brief Inbound carrier for a VDA5050 Order message.
///
/// Wraps a `types::Order` as an `UpdateBase` so it can be pushed through a
/// `Provider`, cached by a `Context`, and queried by an order-execution
/// `Strategy`. The Order is the message master control sends on the `order`
/// topic to instruct the AGV which nodes/edges to traverse.

// Stores the Order message
struct OrderUpdate
: public execution::Initialize<OrderUpdate, execution::UpdateBase>
{
  /// \brief The VDA5050 order as received from master control.
  types::Order order;

  explicit OrderUpdate(types::Order order) : order(std::move(order))
  {
    // Nothing to do here ...
  }
};

/// \brief Persistent identity of this AGV on the VDA5050 interface.
///
/// Stored once as a `ResourceBase` in the Context. It mirrors the topic
/// identity used by the `ProtocolAdapter`
/// (`<interface_name>/<version>/<manufacturer>/<serial_number>/...`) and lets a
/// Strategy reason about which AGV it is acting for.

// Stores the AGV's identity/config information
struct ConfigResource
: public execution::Initialize<ConfigResource, execution::ResourceBase>
{
  /// \brief Interface name, e.g. "uagv".
  std::string interface_name;

  /// \brief Protocol/major version segment, e.g. "v2".
  std::string version;

  /// \brief Manufacturer of the AGV.
  std::string manufacturer;

  /// \brief Serial number of the AGV.
  std::string serial_number;

  ConfigResource(
    std::string interface_name, std::string version, std::string manufacturer,
    std::string serial_number)
  : interface_name(std::move(interface_name)),
    version(std::move(version)),
    manufacturer(std::move(manufacturer)),
    serial_number(std::move(serial_number))
  {
    // Nothing to do here ...
  }
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__ORDER_TYPES_HPP_
