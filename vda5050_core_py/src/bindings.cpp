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
 */

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <sstream>
#include <string>

#include <vda5050_core/adapter/adapter.hpp>
#include <vda5050_core/adapter/navigation_manager.hpp>
#include <vda5050_core/execution/protocol_adapter.hpp>
#include <vda5050_core/transport/mqtt_client_interface.hpp>
#include <vda5050_core/types/action.hpp>
#include <vda5050_core/types/action_parameter.hpp>
#include <vda5050_core/types/action_state.hpp>
#include <vda5050_core/types/action_status.hpp>
#include <vda5050_core/types/agv_position.hpp>
#include <vda5050_core/types/battery_state.hpp>
#include <vda5050_core/types/blocking_type.hpp>
#include <vda5050_core/types/node.hpp>
#include <vda5050_core/types/node_position.hpp>

namespace py = pybind11;
using namespace vda5050_core;

PYBIND11_MODULE(_core, m)
{
  m.doc() =
    "Python bindings for vda5050_core: the AGV-side Adapter facade.\n"
    "The C++ adapter owns the MQTT lifecycle, order graph execution, "
    "header IDs, and 1Hz state publishing. Python implements only the "
    "robot-side navigation callback and reports state back via "
    "NavigationManager.";

  // ===== Enums =====
  py::enum_<types::BlockingType>(m, "BlockingType")
    .value("NONE", types::BlockingType::NONE)
    .value("SOFT", types::BlockingType::SOFT)
    .value("HARD", types::BlockingType::HARD);

  py::enum_<types::ActionStatus>(m, "ActionStatus")
    .value("WAITING", types::ActionStatus::WAITING)
    .value("INITIALIZING", types::ActionStatus::INITIALIZING)
    .value("RUNNING", types::ActionStatus::RUNNING)
    .value("PAUSED", types::ActionStatus::PAUSED)
    .value("FINISHED", types::ActionStatus::FINISHED)
    .value("FAILED", types::ActionStatus::FAILED);

  // ===== Types =====
  py::class_<types::ActionParameter>(m, "ActionParameter")
    .def(py::init<>())
    .def_readwrite("key", &types::ActionParameter::key)
    .def_readwrite("value", &types::ActionParameter::value)
    .def("__repr__", [](const types::ActionParameter& p) {
      return "ActionParameter(key='" + p.key + "', value='" + p.value + "')";
    });

  py::class_<types::Action>(m, "Action")
    .def(py::init<>())
    .def_readwrite("action_type", &types::Action::action_type)
    .def_readwrite("action_id", &types::Action::action_id)
    .def_readwrite("blocking_type", &types::Action::blocking_type)
    .def_readwrite("action_description", &types::Action::action_description)
    .def_readwrite("action_parameters", &types::Action::action_parameters)
    .def("__repr__", [](const types::Action& a) {
      return "Action(action_type='" + a.action_type +
             "', action_id='" + a.action_id + "')";
    });

  py::class_<types::ActionState>(m, "ActionState")
    .def(py::init<>())
    .def_readwrite("action_id", &types::ActionState::action_id)
    .def_readwrite("action_type", &types::ActionState::action_type)
    .def_readwrite("action_description", &types::ActionState::action_description)
    .def_readwrite("action_status", &types::ActionState::action_status)
    .def_readwrite("result_description", &types::ActionState::result_description)
    .def("__repr__", [](const types::ActionState& a) {
      return "ActionState(action_id='" + a.action_id + "')";
    });

  py::class_<types::BatteryState>(m, "BatteryState")
    .def(py::init<>())
    .def_readwrite("battery_charge", &types::BatteryState::battery_charge)
    .def_readwrite("battery_voltage", &types::BatteryState::battery_voltage)
    .def_readwrite("battery_health", &types::BatteryState::battery_health)
    .def_readwrite("charging", &types::BatteryState::charging)
    .def_readwrite("reach", &types::BatteryState::reach)
    .def("__repr__", [](const types::BatteryState& b) {
      std::ostringstream s;
      s << "BatteryState(battery_charge=" << b.battery_charge
        << ", charging=" << (b.charging ? "True" : "False") << ")";
      return s.str();
    });

  py::class_<types::NodePosition>(m, "NodePosition")
    .def(py::init<>())
    .def_readwrite("x", &types::NodePosition::x)
    .def_readwrite("y", &types::NodePosition::y)
    .def_readwrite("theta", &types::NodePosition::theta)
    .def_readwrite(
      "allowed_deviation_x_y", &types::NodePosition::allowed_deviation_x_y)
    .def_readwrite(
      "allowed_deviation_theta", &types::NodePosition::allowed_deviation_theta)
    .def_readwrite("map_id", &types::NodePosition::map_id)
    .def_readwrite("map_description", &types::NodePosition::map_description)
    .def("__repr__", [](const types::NodePosition& p) {
      std::ostringstream s;
      s << "NodePosition(x=" << p.x << ", y=" << p.y
        << ", map_id='" << p.map_id << "')";
      return s.str();
    });

  py::class_<types::Node>(m, "Node")
    .def(py::init<>())
    .def_readwrite("node_id", &types::Node::node_id)
    .def_readwrite("sequence_id", &types::Node::sequence_id)
    .def_readwrite("released", &types::Node::released)
    .def_readwrite("actions", &types::Node::actions)
    .def_readwrite("node_position", &types::Node::node_position)
    .def_readwrite("node_description", &types::Node::node_description)
    .def("__repr__", [](const types::Node& n) {
      std::ostringstream s;
      s << "Node(node_id='" << n.node_id
        << "', sequence_id=" << n.sequence_id
        << ", released=" << (n.released ? "True" : "False") << ")";
      return s.str();
    });

  py::class_<types::AGVPosition>(m, "AGVPosition")
    .def(py::init<>())
    .def_readwrite(
      "position_initialized", &types::AGVPosition::position_initialized)
    .def_readwrite(
      "localization_score", &types::AGVPosition::localization_score)
    .def_readwrite("deviation_range", &types::AGVPosition::deviation_range)
    .def_readwrite("x", &types::AGVPosition::x)
    .def_readwrite("y", &types::AGVPosition::y)
    .def_readwrite("theta", &types::AGVPosition::theta)
    .def_readwrite("map_id", &types::AGVPosition::map_id)
    .def_readwrite("map_description", &types::AGVPosition::map_description)
    .def("__repr__", [](const types::AGVPosition& p) {
      std::ostringstream s;
      s << "AGVPosition(x=" << p.x << ", y=" << p.y
        << ", theta=" << p.theta << ", map_id='" << p.map_id << "')";
      return s.str();
    });

  // ===== Transport =====
  py::class_<transport::MqttClientInterface,
             std::shared_ptr<transport::MqttClientInterface>>(
    m, "MqttClient",
    "Opaque MQTT client handle. Construct via create_default_mqtt_client.");

  m.def(
    "create_default_mqtt_client",
    [](const std::string& broker_address, const std::string& client_id)
      -> std::shared_ptr<transport::MqttClientInterface> {
      // create_default_client_unique returns unique_ptr; promote to shared
      // because ProtocolAdapter::make takes shared_ptr.
      auto unique =
        transport::create_default_client_unique(broker_address, client_id);
      return std::shared_ptr<transport::MqttClientInterface>(std::move(unique));
    },
    py::arg("broker_address"), py::arg("client_id"),
    py::call_guard<py::gil_scoped_release>(),
    "Create a Paho-backed MQTT client. Pass the result to "
    "ProtocolAdapter.make().");

  // ===== Execution =====
  py::class_<execution::ProtocolAdapter,
             std::shared_ptr<execution::ProtocolAdapter>>(m, "ProtocolAdapter")
    .def_static(
      "make", &execution::ProtocolAdapter::make,
      py::arg("mqtt_client"), py::arg("interface"), py::arg("version"),
      py::arg("manufacturer"), py::arg("serial_number"),
      "Create a ProtocolAdapter. `interface` is the VDA5050 interface name "
      "(usually \"uagv\"); `version` is the full protocol version "
      "(e.g. \"2.1.0\"); `manufacturer` and `serial_number` identify the AGV.")
    .def(
      "connected", &execution::ProtocolAdapter::connected,
      py::call_guard<py::gil_scoped_release>());

  // ===== Adapter =====
  py::class_<adapter::NavigationManager,
             std::shared_ptr<adapter::NavigationManager>>(
    m, "NavigationManager",
    "Robot-side handle for reporting navigation completion and state. "
    "Obtained from Adapter.navigation_manager(); cannot be constructed "
    "directly.")
    .def(
      "node_reached", &adapter::NavigationManager::node_reached,
      py::arg("node"),
      py::call_guard<py::gil_scoped_release>(),
      "Signal that the robot has reached `node`. Unblocks order execution "
      "so the next node in the order is dispatched.")
    .def(
      "set_driving", &adapter::NavigationManager::set_driving,
      py::arg("driving"),
      py::call_guard<py::gil_scoped_release>(),
      "Update the published State message's `driving` flag.")
    .def(
      "set_agv_position", &adapter::NavigationManager::set_agv_position,
      py::arg("position"),
      py::call_guard<py::gil_scoped_release>(),
      "Update the published State message's `agv_position`. Typically "
      "called at the robot's localization tick rate.")
    .def(
      "set_battery_state", &adapter::NavigationManager::set_battery_state,
      py::arg("battery_state"),
      py::call_guard<py::gil_scoped_release>(),
      "Update the published State message's `batteryState` fields.")
    .def(
      "set_action_states", &adapter::NavigationManager::set_action_states,
      py::arg("action_states"),
      py::call_guard<py::gil_scoped_release>(),
      "Update the published State message's `actionStates` array.");

  py::class_<adapter::Adapter, std::shared_ptr<adapter::Adapter>>(
    m, "Adapter",
    "AGV-side facade. Wires a ProtocolAdapter into the execution framework, "
    "subscribes to Order, publishes State at 1Hz, and routes per-node "
    "navigation requests to the Python `on_navigate` callback.")
    .def_static(
      "make", &adapter::Adapter::make, py::arg("protocol_adapter"),
      "Construct an Adapter from a ProtocolAdapter. Start the spin loop "
      "with .start().")
    .def(
      "on_navigate", &adapter::Adapter::on_navigate, py::arg("callback"),
      "Register the per-node navigation callback. Signature: "
      "`Callable[[Node], None]`. Invoked on the C++ spin thread; long-"
      "running work (HTTP, polling) should be dispatched to a Python "
      "worker thread so the spin loop is not blocked.")
    .def(
      "navigation_manager", &adapter::Adapter::navigation_manager,
      "Return the NavigationManager for reporting node arrival and state.")
    .def(
      "start", &adapter::Adapter::start,
      py::call_guard<py::gil_scoped_release>(),
      "Connect MQTT, subscribe to Order, publish ONLINE, and start the "
      "execution spin thread. Returns once the spin thread is running.")
    .def(
      "stop", &adapter::Adapter::stop,
      py::call_guard<py::gil_scoped_release>(),
      "Unsubscribe, stop the spin loop, publish OFFLINE, disconnect MQTT. "
      "Joins the spin thread; releases the GIL while waiting so in-flight "
      "Python callbacks can complete.");
}
