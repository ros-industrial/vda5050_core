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

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_client_adapter(py::module& m);
void bind_client_state_manager(py::module& m);
void bind_client_navigation_request(py::module& m);
void bind_client_action_request(py::module& m);
void bind_client_execution(py::module& m);

void bind_rmf_migration_fleet(py::module& m);
void bind_rmf_migration_robot(py::module& m);
void bind_rmf_migration_destination(py::module& m);
void bind_rmf_migration_action(py::module& m);
void bind_rmf_migration_command_execution(py::module& m);
void bind_rmf_migration_robot_update_handle(py::module& m);

PYBIND11_MODULE(vda5050_core_python, m)
{
  m.doc() = "VDA5050 Core Python bindings";

  auto m_client = m.def_submodule("client", "Native VDA5050 adapter API");

  bind_client_adapter(m_client);
  bind_client_state_manager(m_client);
  bind_client_navigation_request(m_client);
  bind_client_action_request(m_client);
  bind_client_execution(m_client);

  auto m_rmf_migration =
    m.def_submodule("rmf_migration", "Open-RMF style migration API");

  bind_rmf_migration_fleet(m_rmf_migration);
  bind_rmf_migration_robot(m_rmf_migration);
  bind_rmf_migration_destination(m_rmf_migration);
  bind_rmf_migration_action(m_rmf_migration);
  bind_rmf_migration_command_execution(m_rmf_migration);
  bind_rmf_migration_robot_update_handle(m_rmf_migration);
}
