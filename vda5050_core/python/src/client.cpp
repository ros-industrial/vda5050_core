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

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>

#include "vda5050_core/client/adapter/action_execution.hpp"
#include "vda5050_core/client/adapter/action_request.hpp"
#include "vda5050_core/client/adapter/adapter.hpp"
#include "vda5050_core/client/adapter/edge_request.hpp"
#include "vda5050_core/client/adapter/execution.hpp"
#include "vda5050_core/client/adapter/localization_request.hpp"
#include "vda5050_core/client/adapter/node_request.hpp"
#include "vda5050_core/client/adapter/order_execution.hpp"
#include "vda5050_core/client/adapter/state_manager.hpp"
#include "vda5050_core/client/adapter/transformation.hpp"
#include "vda5050_core/execution/protocol_adapter.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/action_status.hpp"
#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/battery_state.hpp"
#include "vda5050_core/types/e_stop.hpp"
#include "vda5050_core/types/error.hpp"
#include "vda5050_core/types/error_level.hpp"
#include "vda5050_core/types/error_reference.hpp"
#include "vda5050_core/types/info.hpp"
#include "vda5050_core/types/info_level.hpp"
#include "vda5050_core/types/info_reference.hpp"
#include "vda5050_core/types/operating_mode.hpp"
#include "vda5050_core/types/safety_state.hpp"
#include "vda5050_core/types/velocity.hpp"

#include "vda5050_core_py/client.hpp"

