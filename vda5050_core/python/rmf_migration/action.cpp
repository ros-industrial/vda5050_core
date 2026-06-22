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

#include <string>

#include "vda5050_core/client/adapter/action_request.hpp"

namespace py = pybind11;

using vda5050_core::client::adapter::ActionRequest;

class Action
{
public:
  std::string action_id;
  std::string action_type;
  std::string action_description;
};

void bind_rmf_migration_action(py::module& m)
{
  py::class_<Action>(m, "Action")
    .def_readonly("action_id", &Action::action_id)
    .def_readonly("action_type", &Action::action_type)
    .def_readonly("action_description", &Action::action_description);
}
