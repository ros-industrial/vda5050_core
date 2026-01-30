/*
 * Copyright (C) 2025 ROS-Industrial Consortium Asia Pacific
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

#ifndef VDA5050_MASTER__STANDARD_NAMES_HPP_
#define VDA5050_MASTER__STANDARD_NAMES_HPP_

#include <string>

namespace vda5050_master {
const std::string Version = "v2";                           // NOLINT
const std::string InterfaceName = "rmf2";                   // NOLINT
const std::string ConnectionTopic = "connection";           // NOLINT
const std::string FactsheetTopic = "factsheet";             // NOLINT
const std::string OrderTopic = "order";                     // NOLINT
const std::string StateTopic = "state";                     // NOLINT
const std::string InstantActionsTopic = "instant_actions";  // NOLINT
const std::string VisualizationTopic = "visualization";     // NOLINT

const int ConnectionQos = 1;
const int FactsheetQos = 0;
const int OrderQos = 0;
const int StateQos = 0;
const int VisualizationQos = 0;
const int InstantActionsQos = 0;

const int ConnectionHeartbeatInterval = 15;  // seconds
const int StateHeartbeatInterval = 30;       // seconds
}  // namespace vda5050_master

#endif  // VDA5050_MASTER__STANDARD_NAMES_HPP_