namespace vda5050_core_py {

using vda5050_core::client::adapter::ActionExecution;
using vda5050_core::client::adapter::ActionRequest;
using vda5050_core::client::adapter::Adapter;
using vda5050_core::client::adapter::EdgeRequest;
using vda5050_core::client::adapter::Execution;
using vda5050_core::client::adapter::LocalizationRequest;
using vda5050_core::client::adapter::NodeRequest;
using vda5050_core::client::adapter::OrderExecution;
using vda5050_core::client::adapter::Pose2D;
using vda5050_core::client::adapter::StateManager;
using vda5050_core::client::adapter::Transformation;
using vda5050_core::execution::ProtocolAdapter;

void bind_client(py::module_& m)
{
  auto m_client = m.def_submodule("client", "Native VDA5050 client API");

  py::class_<Pose2D>(m_client, "Pose2D")
    .def(py::init<>())
    .def_readwrite("x", &Pose2D::x)
    .def_readwrite("y", &Pose2D::y)
    .def_readwrite("theta", &Pose2D::theta);

  py::class_<Transformation>(m_client, "Transformation")
    .def_static("calibrate", &Transformation::calibrate)
    .def("to_world_pose", &Transformation::to_world_pose)
    .def("to_agv_pose", &Transformation::to_agv_pose);

  py::class_<StateManager, std::shared_ptr<StateManager>>(
    m_client, "StateManager")
    .def("set_position", &StateManager::set_position)
    .def("set_velocity", &StateManager::set_velocity)
    .def("set_driving", &StateManager::set_driving)
    .def("set_paused", &StateManager::set_paused)
    .def("set_new_base_request", &StateManager::set_new_base_request)
    .def(
      "set_distance_since_last_node",
      &StateManager::set_distance_since_last_node)
    .def("set_battery_state", &StateManager::set_battery_state)
    .def("set_operating_mode", &StateManager::set_operating_mode)
    .def("set_safety_state", &StateManager::set_safety_state)
    .def("add_action_state", &StateManager::add_action_state)
    .def("set_action_states", &StateManager::set_action_states)
    .def("clear_action_states", &StateManager::clear_action_states)
    .def("add_error", &StateManager::add_error)
    .def("set_errors", &StateManager::set_errors)
    .def("clear_errors", &StateManager::clear_errors)
    .def("add_information", &StateManager::add_information)
    .def("set_information", &StateManager::set_information)
    .def("remove_information", &StateManager::remove_information)
    .def("initialize_position", &StateManager::initialize_position)
    .def("set_transformation", &StateManager::set_transformation);

  py::class_<ProtocolAdapter, std::shared_ptr<ProtocolAdapter>>(
    m_client, "ProtocolAdapter")
    .def_static(
      "make", &ProtocolAdapter::make, py::arg("mqtt_client"),
      py::arg("interface"), py::arg("version"), py::arg("manufacturer"),
      py::arg("serial_number"))
    .def("connect", &ProtocolAdapter::connect)
    .def("disconnect", &ProtocolAdapter::disconnect)
    .def("connected", &ProtocolAdapter::connected);

  // --- Requests (read-only, created by the Adapter) ---

  py::class_<NodeRequest>(m_client, "NodeRequest")
    .def("node_id", &NodeRequest::node_id)
    .def("sequence_id", &NodeRequest::sequence_id)
    .def("node_position", &NodeRequest::node_position)
    .def("node_description", &NodeRequest::node_description);

  py::class_<EdgeRequest>(m_client, "EdgeRequest")
    .def("edge_id", &EdgeRequest::edge_id)
    .def("sequence_id", &EdgeRequest::sequence_id)
    .def("trajectory", &EdgeRequest::trajectory)
    .def("max_speed", &EdgeRequest::max_speed)
    .def("min_height", &EdgeRequest::min_height)
    .def("max_height", &EdgeRequest::max_height)
    .def("rotation_allowed", &EdgeRequest::rotation_allowed)
    .def("max_rotation_speed", &EdgeRequest::max_rotation_speed)
    .def("length", &EdgeRequest::length);

  py::class_<ActionRequest>(m_client, "ActionRequest")
    .def("action_id", &ActionRequest::action_id)
    .def("action_type", &ActionRequest::action_type)
    .def("action_parameters", &ActionRequest::action_parameters)
    .def("action_description", &ActionRequest::action_description)
    .def("order_id", &ActionRequest::order_id)
    .def("order_update_id", &ActionRequest::order_update_id);

  py::class_<LocalizationRequest>(m_client, "LocalizationRequest")
    .def("x", &LocalizationRequest::x)
    .def("y", &LocalizationRequest::y)
    .def("theta", &LocalizationRequest::theta)
    .def("map_id", &LocalizationRequest::map_id);

  // --- Executions (handles used to report progress back) ---

  py::class_<Execution, std::shared_ptr<Execution>>(m_client, "Execution")
    .def("finished", py::overload_cast<>(&Execution::finished))
    .def("failed", &Execution::failed, py::arg("reason"))
    .def("okay", &Execution::okay)
    .def("is_finished", &Execution::is_finished)
    .def("failure_reason", &Execution::failure_reason);

  py::class_<OrderExecution, Execution, std::shared_ptr<OrderExecution>>(
    m_client, "OrderExecution")
    .def("order_id", &OrderExecution::order_id)
    .def("order_update_id", &OrderExecution::order_update_id);

  py::class_<ActionExecution, Execution, std::shared_ptr<ActionExecution>>(
    m_client, "ActionExecution")
    .def("initializing", &ActionExecution::initializing)
    .def("running", &ActionExecution::running)
    .def(
      "paused", &ActionExecution::paused,
      py::arg("result_description") = std::nullopt)
    .def("finished", py::overload_cast<>(&ActionExecution::finished))
    .def(
      "finished",
      py::overload_cast<std::optional<std::string>>(&ActionExecution::finished),
      py::arg("result_description"));

  // --- Adapter ---
  // Callbacks are invoked from native threads; pybind11 acquires the GIL for
  // them. start()/stop() release the GIL so they cannot deadlock against
  // callbacks that are still running.

  py::class_<Adapter, std::shared_ptr<Adapter>>(m_client, "Adapter")
    .def_static("make", &Adapter::make, py::arg("protocol_adapter"))
    .def("on_navigate", &Adapter::on_navigate, py::arg("callback"))
    .def("on_action", &Adapter::on_action, py::arg("callback"))
    .def("on_localize", &Adapter::on_localize, py::arg("callback"))
    .def("state_manager", &Adapter::state_manager)
    .def("set_factsheet", &Adapter::set_factsheet, py::arg("factsheet"))
    .def("start", &Adapter::start, py::call_guard<py::gil_scoped_release>())
    .def("stop", &Adapter::stop, py::call_guard<py::gil_scoped_release>());
}

}  // namespace vda5050_core_py
