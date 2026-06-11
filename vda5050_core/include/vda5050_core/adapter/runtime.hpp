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

#ifndef VDA5050_CORE__ADAPTER__RUNTIME_HPP_
#define VDA5050_CORE__ADAPTER__RUNTIME_HPP_

#include <memory>
#include <string>

#include "vda5050_core/adapter/adapter.hpp"
#include "vda5050_core/adapter/reporter.hpp"

namespace vda5050_core {

namespace adapter {

/// \brief High-level robot runtime: the single facade exposed to behaviour code.
///
/// Owns the whole runtime stack — MQTT client, `ProtocolAdapter`, `Adapter`,
/// execution loop and threading — so callers (notably the Python bindings) only
/// implement robot behaviour: a navigation callback and state reporting. Keeping
/// transport and orchestration inside C++ means the callback bridge owns all GIL
/// management, rather than scattering it across the binding layer.
class RobotRuntime
{
public:
  /// \brief Build a runtime bound to an MQTT broker.
  ///
  /// \param broker_address  e.g. "tcp://localhost:1883"
  /// \param client_id       MQTT client id, unique per AGV
  /// \param manufacturer    VDA5050 manufacturer id
  /// \param serial_number   VDA5050 AGV serial number
  /// \param interface       VDA5050 interface name (default "uagv")
  /// \param version         full protocol version (default "2.0.0")
  static std::shared_ptr<RobotRuntime> make(
    const std::string& broker_address, const std::string& client_id,
    const std::string& manufacturer, const std::string& serial_number,
    const std::string& interface = "uagv",
    const std::string& version = "2.0.0");

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

private:
  explicit RobotRuntime(std::shared_ptr<Adapter> adapter);

  std::shared_ptr<Adapter> adapter_;
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__RUNTIME_HPP_
