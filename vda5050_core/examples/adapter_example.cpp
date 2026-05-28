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
#include <csignal>
#include <thread>

#include <vda5050_core/adapter/adapter.hpp>
#include <vda5050_core/execution/protocol_adapter.hpp>
#include <vda5050_core/logger/logger.hpp>
#include <vda5050_core/transport/mqtt_client_interface.hpp>
#include <vda5050_core/types/action_state.hpp>
#include <vda5050_core/types/action_status.hpp>
#include <vda5050_core/types/battery_state.hpp>

using vda5050_core::adapter::Adapter;
using vda5050_core::adapter::NavigationManager;
using vda5050_core::execution::ProtocolAdapter;
using vda5050_core::types::ActionState;
using vda5050_core::types::ActionStatus;
using vda5050_core::types::BatteryState;

namespace {

void apply_empty_reported_state(NavigationManager& nav)
{
  nav.set_action_states({});

  BatteryState battery;
  battery.battery_charge = 0.0;
  battery.charging = false;
  nav.set_battery_state(battery);
}

void apply_full_reported_state(NavigationManager& nav)
{
  ActionState action;
  action.action_id = "example_action_0";
  action.action_type = "startPause";
  action.action_status = ActionStatus::FINISHED;
  nav.set_action_states({action});

  BatteryState battery;
  battery.battery_charge = 85.0;
  battery.battery_voltage = 48.0;
  battery.battery_health = 95;
  battery.charging = false;
  battery.reach = 1200;
  nav.set_battery_state(battery);
}

}  // namespace

std::atomic_bool running{true};

void signal_handler(int /*signal*/)
{
  running = false;
}

int main(int argc, char** argv)
{
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  const std::string broker =
    (argc > 1) ? argv[1] : "tcp://localhost:1883";
  const std::string client_id =
    (argc > 2) ? argv[2] : "vda5050_adapter_example";

  constexpr const char* k_interface = "uagv";
  constexpr const char* k_protocol_version = "2.1.0";  // JSON header field
  constexpr const char* k_manufacturer = "Manufacturer";
  constexpr const char* k_serial_number = "S001";

  auto mqtt_client = vda5050_core::transport::create_default_client_unique(
    broker, client_id);
  auto protocol_adapter = ProtocolAdapter::make(
    std::move(mqtt_client), k_interface, k_protocol_version, k_manufacturer,
    k_serial_number);
  auto adapter = Adapter::make(protocol_adapter);
  auto nav = adapter->navigation_manager();

  constexpr bool k_use_full_state = false;
  if (k_use_full_state)
  {
    apply_full_reported_state(*nav);
  }
  else
  {
    apply_empty_reported_state(*nav);
  }

  adapter->on_navigate([&nav](vda5050_core::types::Node node) {
    if (!node.node_position.has_value())
    {
      VDA5050_WARN("Received node without position: {}", node.node_id);
      return;
    }

    const auto& position = node.node_position.value();
    VDA5050_INFO_STREAM(
      "Navigate to node [" << node.node_id << "] at ["
                           << position.x << ", " << position.y << "]");

    nav->set_driving(true);

    std::thread([nav, node]() {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      nav->node_reached(node);
      VDA5050_INFO("Simulated arrival at node {}", node.node_id);
    }).detach();
  });

  // on_cancel() is not yet implemented — see adapter.hpp
  // adapter->on_cancel([]() {
  //   VDA5050_INFO("Cancel navigation requested");
  // });

  adapter->start();
  VDA5050_INFO(
    "Adapter started on {} — order topic: {}/{}/{}/{}/order",
    broker, k_interface,
    ProtocolAdapter::get_topic_version(k_protocol_version), k_manufacturer,
    k_serial_number);

  while (running)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  VDA5050_INFO("Shutting down ...");
  adapter->stop();
  VDA5050_INFO("Adapter stopped");

  return 0;
}
