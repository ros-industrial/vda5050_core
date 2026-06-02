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

#ifndef VDA5050_CORE__MASTER__LAYOUT__VEHICLE_TYPE_NODE_PROPERTY_HPP_
#define VDA5050_CORE__MASTER__LAYOUT__VEHICLE_TYPE_NODE_PROPERTY_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/master/layout/layout_action.hpp"

namespace vda5050_core::master::layout {

struct VehicleTypeNodeProperty
{
  std::string vehicle_type_id;
  std::optional<double> theta;  ///< [rad], range [-Pi..Pi]
  std::optional<std::vector<Action>> actions;
};

}  // namespace vda5050_core::master::layout

#endif  // VDA5050_CORE__MASTER__LAYOUT__VEHICLE_TYPE_NODE_PROPERTY_HPP_
