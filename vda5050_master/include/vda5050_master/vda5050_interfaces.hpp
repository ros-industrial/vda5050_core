/**
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

#ifndef VDA5050_MASTER__VDA5050_INTERFACES_HPP_
#define VDA5050_MASTER__VDA5050_INTERFACES_HPP_

#include "vda5050_msgs/json_utils/action.hpp"
#include "vda5050_msgs/json_utils/connection.hpp"
#include "vda5050_msgs/json_utils/factsheet.hpp"
#include "vda5050_msgs/json_utils/instant_actions.hpp"
#include "vda5050_msgs/json_utils/state.hpp"
#include "vda5050_msgs/json_utils/visualization.hpp"
#include "vda5050_msgs/msg/action.hpp"
#include "vda5050_msgs/msg/action_parameter.hpp"
#include "vda5050_msgs/msg/action_parameter_factsheet.hpp"
#include "vda5050_msgs/msg/action_parameter_value.hpp"
#include "vda5050_msgs/msg/action_state.hpp"
#include "vda5050_msgs/msg/agv_action.hpp"
#include "vda5050_msgs/msg/agv_geometry.hpp"
#include "vda5050_msgs/msg/agv_position.hpp"
#include "vda5050_msgs/msg/battery_state.hpp"
#include "vda5050_msgs/msg/bounding_box_reference.hpp"
#include "vda5050_msgs/msg/connection.hpp"
#include "vda5050_msgs/msg/control_point.hpp"
#include "vda5050_msgs/msg/edge.hpp"
#include "vda5050_msgs/msg/edge_state.hpp"
#include "vda5050_msgs/msg/envelope2d.hpp"
#include "vda5050_msgs/msg/envelope3d.hpp"
#include "vda5050_msgs/msg/error.hpp"
#include "vda5050_msgs/msg/error_reference.hpp"
#include "vda5050_msgs/msg/factsheet.hpp"
#include "vda5050_msgs/msg/header.hpp"
#include "vda5050_msgs/msg/info.hpp"
#include "vda5050_msgs/msg/info_reference.hpp"
#include "vda5050_msgs/msg/instant_actions.hpp"
#include "vda5050_msgs/msg/load.hpp"
#include "vda5050_msgs/msg/load_dimensions.hpp"
#include "vda5050_msgs/msg/load_set.hpp"
#include "vda5050_msgs/msg/load_specification.hpp"
#include "vda5050_msgs/msg/max_array_lens.hpp"
#include "vda5050_msgs/msg/max_string_lens.hpp"
#include "vda5050_msgs/msg/network.hpp"
#include "vda5050_msgs/msg/node.hpp"
#include "vda5050_msgs/msg/node_position.hpp"
#include "vda5050_msgs/msg/node_state.hpp"
#include "vda5050_msgs/msg/optional_parameters.hpp"
#include "vda5050_msgs/msg/order.hpp"
#include "vda5050_msgs/msg/physical_parameters.hpp"
#include "vda5050_msgs/msg/polygon_point.hpp"
#include "vda5050_msgs/msg/position.hpp"
#include "vda5050_msgs/msg/protocol_features.hpp"
#include "vda5050_msgs/msg/protocol_limits.hpp"
#include "vda5050_msgs/msg/safety_state.hpp"
#include "vda5050_msgs/msg/state.hpp"
#include "vda5050_msgs/msg/timing.hpp"
#include "vda5050_msgs/msg/trajectory.hpp"
#include "vda5050_msgs/msg/type_specification.hpp"
#include "vda5050_msgs/msg/vehicle_config.hpp"
#include "vda5050_msgs/msg/velocity.hpp"
#include "vda5050_msgs/msg/version_info.hpp"
#include "vda5050_msgs/msg/visualization.hpp"
#include "vda5050_msgs/msg/wheel_definition.hpp"

using vda5050_msgs::msg::Action;
using vda5050_msgs::msg::ActionParameter;
using vda5050_msgs::msg::ActionParameterFactsheet;
using vda5050_msgs::msg::ActionParameterValue;
using vda5050_msgs::msg::ActionState;
using vda5050_msgs::msg::AGVAction;
using vda5050_msgs::msg::AGVGeometry;
using vda5050_msgs::msg::AGVPosition;
using vda5050_msgs::msg::BatteryState;
using vda5050_msgs::msg::BoundingBoxReference;
using vda5050_msgs::msg::Connection;
using vda5050_msgs::msg::ControlPoint;
using vda5050_msgs::msg::Edge;
using vda5050_msgs::msg::EdgeState;
using vda5050_msgs::msg::Envelope2d;
using vda5050_msgs::msg::Envelope3d;
using vda5050_msgs::msg::Error;
using vda5050_msgs::msg::ErrorReference;
using vda5050_msgs::msg::Factsheet;
using vda5050_msgs::msg::Header;
using vda5050_msgs::msg::Info;
using vda5050_msgs::msg::InfoReference;
using vda5050_msgs::msg::InstantActions;
using vda5050_msgs::msg::Load;
using vda5050_msgs::msg::LoadDimensions;
using vda5050_msgs::msg::LoadSet;
using vda5050_msgs::msg::LoadSpecification;
using vda5050_msgs::msg::MaxArrayLens;
using vda5050_msgs::msg::MaxStringLens;
using vda5050_msgs::msg::Network;
using vda5050_msgs::msg::Node;
using vda5050_msgs::msg::NodePosition;
using vda5050_msgs::msg::NodeState;
using vda5050_msgs::msg::OptionalParameters;
using vda5050_msgs::msg::Order;
using vda5050_msgs::msg::PhysicalParameters;
using vda5050_msgs::msg::PolygonPoint;
using vda5050_msgs::msg::Position;
using vda5050_msgs::msg::ProtocolFeatures;
using vda5050_msgs::msg::ProtocolLimits;
using vda5050_msgs::msg::SafetyState;
using vda5050_msgs::msg::State;
using vda5050_msgs::msg::Timing;
using vda5050_msgs::msg::Trajectory;
using vda5050_msgs::msg::TypeSpecification;
using vda5050_msgs::msg::VehicleConfig;
using vda5050_msgs::msg::Velocity;
using vda5050_msgs::msg::VersionInfo;
using vda5050_msgs::msg::Visualization;
using vda5050_msgs::msg::WheelDefinition;
#endif  // VDA5050_MASTER__VDA5050_INTERFACES_HPP_
