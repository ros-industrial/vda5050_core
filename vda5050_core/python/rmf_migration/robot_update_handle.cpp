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

#include <memory>

#include "vda5050_core/client/adapter/state_manager.hpp"
#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/velocity.hpp"

namespace py = pybind11;

using vda5050_core::client::adapter::StateManager;
using vda5050_core::types::AGVPosition;
using vda5050_core::types::Velocity;

class RobotUpdateHandle : public std::enable_shared_from_this<RobotUpdateHandle>
{
public:
  explicit RobotUpdateHandle(std::shared_ptr<StateManager> state_manager)
  : state_manager_(std::move(state_manager))
  {
    // Nothing to do here ...
  }

  void set_position(double x, double y, double theta, const std::string& map_id)
  {
    AGVPosition position;

    position.x = x;
    position.y = y;
    position.theta = theta;
    position.map_id = map_id;

    state_manager_->set_agv_position(position);
  }

  void set_velocity(double vx, double vy, double omega)
  {
    Velocity velocity;

    velocity.vx = vx;
    velocity.vy = vy;
    velocity.omega = omega;

    state_manager_->set_velocity(velocity);
  }

  void set_battery_soc(double soc)
  {
    auto state = state_manager_->state();

    auto battery = state.battery_state;
    battery.battery_charge = soc;

    state_manager_->set_battery_state(battery);
  }

  void set_driving(bool driving)
  {
    state_manager_->set_driving(driving);
  }

  void set_paused(bool paused)
  {
    state_manager_->set_paused(paused);
  }

  void request_new_base(bool request)
  {
    state_manager_->set_new_base_request(request);
  }

private:
  std::shared_ptr<StateManager> state_manager_;
};

void bind_rmf_migration_robot_update_handle(py::module& m)
{
  py::class_<RobotUpdateHandle, std::shared_ptr<RobotUpdateHandle>>(
    m, "RobotUpdateHandle")
    .def(
      "set_position", &RobotUpdateHandle::set_position, py::arg("x"),
      py::arg("y"), py::arg("theta"), py::arg("map"))
    .def(
      "set_velocity", &RobotUpdateHandle::set_velocity, py::arg("vx"),
      py ::arg("vy"), py::arg("omega"))
    .def("set_battery_soc", &RobotUpdateHandle::set_battery_soc)
    .def("set_driving", &RobotUpdateHandle::set_driving)
    .def("set_paused", &RobotUpdateHandle::set_paused)
    .def("request_new_base", &RobotUpdateHandle::request_new_base);
}
