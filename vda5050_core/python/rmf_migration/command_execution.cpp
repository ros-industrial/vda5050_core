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

#include "vda5050_core/client/adapter/execution.hpp"

namespace py = pybind11;

using vda5050_core::client::adapter::Execution;

class CommandExecution
{
public:
  explicit CommandExecution(std::shared_ptr<Execution> execution)
  : execution_(execution)
  {
    // Nothing to do here ...
  }

  void finished()
  {
    execution_->finished();
  }

  void failed(const std::string& reason)
  {
    execution_->failed(reason);
  }

private:
  std::shared_ptr<Execution> execution_;
};

void bind_rmf_migration_command_execution(py::module& m)
{
  py::class_<CommandExecution, std::shared_ptr<CommandExecution>>(
    m, "CommandExecution")
    .def("finished", &CommandExecution::finished)
    .def("failed", &CommandExecution::failed);
}
