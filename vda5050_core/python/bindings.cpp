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

#include "rmf_migration.hpp"

namespace py = pybind11;

using vda5050_core::python::rmf_migration::ActionExecutor;
using vda5050_core::python::rmf_migration::ActivityIdentifier;
using vda5050_core::python::rmf_migration::Adapter;
using vda5050_core::python::rmf_migration::CommandExecution;
using vda5050_core::python::rmf_migration::Destination;
using vda5050_core::python::rmf_migration::FleetConfiguration;
using vda5050_core::python::rmf_migration::FleetUpdateHandle;
using vda5050_core::python::rmf_migration::LocalizationRequest;
using vda5050_core::python::rmf_migration::NavigationRequest;
using vda5050_core::python::rmf_migration::RobotCallbacks;
using vda5050_core::python::rmf_migration::RobotConfiguration;
using vda5050_core::python::rmf_migration::RobotState;
using vda5050_core::python::rmf_migration::RobotUpdateHandle;
using vda5050_core::python::rmf_migration::StopRequest;

PYBIND11_MODULE(vda5050_core_python, m)
{
  m.doc() = "VDA5050 Core Python bindings";

  auto m_rmf_migration =
    m.def_submodule("rmf_migration", "Open-RMF style migration API");

  py::class_<ActivityIdentifier>(m, "ActivityIdentifier")
    .def("order_id", &ActivityIdentifier::order_id)
    .def("action_id", &ActivityIdentifier::action_id)
    .def("__eq__", &ActivityIdentifier::operator==)
    .def("__ne__", &ActivityIdentifier::operator!=);

  py::class_<RobotState>(m, "RobotState")
    .def(py::init<std::string, std::array<double, 3>, double>())
    .def("map", &RobotState::map)
    .def("set_map", &RobotState::set_map)
    .def("position", &RobotState::position)
    .def("set_position", &RobotState::set_position)
    .def("battery_state_of_charge", &RobotState::battery_state_of_charge)
    .def(
      "set_battery_state_of_charge", &RobotState::set_battery_state_of_charge);

  py::class_<RobotConfiguration>(m, "RobotConfiguration")
    .def(
      py::init<std::string, std::string, std::string, std::string>(),
      py::arg("manufacturer"), py::arg("serial_number"),
      py::arg("interface_name") = "uagv", py::arg("version") = "2.0.0")
    .def_readwrite("manufacturer", &RobotConfiguration::manufacturer)
    .def_readwrite("serial_number", &RobotConfiguration::serial_number)
    .def_readwrite("interface_name", &RobotConfiguration::interface_name)
    .def_readwrite("version", &RobotConfiguration::version)
    .def_readwrite("factsheet", &RobotConfiguration::factsheet);

  py::class_<Destination>(m, "Destination")
    .def("map", &Destination::map)
    .def("position", &Destination::position)
    .def("xy", &Destination::xy)
    .def("yaw", &Destination::yaw)
    .def("graph_index", &Destination::graph_index)
    .def("name", &Destination::name)
    .def("speed_limit", &Destination::speed_limit);

  py::class_<CommandExecution>(m, "CommandExecution")
    .def("finished", &CommandExecution::finished)
    .def("failed", &CommandExecution::failed)
    .def("okay", &CommandExecution::okay)
    .def("is_finished", &CommandExecution::is_finished)
    .def("identifier", &CommandExecution::identifier);

  py::class_<RobotCallbacks>(m, "RobotCallbacks")
    .def(py::init<NavigationRequest, StopRequest, ActionExecutor>())
    .def("navigate", &RobotCallbacks::navigate)
    .def("stop", &RobotCallbacks::stop)
    .def("action_executor", &RobotCallbacks::action_executor)
    .def(
      "with_localization", &RobotCallbacks::with_localization,
      py::return_value_policy::reference_internal)
    .def("localize", &RobotCallbacks::localize);

  py::class_<RobotUpdateHandle, std::shared_ptr<RobotUpdateHandle>>(
    m, "RobotUpdateHandle")
    .def("update", &RobotUpdateHandle::update)
    .def("more", &RobotUpdateHandle::more);

  py::class_<FleetConfiguration>(m, "FleetConfiguration")
    .def(
      py::init<std::string, std::string, std::string, std::chrono::seconds>(),
      py::arg("fleet_name"), py::arg("broker_uri"), py::arg("client_id_prefix"),
      py::arg("update_interval") = std::chrono::seconds(30))
    .def("fleet_name", &FleetConfiguration::fleet_name)
    .def("set_fleet_name", &FleetConfiguration::set_fleet_name)
    .def("broker_uri", &FleetConfiguration::broker_uri)
    .def("set_broker_uri", &FleetConfiguration::set_broker_uri)
    .def("client_id_prefix", &FleetConfiguration::client_id_prefix)
    .def("set_client_id_prefix", &FleetConfiguration::set_client_id_prefix)
    .def("update_interval", &FleetConfiguration::update_interval)
    .def("set_update_interval", &FleetConfiguration::set_update_interval)
    .def("known_robots", &FleetConfiguration::known_robots)
    .def(
      "add_known_robot_configuration",
      &FleetConfiguration::add_known_robot_configuration)
    .def(
      "get_known_robot_configuration",
      &FleetConfiguration::get_known_robot_configuration);

  py::class_<FleetUpdateHandle, std::shared_ptr<FleetUpdateHandle>>(
    m, "FleetUpdateHandle")
    .def("add_robot", &FleetUpdateHandle::add_robot);

  py::class_<Adapter, std::shared_ptr<Adapter>>(m, "Adapter")
    .def_static("make", &Adapter::make)
    .def("add_vda5050_fleet", &Adapter::add_vda5050_fleet)
    .def("start", &Adapter::start)
    .def("stop", &Adapter::stop);
}
