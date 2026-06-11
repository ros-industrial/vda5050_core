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

#include <sstream>
#include <string>

#include <vda5050_core/adapter/reporter.hpp>
#include <vda5050_core/adapter/runtime.hpp>
#include <vda5050_core/types/action.hpp>
#include <vda5050_core/types/action_parameter.hpp>
#include <vda5050_core/types/action_state.hpp>
#include <vda5050_core/types/action_status.hpp>
#include <vda5050_core/types/agv_position.hpp>
#include <vda5050_core/types/battery_state.hpp>
#include <vda5050_core/types/blocking_type.hpp>
#include <vda5050_core/types/edge.hpp>
#include <vda5050_core/types/node.hpp>
#include <vda5050_core/types/node_position.hpp>
#include <vda5050_core/types/order.hpp>

namespace py = pybind11;
using namespace vda5050_core;

PYBIND11_MODULE(_core, m)
{
  m.doc() =
    "Python bindings for vda5050_core: the AGV-side RobotRuntime facade.\n"
    "C++ owns the runtime — MQTT, protocol handling, order execution, threading "
    "and 1Hz state publishing. Python implements only robot behaviour: the "
    "navigation callback and state reporting via Reporter. All GIL management "
    "stays inside the runtime's callback bridge.";

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
    .def_readwrite("value", &types::ActionParameter::value);

  py::class_<types::Action>(m, "Action")
    .def(py::init<>())
    .def_readwrite("action_type", &types::Action::action_type)
    .def_readwrite("action_id", &types::Action::action_id)
    .def_readwrite("blocking_type", &types::Action::blocking_type)
    .def_readwrite("action_description", &types::Action::action_description)
    .def_readwrite("action_parameters", &types::Action::action_parameters);

  py::class_<types::ActionState>(m, "ActionState")
    .def(py::init<>())
    .def_readwrite("action_id", &types::ActionState::action_id)
    .def_readwrite("action_type", &types::ActionState::action_type)
    .def_readwrite("action_description", &types::ActionState::action_description)
    .def_readwrite("action_status", &types::ActionState::action_status)
    .def_readwrite("result_description", &types::ActionState::result_description);

  py::class_<types::BatteryState>(m, "BatteryState")
    .def(py::init<>())
    .def_readwrite("battery_charge", &types::BatteryState::battery_charge)
    .def_readwrite("battery_voltage", &types::BatteryState::battery_voltage)
    .def_readwrite("battery_health", &types::BatteryState::battery_health)
    .def_readwrite("charging", &types::BatteryState::charging)
    .def_readwrite("reach", &types::BatteryState::reach);

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
      s << "NodePosition(x=" << p.x << ", y=" << p.y << ", map_id='" << p.map_id
        << "')";
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
      s << "Node(node_id='" << n.node_id << "', sequence_id=" << n.sequence_id
        << ")";
      return s.str();
    });

  py::class_<types::Edge>(m, "Edge")
    .def(py::init<>())
    .def_readwrite("edge_id", &types::Edge::edge_id)
    .def_readwrite("sequence_id", &types::Edge::sequence_id)
    .def_readwrite("released", &types::Edge::released)
    .def_readwrite("start_node_id", &types::Edge::start_node_id)
    .def_readwrite("end_node_id", &types::Edge::end_node_id)
    .def_readwrite("actions", &types::Edge::actions)
    .def_readwrite("edge_description", &types::Edge::edge_description)
    .def("__repr__", [](const types::Edge& e) {
      std::ostringstream s;
      s << "Edge(edge_id='" << e.edge_id << "', sequence_id=" << e.sequence_id
        << ", " << e.start_node_id << "->" << e.end_node_id << ")";
      return s.str();
    });

  py::class_<types::Order>(m, "Order")
    .def(py::init<>())
    .def_readwrite("order_id", &types::Order::order_id)
    .def_readwrite("order_update_id", &types::Order::order_update_id)
    .def_readwrite("nodes", &types::Order::nodes)
    .def_readwrite("edges", &types::Order::edges)
    .def("__repr__", [](const types::Order& o) {
      std::ostringstream s;
      s << "Order(order_id='" << o.order_id << "', nodes=" << o.nodes.size()
        << ", edges=" << o.edges.size() << ")";
      return s.str();
    });

  py::class_<types::AGVPosition>(m, "AGVPosition")
    .def(py::init<>())
    .def_readwrite(
      "position_initialized", &types::AGVPosition::position_initialized)
    .def_readwrite("localization_score", &types::AGVPosition::localization_score)
    .def_readwrite("deviation_range", &types::AGVPosition::deviation_range)
    .def_readwrite("x", &types::AGVPosition::x)
    .def_readwrite("y", &types::AGVPosition::y)
    .def_readwrite("theta", &types::AGVPosition::theta)
    .def_readwrite("map_id", &types::AGVPosition::map_id)
    .def_readwrite("map_description", &types::AGVPosition::map_description);

  // ===== Reporter =====
  py::class_<adapter::Reporter, std::shared_ptr<adapter::Reporter>>(
    m, "Reporter",
    "Robot-side handle for reporting arrival and live state. Obtained from "
    "RobotRuntime.reporter(); cannot be constructed directly.")
    .def(
      "node_reached", &adapter::Reporter::node_reached, py::arg("node_id"),
      py::arg("sequence_id"), py::call_guard<py::gil_scoped_release>(),
      "Report that the robot reached the node with this id + sequence id, "
      "unblocking order execution so the next node is dispatched.")
    .def(
      "set_driving", &adapter::Reporter::set_driving, py::arg("driving"),
      py::call_guard<py::gil_scoped_release>(),
      "Update the published State's `driving` flag.")
    .def(
      "set_agv_position", &adapter::Reporter::set_agv_position,
      py::arg("position"), py::call_guard<py::gil_scoped_release>(),
      "Update the published State's `agvPosition`.")
    .def(
      "set_battery_state", &adapter::Reporter::set_battery_state,
      py::arg("battery_state"), py::call_guard<py::gil_scoped_release>(),
      "Update the published State's `batteryState`.")
    .def(
      "set_action_states", &adapter::Reporter::set_action_states,
      py::arg("action_states"), py::call_guard<py::gil_scoped_release>(),
      "Update the published State's `actionStates` array.");

  // ===== RobotRuntime =====
  py::class_<adapter::RobotRuntime, std::shared_ptr<adapter::RobotRuntime>>(
    m, "RobotRuntime",
    "AGV-side runtime facade. Owns MQTT, protocol handling, order execution and "
    "1Hz state publishing; Python implements only the navigation callback and "
    "state reporting.")
    .def(
      py::init(&adapter::RobotRuntime::make), py::arg("broker"),
      py::arg("client_id"), py::arg("manufacturer"), py::arg("serial_number"),
      py::arg("interface") = "uagv", py::arg("version") = "2.0.0",
      "Build a runtime bound to an MQTT broker (e.g. "
      "broker='tcp://localhost:1883').")
    .def(
      "on_navigate", &adapter::RobotRuntime::on_navigate, py::arg("callback"),
      "Register the per-node navigation callback. Signature: "
      "`Callable[[Node, Optional[Edge]], None]` — the node to drive to and the "
      "edge entered to reach it (None for the first node). Invoked on the C++ "
      "spin thread; dispatch long-running work to a worker thread.")
    .def(
      "on_base", &adapter::RobotRuntime::on_base, py::arg("callback"),
      "Register the whole-base callback. Signature: `Callable[[Order], None]` — "
      "the full released base of a newly accepted order, for robots that plan a "
      "whole route at once.")
    .def(
      "reporter", &adapter::RobotRuntime::reporter,
      "Return the Reporter for arrival + state reporting.")
    .def(
      "start", &adapter::RobotRuntime::start,
      py::call_guard<py::gil_scoped_release>(),
      "Connect MQTT, subscribe to Order, publish ONLINE, start the loop.")
    .def(
      "stop", &adapter::RobotRuntime::stop,
      py::call_guard<py::gil_scoped_release>(),
      "Stop the loop, publish OFFLINE, disconnect MQTT.");
}
