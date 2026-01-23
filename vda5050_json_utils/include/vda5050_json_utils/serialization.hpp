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

#ifndef VDA5050_JSON_UTILS__SERIALIZATION_HPP_
#define VDA5050_JSON_UTILS__SERIALIZATION_HPP_

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include <vda5050_types/action.hpp>
#include <vda5050_types/action_parameter.hpp>
#include <vda5050_types/action_state.hpp>
#include <vda5050_types/agv_position.hpp>
#include <vda5050_types/battery_state.hpp>
#include <vda5050_types/bounding_box_reference.hpp>
#include <vda5050_types/connection.hpp>
#include <vda5050_types/control_point.hpp>
#include <vda5050_types/edge.hpp>
#include <vda5050_types/edge_state.hpp>
#include <vda5050_types/error.hpp>
#include <vda5050_types/error_reference.hpp>
#include <vda5050_types/factsheet.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/info.hpp>
#include <vda5050_types/info_reference.hpp>
#include <vda5050_types/instant_actions.hpp>
#include <vda5050_types/load.hpp>
#include <vda5050_types/node.hpp>
#include <vda5050_types/node_position.hpp>
#include <vda5050_types/node_state.hpp>
#include <vda5050_types/order.hpp>
#include <vda5050_types/safety_state.hpp>
#include <vda5050_types/state.hpp>
#include <vda5050_types/trajectory.hpp>
#include <vda5050_types/velocity.hpp>
#include <vda5050_types/visualization.hpp>

#ifdef ENABLE_ROS2
#include <vda5050_interfaces/msg/action_state.hpp>
#include <vda5050_interfaces/msg/agv_position.hpp>
#include <vda5050_interfaces/msg/battery_state.hpp>
#include <vda5050_interfaces/msg/bounding_box_reference.hpp>
#include <vda5050_interfaces/msg/connection.hpp>
#include <vda5050_interfaces/msg/edge_state.hpp>
#include <vda5050_interfaces/msg/error.hpp>
#include <vda5050_interfaces/msg/error_reference.hpp>
#include <vda5050_interfaces/msg/factsheet.hpp>
#include <vda5050_interfaces/msg/header.hpp>
#include <vda5050_interfaces/msg/info.hpp>
#include <vda5050_interfaces/msg/info_reference.hpp>
#include <vda5050_interfaces/msg/instant_actions.hpp>
#include <vda5050_interfaces/msg/load.hpp>
#include <vda5050_interfaces/msg/node_position.hpp>
#include <vda5050_interfaces/msg/node_state.hpp>
#include <vda5050_interfaces/msg/order.hpp>
#include <vda5050_interfaces/msg/safety_state.hpp>
#include <vda5050_interfaces/msg/state.hpp>
#include <vda5050_interfaces/msg/trajectory.hpp>
#include <vda5050_interfaces/msg/velocity.hpp>
#include <vda5050_interfaces/msg/visualization.hpp>
#endif  // ENABLE_ROS2

#include "traits.hpp"

namespace vda5050_types {

namespace header_detail {

//=============================================================================
template <typename HeaderT>
void to_json(nlohmann::json& j, const HeaderT& msg)
{
  using vda5050_json_utils::timestamp_traits;

  j = nlohmann::json{
    {"headerId", msg.header_id},
    {"timestamp",
     timestamp_traits<decltype(msg.timestamp)>::to_string(msg.timestamp)},
    {"version", msg.version},
    {"manufacturer", msg.manufacturer},
    {"serialNumber", msg.serial_number}};
}

//=============================================================================
template <typename HeaderT>
void from_json(const nlohmann::json& j, HeaderT& msg)
{
  using vda5050_json_utils::timestamp_traits;

  msg.header_id = j.at("headerId").get<uint32_t>();
  msg.timestamp = timestamp_traits<decltype(msg.timestamp)>::from_string(
    j.at("timestamp").get<std::string>());
  msg.version = j.at("version").get<std::string>();
  msg.manufacturer = j.at("manufacturer").get<std::string>();
  msg.serial_number = j.at("serialNumber").get<std::string>();
}

}  // namespace header_detail

namespace connection_detail {

//=============================================================================
template <typename ConnectionT>
void to_json(nlohmann::json& j, const ConnectionT& msg)
{
  using vda5050_json_utils::connection_state_traits;

  to_json(j, msg.header);

  j["connectionState"] =
    connection_state_traits<decltype(msg.connection_state)>::to_string(
      msg.connection_state);
}

//=============================================================================
template <typename ConnectionT>
void from_json(const nlohmann::json& j, ConnectionT& msg)
{
  using vda5050_json_utils::connection_state_traits;

  from_json(j, msg.header);

  msg.connection_state =
    connection_state_traits<decltype(msg.connection_state)>::from_string(
      j.at("connectionState").get<std::string>());
}

}  // namespace connection_detail

namespace node_position_detail {

//=============================================================================
template <typename NodePositionT>
void to_json(nlohmann::json& j, const NodePositionT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using theta_trait = optional_field_traits<decltype(msg.theta)>;
  using allowed_deviation_x_y_trait =
    optional_field_traits<decltype(msg.allowed_deviation_x_y)>;
  using allowed_deviation_theta_trait =
    optional_field_traits<decltype(msg.allowed_deviation_theta)>;
  using map_description_trait =
    optional_field_traits<decltype(msg.map_description)>;

  j["x"] = msg.x;
  j["y"] = msg.y;
  j["mapId"] = msg.map_id;

  if (theta_trait::has_value(msg.theta))
  {
    j["theta"] = theta_trait::get(msg.theta);
  }

  if (allowed_deviation_x_y_trait::has_value(msg.allowed_deviation_x_y))
  {
    j["allowedDeviationXY"] =
      allowed_deviation_x_y_trait::get(msg.allowed_deviation_x_y);
  }

  if (allowed_deviation_theta_trait::has_value(msg.allowed_deviation_theta))
  {
    j["allowedDeviationTheta"] =
      allowed_deviation_theta_trait::get(msg.allowed_deviation_theta);
  }

  if (map_description_trait::has_value(msg.map_description))
  {
    j["mapDescription"] = map_description_trait::get(msg.map_description);
  }
}

//=============================================================================
template <typename NodePositionT>
void from_json(const nlohmann::json& j, NodePositionT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using theta_trait = optional_field_traits<decltype(msg.theta)>;
  using allowed_deviation_x_y_trait =
    optional_field_traits<decltype(msg.allowed_deviation_x_y)>;
  using allowed_deviation_theta_trait =
    optional_field_traits<decltype(msg.allowed_deviation_theta)>;
  using map_description_trait =
    optional_field_traits<decltype(msg.map_description)>;

  msg.x = j.at("x").get<double>();
  msg.y = j.at("y").get<double>();
  msg.map_id = j.at("mapId").get<std::string>();

  if (j.contains("theta"))
  {
    theta_trait::set(msg.theta, j.at("theta").get<double>());
  }

  if (j.contains("allowedDeviationXY"))
  {
    allowed_deviation_x_y_trait::set(
      msg.allowed_deviation_x_y, j.at("allowedDeviationXY").get<double>());
  }

  if (j.contains("allowedDeviationTheta"))
  {
    allowed_deviation_theta_trait::set(
      msg.allowed_deviation_theta, j.at("allowedDeviationTheta").get<double>());
  }

  if (j.contains("mapDescription"))
  {
    map_description_trait::set(
      msg.map_description, j.at("mapDescription").get<std::string>());
  }
}

}  // namespace node_position_detail

namespace node_state_detail {

//=============================================================================
template <typename NodeStateT>
void to_json(nlohmann::json& j, const NodeStateT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using node_description_trait =
    optional_field_traits<decltype(msg.node_description)>;
  using node_position_trait =
    optional_field_traits<decltype(msg.node_position)>;

  j["nodeId"] = msg.node_id;
  j["sequenceId"] = msg.sequence_id;
  j["released"] = msg.released;

  if (node_description_trait::has_value(msg.node_description))
  {
    j["nodeDescription"] = node_description_trait::get(msg.node_description);
  }

  if (node_position_trait::has_value(msg.node_position))
  {
    j["nodePosition"] = node_position_trait::get(msg.node_position);
  }
}

//=============================================================================
template <typename NodeStateT>
void from_json(const nlohmann::json& j, NodeStateT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using node_description_trait =
    optional_field_traits<decltype(msg.node_description)>;
  using node_position_trait =
    optional_field_traits<decltype(msg.node_position)>;

  msg.node_id = j.at("nodeId").get<std::string>();
  msg.sequence_id = j.at("sequenceId").get<uint32_t>();
  msg.released = j.at("released").get<bool>();

  if (j.contains("nodeDescription"))
  {
    node_description_trait::set(
      msg.node_description, j.at("nodeDescription").get<std::string>());
  }

  if (j.contains("nodePosition"))
  {
    node_position_trait::set(msg.node_position, j.at("nodePosition"));
  }
}

}  // namespace node_state_detail

namespace control_point_detail {

//=============================================================================
template <typename ControlPointT>
void to_json(nlohmann::json& j, const ControlPointT& msg)
{
  j["x"] = msg.x;
  j["y"] = msg.y;
  j["weight"] = msg.weight;
}

//=============================================================================
template <typename ControlPointT>
void from_json(const nlohmann::json& j, ControlPointT& msg)
{
  msg.x = j.at("x").get<double>();
  msg.y = j.at("y").get<double>();

  if (j.contains("weight"))
  {
    msg.weight = j.at("weight").get<double>();
  }
}

}  // namespace control_point_detail

namespace trajectory_detail {

//=============================================================================
template <typename TrajectoryT>
void to_json(nlohmann::json& j, const TrajectoryT& msg)
{
  j["knotVector"] = msg.knot_vector;
  j["controlPoints"] = msg.control_points;
  j["degree"] = msg.degree;
}

//=============================================================================
template <typename TrajectoryT>
void from_json(const nlohmann::json& j, TrajectoryT& msg)
{
  msg.knot_vector = j.at("knotVector").get<std::vector<double>>();
  msg.control_points = j.at("controlPoints");
  msg.degree = j.at("degree").get<double>();
}

}  // namespace trajectory_detail

namespace edge_state_detail {

//=============================================================================
template <typename EdgeStateT>
void to_json(nlohmann::json& j, const EdgeStateT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using edge_description_trait =
    optional_field_traits<decltype(msg.edge_description)>;
  using trajectory_trait = optional_field_traits<decltype(msg.trajectory)>;

  j["edgeId"] = msg.edge_id;
  j["sequenceId"] = msg.sequence_id;
  j["released"] = msg.released;

  if (edge_description_trait::has_value(msg.edge_description))
  {
    j["edgeDescription"] = edge_description_trait::get(msg.edge_description);
  }

  if (trajectory_trait::has_value(msg.trajectory))
  {
    j["trajectory"] = trajectory_trait::get(msg.trajectory);
  }
}

//=============================================================================
template <typename EdgeStateT>
void from_json(const nlohmann::json& j, EdgeStateT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using edge_description_trait =
    optional_field_traits<decltype(msg.edge_description)>;
  using trajectory_trait = optional_field_traits<decltype(msg.trajectory)>;

  msg.edge_id = j.at("edgeId").get<std::string>();
  msg.sequence_id = j.at("sequenceId").get<uint32_t>();
  msg.released = j.at("released").get<bool>();

  if (j.contains("edgeDescription"))
  {
    edge_description_trait::set(
      msg.edge_description, j.at("edgeDescription").get<std::string>());
  }

  if (j.contains("trajectory"))
  {
    trajectory_trait::set(msg.trajectory, j.at("trajectory"));
  }
}

}  // namespace edge_state_detail

namespace action_state_detail {

//=============================================================================
template <typename ActionStateT>
void to_json(nlohmann::json& j, const ActionStateT& msg)
{
  using vda5050_json_utils::action_status_traits;
  using vda5050_json_utils::optional_field_traits;

  using action_type_trait = optional_field_traits<decltype(msg.action_type)>;
  using action_description_trait =
    optional_field_traits<decltype(msg.action_description)>;
  using result_description_trait =
    optional_field_traits<decltype(msg.result_description)>;

  j["actionId"] = msg.action_id;

  j["actionStatus"] =
    action_status_traits<decltype(msg.action_status)>::to_string(
      msg.action_status);

  if (action_type_trait::has_value(msg.action_type))
  {
    j["actionType"] = action_type_trait::get(msg.action_type);
  }

  if (action_description_trait::has_value(msg.action_description))
  {
    j["actionDescription"] =
      action_description_trait::get(msg.action_description);
  }

  if (result_description_trait::has_value(msg.result_description))
  {
    j["resultDescription"] =
      result_description_trait::get(msg.result_description);
  }
}

//=============================================================================
template <typename ActionStateT>
void from_json(const nlohmann::json& j, ActionStateT& msg)
{
  using vda5050_json_utils::action_status_traits;
  using vda5050_json_utils::optional_field_traits;

  using action_type_trait = optional_field_traits<decltype(msg.action_type)>;
  using action_description_trait =
    optional_field_traits<decltype(msg.action_description)>;
  using result_description_trait =
    optional_field_traits<decltype(msg.result_description)>;

  msg.action_id = j.at("actionId").get<std::string>();

  msg.action_status =
    action_status_traits<decltype(msg.action_status)>::from_string(
      j.at("actionStatus").get<std::string>());

  if (j.contains("actionType"))
  {
    action_type_trait::set(
      msg.action_type, j.at("actionType").get<std::string>());
  }

  if (j.contains("actionDescription"))
  {
    action_description_trait::set(
      msg.action_description, j.at("actionDescription").get<std::string>());
  }

  if (j.contains("resultDescription"))
  {
    result_description_trait::set(
      msg.result_description, j.at("resultDescription").get<std::string>());
  }
}

}  // namespace action_state_detail

namespace error_reference_detail {

//=============================================================================
template <typename ErrorReferenceT>
void to_json(nlohmann::json& j, const ErrorReferenceT& msg)
{
  j["referenceKey"] = msg.reference_key;
  j["referenceValue"] = msg.reference_value;
}

//=============================================================================
template <typename ErrorReferenceT>
void from_json(const nlohmann::json& j, ErrorReferenceT& msg)
{
  msg.reference_key = j.at("referenceKey").get<std::string>();
  msg.reference_value = j.at("referenceValue").get<std::string>();
}

}  // namespace error_reference_detail

namespace error_detail {

//=============================================================================
template <typename ErrorT>
void to_json(nlohmann::json& j, const ErrorT& msg)
{
  using vda5050_json_utils::error_level_traits;
  using vda5050_json_utils::optional_field_traits;

  using error_references_trait =
    optional_field_traits<decltype(msg.error_references)>;
  using error_description_trait =
    optional_field_traits<decltype(msg.error_description)>;

  j["errorType"] = msg.error_type;

  j["errorLevel"] =
    error_level_traits<decltype(msg.error_level)>::to_string(msg.error_level);

  if (error_references_trait::has_value(msg.error_references))
  {
    j["errorReferences"] = error_references_trait::get(msg.error_references);
  }

  if (error_description_trait::has_value(msg.error_description))
  {
    j["errorDescription"] = error_description_trait::get(msg.error_description);
  }
}

//=============================================================================
template <typename ErrorT>
void from_json(const nlohmann::json& j, ErrorT& msg)
{
  using vda5050_json_utils::error_level_traits;
  using vda5050_json_utils::optional_field_traits;

  using error_references_trait =
    optional_field_traits<decltype(msg.error_references)>;
  using error_description_trait =
    optional_field_traits<decltype(msg.error_description)>;

  msg.error_type = j.at("errorType").get<std::string>();

  msg.error_level = error_level_traits<decltype(msg.error_level)>::from_string(
    j.at("errorLevel").get<std::string>());

  if (j.contains("errorReferences"))
  {
    error_references_trait::set(msg.error_references, j.at("errorReferences"));
  }

  if (j.contains("errorDescription"))
  {
    error_description_trait::set(
      msg.error_description, j.at("errorDescription").get<std::string>());
  }
}

}  // namespace error_detail

namespace battery_state_detail {

//=============================================================================
template <typename BatteryStateT>
void to_json(nlohmann::json& j, const BatteryStateT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using battery_voltage_trait =
    optional_field_traits<decltype(msg.battery_voltage)>;
  using battery_health_trait =
    optional_field_traits<decltype(msg.battery_health)>;
  using reach_trait = optional_field_traits<decltype(msg.reach)>;

  j["batteryCharge"] = msg.battery_charge;
  j["charging"] = msg.charging;

  if (battery_voltage_trait::has_value(msg.battery_voltage))
  {
    j["batteryVoltage"] = battery_voltage_trait::get(msg.battery_voltage);
  }

  if (battery_health_trait::has_value(msg.battery_health))
  {
    j["batteryHealth"] = battery_health_trait::get(msg.battery_health);
  }

  if (reach_trait::has_value(msg.reach))
  {
    j["reach"] = reach_trait::get(msg.reach);
  }
}

//=============================================================================
template <typename BatteryStateT>
void from_json(const nlohmann::json& j, BatteryStateT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using battery_voltage_trait =
    optional_field_traits<decltype(msg.battery_voltage)>;
  using battery_health_trait =
    optional_field_traits<decltype(msg.battery_health)>;
  using reach_trait = optional_field_traits<decltype(msg.reach)>;

  msg.battery_charge = j.at("batteryCharge").get<double>();
  msg.charging = j.at("charging").get<bool>();

  if (j.contains("batteryVoltage"))
  {
    battery_voltage_trait::set(
      msg.battery_voltage, j.at("batteryVoltage").get<double>());
  }

  if (j.contains("batteryHealth"))
  {
    battery_health_trait::set(
      msg.battery_health, j.at("batteryHealth").get<int8_t>());
  }

  if (j.contains("reach"))
  {
    reach_trait::set(msg.reach, j.at("reach").get<uint32_t>());
  }
}

}  // namespace battery_state_detail

namespace safety_state_detail {

//=============================================================================
template <typename SafetyStateT>
void to_json(nlohmann::json& j, const SafetyStateT& msg)
{
  using vda5050_json_utils::e_stop_traits;

  j["eStop"] = e_stop_traits<decltype(msg.e_stop)>::to_string(msg.e_stop);

  j["fieldViolation"] = msg.field_violation;
}

//=============================================================================
template <typename SafetyStateT>
void from_json(const nlohmann::json& j, SafetyStateT& msg)
{
  using vda5050_json_utils::e_stop_traits;

  msg.e_stop = e_stop_traits<decltype(msg.e_stop)>::from_string(
    j.at("eStop").get<std::string>());

  msg.field_violation = j.at("fieldViolation").get<bool>();
}

}  // namespace safety_state_detail

namespace agv_position_detail {

//=============================================================================
template <typename AGVPositionT>
void to_json(nlohmann::json& j, const AGVPositionT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using map_description_trait =
    optional_field_traits<decltype(msg.map_description)>;
  using localization_score_trait =
    optional_field_traits<decltype(msg.localization_score)>;
  using deviation_range_trait =
    optional_field_traits<decltype(msg.deviation_range)>;

  j["x"] = msg.x;
  j["y"] = msg.y;
  j["theta"] = msg.theta;
  j["mapId"] = msg.map_id;
  j["positionInitialized"] = msg.position_initialized;

  if (map_description_trait::has_value(msg.map_description))
  {
    j["mapDescription"] = map_description_trait::get(msg.map_description);
  }

  if (localization_score_trait::has_value(msg.localization_score))
  {
    j["localizationScore"] =
      localization_score_trait::get(msg.localization_score);
  }

  if (deviation_range_trait::has_value(msg.deviation_range))
  {
    j["deviationRange"] = deviation_range_trait::get(msg.deviation_range);
  }
}

//=============================================================================
template <typename AGVPositionT>
void from_json(const nlohmann::json& j, AGVPositionT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using map_description_trait =
    optional_field_traits<decltype(msg.map_description)>;
  using localization_score_trait =
    optional_field_traits<decltype(msg.localization_score)>;
  using deviation_range_trait =
    optional_field_traits<decltype(msg.deviation_range)>;

  msg.x = j.at("x").get<double>();
  msg.y = j.at("y").get<double>();
  msg.theta = j.at("theta").get<double>();
  msg.map_id = j.at("mapId").get<std::string>();
  msg.position_initialized = j.at("positionInitialized").get<bool>();

  if (j.contains("mapDescription"))
  {
    map_description_trait::set(
      msg.map_description, j.at("mapDescription").get<std::string>());
  }

  if (j.contains("localizationScore"))
  {
    localization_score_trait::set(
      msg.localization_score, j.at("localizationScore").get<double>());
  }

  if (j.contains("deviationRange"))
  {
    deviation_range_trait::set(
      msg.deviation_range, j.at("deviationRange").get<double>());
  }
}

}  // namespace agv_position_detail

namespace velocity_detail {

//=============================================================================
template <typename VelocityT>
void to_json(nlohmann::json& j, const VelocityT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using vx_trait = optional_field_traits<decltype(msg.vx)>;
  using vy_trait = optional_field_traits<decltype(msg.vy)>;
  using omega_trait = optional_field_traits<decltype(msg.omega)>;

  if (vx_trait::has_value(msg.vx))
  {
    j["vx"] = vx_trait::get(msg.vx);
  }

  if (vy_trait::has_value(msg.vy))
  {
    j["vy"] = vy_trait::get(msg.vy);
  }

  if (omega_trait::has_value(msg.omega))
  {
    j["omega"] = omega_trait::get(msg.omega);
  }
}

//=============================================================================
template <typename VelocityT>
void from_json(const nlohmann::json& j, VelocityT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using vx_trait = optional_field_traits<decltype(msg.vx)>;
  using vy_trait = optional_field_traits<decltype(msg.vy)>;
  using omega_trait = optional_field_traits<decltype(msg.omega)>;

  if (j.contains("vx"))
  {
    vx_trait::set(msg.vx, j.at("vx").get<double>());
  }

  if (j.contains("vy"))
  {
    vy_trait::set(msg.vy, j.at("vy").get<double>());
  }

  if (j.contains("omega"))
  {
    omega_trait::set(msg.omega, j.at("omega").get<double>());
  }
}

}  // namespace velocity_detail

namespace bounding_box_reference_detail {

//=============================================================================
template <typename BoundingBoxReferenceT>
void to_json(nlohmann::json& j, const BoundingBoxReferenceT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using theta_trait = optional_field_traits<decltype(msg.theta)>;

  j["x"] = msg.x;
  j["y"] = msg.y;
  j["z"] = msg.z;

  if (theta_trait::has_value(msg.theta))
  {
    j["theta"] = theta_trait::get(msg.theta);
  }
}

//=============================================================================
template <typename BoundingBoxReferenceT>
void from_json(const nlohmann::json& j, BoundingBoxReferenceT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using theta_trait = optional_field_traits<decltype(msg.theta)>;

  msg.x = j.at("x").get<double>();
  msg.y = j.at("y").get<double>();
  msg.z = j.at("z").get<double>();

  if (j.contains("theta"))
  {
    theta_trait::set(msg.theta, j.at("theta").get<double>());
  }
}

}  // namespace bounding_box_reference_detail

namespace load_dimensions_detail {

//=============================================================================
template <typename LoadDimensionsT>
void to_json(nlohmann::json& j, const LoadDimensionsT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using height_trait = optional_field_traits<decltype(msg.height)>;

  j["length"] = msg.length;
  j["width"] = msg.width;

  if (height_trait::has_value(msg.height))
  {
    j["height"] = height_trait::get(msg.height);
  }
}

//=============================================================================
template <typename LoadDimensionsT>
void from_json(const nlohmann::json& j, LoadDimensionsT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using height_trait = optional_field_traits<decltype(msg.height)>;

  msg.length = j.at("length").get<double>();
  msg.width = j.at("width").get<double>();

  if (j.contains("height"))
  {
    height_trait::set(msg.height, j.at("height").get<double>());
  }
}

}  // namespace load_dimensions_detail

namespace load_detail {

//=============================================================================
template <typename LoadT>
void to_json(nlohmann::json& j, const LoadT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using load_id_trait = optional_field_traits<decltype(msg.load_id)>;
  using load_type_trait = optional_field_traits<decltype(msg.load_type)>;
  using load_position_trait =
    optional_field_traits<decltype(msg.load_position)>;
  using bounding_box_reference_trait =
    optional_field_traits<decltype(msg.bounding_box_reference)>;
  using load_dimensions_trait =
    optional_field_traits<decltype(msg.load_dimensions)>;
  using weight_trait = optional_field_traits<decltype(msg.weight)>;

  if (load_id_trait::has_value(msg.load_id))
  {
    j["loadId"] = load_id_trait::get(msg.load_id);
  }

  if (load_type_trait::has_value(msg.load_type))
  {
    j["loadType"] = load_type_trait::get(msg.load_type);
  }

  if (load_position_trait::has_value(msg.load_position))
  {
    j["loadPosition"] = load_position_trait::get(msg.load_position);
  }

  if (bounding_box_reference_trait::has_value(msg.bounding_box_reference))
  {
    j["boundingBoxReference"] =
      bounding_box_reference_trait::get(msg.bounding_box_reference);
  }

  if (load_dimensions_trait::has_value(msg.load_dimensions))
  {
    j["loadDimensions"] = load_dimensions_trait::get(msg.load_dimensions);
  }

  if (weight_trait::has_value(msg.weight))
  {
    j["weight"] = weight_trait::get(msg.weight);
  }
}

//=============================================================================
template <typename LoadT>
void from_json(const nlohmann::json& j, LoadT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using load_id_trait = optional_field_traits<decltype(msg.load_id)>;
  using load_type_trait = optional_field_traits<decltype(msg.load_type)>;
  using load_position_trait =
    optional_field_traits<decltype(msg.load_position)>;
  using bounding_box_reference_trait =
    optional_field_traits<decltype(msg.bounding_box_reference)>;
  using load_dimensions_trait =
    optional_field_traits<decltype(msg.load_dimensions)>;
  using weight_trait = optional_field_traits<decltype(msg.weight)>;

  if (j.contains("loadId"))
  {
    load_id_trait::set(msg.load_id, j.at("loadId").get<std::string>());
  }

  if (j.contains("loadType"))
  {
    load_type_trait::set(msg.load_type, j.at("loadType").get<std::string>());
  }

  if (j.contains("loadPosition"))
  {
    load_position_trait::set(
      msg.load_position, j.at("loadPosition").get<std::string>());
  }

  if (j.contains("boundingBoxReference"))
  {
    bounding_box_reference_trait::set(
      msg.bounding_box_reference, j.at("boundingBoxReference"));
  }

  if (j.contains("loadDimensions"))
  {
    load_dimensions_trait::set(msg.load_dimensions, j.at("loadDimensions"));
  }

  if (j.contains("weight"))
  {
    weight_trait::set(msg.weight, j.at("weight").get<double>());
  }
}

}  // namespace load_detail

namespace info_reference_detail {

//=============================================================================
template <typename InfoReferenceT>
void to_json(nlohmann::json& j, const InfoReferenceT& msg)
{
  j["referenceKey"] = msg.reference_key;
  j["referenceValue"] = msg.reference_value;
}

//=============================================================================
template <typename InfoReferenceT>
void from_json(const nlohmann::json& j, InfoReferenceT& msg)
{
  msg.reference_key = j.at("referenceKey").get<std::string>();
  msg.reference_value = j.at("referenceValue").get<std::string>();
}

}  // namespace info_reference_detail

namespace info_detail {

//=============================================================================
template <typename InfoT>
void to_json(nlohmann::json& j, const InfoT& msg)
{
  using vda5050_json_utils::info_level_traits;
  using vda5050_json_utils::optional_field_traits;

  using info_references_trait =
    optional_field_traits<decltype(msg.info_references)>;
  using info_description_trait =
    optional_field_traits<decltype(msg.info_description)>;

  j["infoType"] = msg.info_type;

  j["infoLevel"] =
    info_level_traits<decltype(msg.info_level)>::to_string(msg.info_level);

  if (info_references_trait::has_value(msg.info_references))
  {
    j["infoReferences"] = info_references_trait::get(msg.info_references);
  }

  if (info_description_trait::has_value(msg.info_description))
  {
    j["infoDescription"] = info_description_trait::get(msg.info_description);
  }
}

//=============================================================================
template <typename InfoT>
void from_json(const nlohmann::json& j, InfoT& msg)
{
  using vda5050_json_utils::info_level_traits;
  using vda5050_json_utils::optional_field_traits;

  using info_references_trait =
    optional_field_traits<decltype(msg.info_references)>;
  using info_description_trait =
    optional_field_traits<decltype(msg.info_description)>;

  msg.info_type = j.at("infoType").get<std::string>();

  msg.info_level = info_level_traits<decltype(msg.info_level)>::from_string(
    j.at("infoLevel").get<std::string>());

  if (j.contains("infoReferences"))
  {
    info_references_trait::set(msg.info_references, j.at("infoReferences"));
  }

  if (j.contains("infoDescription"))
  {
    info_description_trait::set(
      msg.info_description, j.at("infoDescription").get<std::string>());
  }
}

}  // namespace info_detail

namespace state_detail {

//=============================================================================
template <typename StateT>
void to_json(nlohmann::json& j, const StateT& msg)
{
  using vda5050_json_utils::operating_mode_traits;
  using vda5050_json_utils::optional_field_traits;

  using zone_set_id_trait = optional_field_traits<decltype(msg.zone_set_id)>;
  using paused_trait = optional_field_traits<decltype(msg.paused)>;
  using new_base_request_trait =
    optional_field_traits<decltype(msg.new_base_request)>;
  using distance_since_last_node_trait =
    optional_field_traits<decltype(msg.distance_since_last_node)>;
  using agv_position_trait = optional_field_traits<decltype(msg.agv_position)>;
  using velocity_trait = optional_field_traits<decltype(msg.velocity)>;
  using loads_trait = optional_field_traits<decltype(msg.loads)>;
  using information_traits = optional_field_traits<decltype(msg.information)>;

  to_json(j, msg.header);

  j["orderId"] = msg.order_id;
  j["orderUpdateId"] = msg.order_update_id;
  j["lastNodeId"] = msg.last_node_id;
  j["lastNodeSequenceId"] = msg.last_node_sequence_id;
  j["driving"] = msg.driving;

  j["operatingMode"] =
    operating_mode_traits<decltype(msg.operating_mode)>::to_string(
      msg.operating_mode);

  j["nodeStates"] = msg.node_states;
  j["edgeStates"] = msg.edge_states;
  j["actionStates"] = msg.action_states;
  j["errors"] = msg.errors;
  j["batteryState"] = msg.battery_state;
  j["safetyState"] = msg.safety_state;

  if (zone_set_id_trait::has_value(msg.zone_set_id))
  {
    j["zoneSetId"] = zone_set_id_trait::get(msg.zone_set_id);
  }

  if (paused_trait::has_value(msg.paused))
  {
    j["paused"] = paused_trait::get(msg.paused);
  }

  if (new_base_request_trait::has_value(msg.new_base_request))
  {
    j["newBaseRequest"] = new_base_request_trait::get(msg.new_base_request);
  }

  if (distance_since_last_node_trait::has_value(msg.distance_since_last_node))
  {
    j["distanceSinceLastNode"] =
      distance_since_last_node_trait::get(msg.distance_since_last_node);
  }

  if (agv_position_trait::has_value(msg.agv_position))
  {
    j["agvPosition"] = agv_position_trait::get(msg.agv_position);
  }

  if (velocity_trait::has_value(msg.velocity))
  {
    j["velocity"] = velocity_trait::get(msg.velocity);
  }

  if (loads_trait::has_value(msg.loads))
  {
    j["loads"] = loads_trait::get(msg.loads);
  }

  if (information_traits::has_value(msg.information))
  {
    j["information"] = information_traits::get(msg.information);
  }
}

//=============================================================================
template <typename StateT>
void from_json(const nlohmann::json& j, StateT& msg)
{
  using vda5050_json_utils::operating_mode_traits;
  using vda5050_json_utils::optional_field_traits;

  using zone_set_id_trait = optional_field_traits<decltype(msg.zone_set_id)>;
  using paused_trait = optional_field_traits<decltype(msg.paused)>;
  using new_base_request_trait =
    optional_field_traits<decltype(msg.new_base_request)>;
  using distance_since_last_node_trait =
    optional_field_traits<decltype(msg.distance_since_last_node)>;
  using agv_position_trait = optional_field_traits<decltype(msg.agv_position)>;
  using velocity_trait = optional_field_traits<decltype(msg.velocity)>;
  using loads_trait = optional_field_traits<decltype(msg.loads)>;
  using information_traits = optional_field_traits<decltype(msg.information)>;

  from_json(j, msg.header);

  msg.order_id = j.at("orderId").get<std::string>();
  msg.order_update_id = j.at("orderUpdateId").get<uint32_t>();
  msg.last_node_id = j.at("lastNodeId").get<std::string>();
  msg.last_node_sequence_id = j.at("lastNodeSequenceId").get<uint32_t>();
  msg.driving = j.at("driving").get<bool>();

  msg.operating_mode =
    operating_mode_traits<decltype(msg.operating_mode)>::from_string(
      j.at("operatingMode").get<std::string>());

  msg.node_states = j.at("nodeStates");
  msg.edge_states = j.at("edgeStates");
  msg.action_states = j.at("actionStates");
  msg.errors = j.at("errors");
  msg.battery_state = j.at("batteryState");
  msg.safety_state = j.at("safetyState");

  if (j.contains("zoneSetId"))
  {
    zone_set_id_trait::set(
      msg.zone_set_id, j.at("zoneSetId").get<std::string>());
  }

  if (j.contains("paused"))
  {
    paused_trait::set(msg.paused, j.at("paused").get<bool>());
  }

  if (j.contains("newBaseRequest"))
  {
    new_base_request_trait::set(
      msg.new_base_request, j.at("newBaseRequest").get<bool>());
  }

  if (j.contains("distanceSinceLastNode"))
  {
    distance_since_last_node_trait::set(
      msg.distance_since_last_node,
      j.at("distanceSinceLastNode").get<double>());
  }

  if (j.contains("agvPosition"))
  {
    agv_position_trait::set(msg.agv_position, j.at("agvPosition"));
  }

  if (j.contains("velocity"))
  {
    velocity_trait::set(msg.velocity, j.at("velocity"));
  }

  if (j.contains("loads"))
  {
    loads_trait::set(msg.loads, j.at("loads"));
  }

  if (j.contains("information"))
  {
    information_traits::set(msg.information, j.at("information"));
  }
}

}  // namespace state_detail

namespace action_parameter_detail {

//=============================================================================
template <typename ActionParameterT>
void to_json(nlohmann::json& j, const ActionParameterT& msg)
{
  j["key"] = msg.key;
  j["value"] = msg.value;
}

//=============================================================================
template <typename ActionParameterT>
void from_json(const nlohmann::json& j, ActionParameterT& msg)
{
  msg.key = j.at("key").get<std::string>();
  msg.value = j.at("value").get<std::string>();
}

}  // namespace action_parameter_detail

namespace action_detail {

//=============================================================================
template <typename ActionT>
void to_json(nlohmann::json& j, const ActionT& msg)
{
  using vda5050_json_utils::blocking_type_traits;
  using vda5050_json_utils::optional_field_traits;

  using action_description_trait =
    optional_field_traits<decltype(msg.action_description)>;
  using action_parameters_trait =
    optional_field_traits<decltype(msg.action_parameters)>;

  j["actionType"] = msg.action_type;
  j["actionId"] = msg.action_id;

  j["blockingType"] =
    blocking_type_traits<decltype(msg.blocking_type)>::to_string(
      msg.blocking_type);

  if (action_description_trait::has_value(msg.action_description))
  {
    j["actionDescription"] =
      action_description_trait::get(msg.action_description);
  }

  if (action_parameters_trait::has_value(msg.action_parameters))
  {
    j["actionParameters"] = action_parameters_trait::get(msg.action_parameters);
  }
}

//=============================================================================
template <typename ActionT>
void from_json(const nlohmann::json& j, ActionT& msg)
{
  using vda5050_json_utils::blocking_type_traits;
  using vda5050_json_utils::optional_field_traits;

  using action_description_trait =
    optional_field_traits<decltype(msg.action_description)>;
  using action_parameters_trait =
    optional_field_traits<decltype(msg.action_parameters)>;

  msg.action_type = j.at("actionType").get<std::string>();
  msg.action_id = j.at("actionId").get<std::string>();
  msg.blocking_type =
    blocking_type_traits<decltype(msg.blocking_type)>::from_string(
      j.at("blockingType").get<std::string>());

  if (j.contains("actionDescription"))
  {
    action_description_trait::set(
      msg.action_description, j.at("actionDescription").get<std::string>());
  }

  if (j.contains("actionParameters"))
  {
    action_parameters_trait::set(
      msg.action_parameters, j.at("actionParameters"));
  }
}

}  // namespace action_detail

namespace node_detail {

//=============================================================================
template <typename NodeT>
void to_json(nlohmann::json& j, const NodeT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using node_position_trait =
    optional_field_traits<decltype(msg.node_position)>;
  using node_description_trait =
    optional_field_traits<decltype(msg.node_description)>;

  j["nodeId"] = msg.node_id;
  j["sequenceId"] = msg.sequence_id;
  j["released"] = msg.released;
  j["actions"] = msg.actions;

  if (node_position_trait::has_value(msg.node_position))
  {
    j["nodePosition"] = node_position_trait::get(msg.node_position);
  }

  if (node_description_trait::has_value(msg.node_description))
  {
    j["nodeDescription"] = node_description_trait::get(msg.node_description);
  }
}

//=============================================================================
template <typename NodeT>
void from_json(const nlohmann::json& j, NodeT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using node_position_trait =
    optional_field_traits<decltype(msg.node_position)>;
  using node_description_trait =
    optional_field_traits<decltype(msg.node_description)>;

  msg.node_id = j.at("nodeId").get<std::string>();
  msg.sequence_id = j.at("sequenceId").get<uint32_t>();
  msg.released = j.at("released").get<bool>();
  msg.actions = j.at("actions");

  if (j.contains("nodePosition"))
  {
    node_position_trait::set(msg.node_position, j.at("nodePosition"));
  }

  if (j.contains("nodeDescription"))
  {
    node_description_trait::set(
      msg.node_description, j.at("nodeDescription").get<std::string>());
  }
}

}  // namespace node_detail

namespace edge_detail {

//=============================================================================
template <typename EdgeT>
void to_json(nlohmann::json& j, const EdgeT& msg)
{
  using vda5050_json_utils::optional_field_traits;
  using vda5050_json_utils::orientation_type_traits;

  using edge_description_trait =
    optional_field_traits<decltype(msg.edge_description)>;
  using max_speed_trait = optional_field_traits<decltype(msg.max_speed)>;
  using max_height_trait = optional_field_traits<decltype(msg.max_height)>;
  using min_height_trait = optional_field_traits<decltype(msg.min_height)>;
  using orientation_trait = optional_field_traits<decltype(msg.orientation)>;
  using orientation_type_trait =
    optional_field_traits<decltype(msg.orientation_type)>;
  using direction_trait = optional_field_traits<decltype(msg.direction)>;
  using rotation_allowed_trait =
    optional_field_traits<decltype(msg.rotation_allowed)>;
  using max_rotation_speed_trait =
    optional_field_traits<decltype(msg.max_rotation_speed)>;
  using trajectory_trait = optional_field_traits<decltype(msg.trajectory)>;
  using length_trait = optional_field_traits<decltype(msg.length)>;

  j["edgeId"] = msg.edge_id;
  j["sequenceId"] = msg.sequence_id;
  j["startNodeId"] = msg.start_node_id;
  j["endNodeId"] = msg.end_node_id;
  j["released"] = msg.released;
  j["actions"] = msg.actions;

  if (edge_description_trait::has_value(msg.edge_description))
  {
    j["edgeDescription"] = edge_description_trait::get(msg.edge_description);
  }

  if (max_speed_trait::has_value(msg.max_speed))
  {
    j["maxSpeed"] = max_speed_trait::get(msg.max_speed);
  }

  if (max_height_trait::has_value(msg.max_height))
  {
    j["maxHeight"] = max_height_trait::get(msg.max_height);
  }

  if (min_height_trait::has_value(msg.min_height))
  {
    j["minHeight"] = min_height_trait::get(msg.min_height);
  }

  if (orientation_trait::has_value(msg.orientation))
  {
    j["orientation"] = orientation_trait::get(msg.orientation);
  }

  if (orientation_type_trait::has_value(msg.orientation_type))
  {
    using inner_t = typename orientation_type_trait::value_type;

    inner_t value = orientation_type_trait::get(msg.orientation_type);
    j["orientationType"] =
      orientation_type_traits<decltype(value)>::to_string(value);
  }

  if (direction_trait::has_value(msg.direction))
  {
    j["direction"] = direction_trait::get(msg.direction);
  }

  if (rotation_allowed_trait::has_value(msg.rotation_allowed))
  {
    j["rotationAllowed"] = rotation_allowed_trait::get(msg.rotation_allowed);
  }

  if (max_rotation_speed_trait::has_value(msg.max_rotation_speed))
  {
    j["maxRotationSpeed"] =
      max_rotation_speed_trait::get(msg.max_rotation_speed);
  }

  if (trajectory_trait::has_value(msg.trajectory))
  {
    j["trajectory"] = trajectory_trait::get(msg.trajectory);
  }

  if (length_trait::has_value(msg.length))
  {
    j["length"] = length_trait::get(msg.length);
  }
}

//=============================================================================
template <typename EdgeT>
void from_json(const nlohmann::json& j, EdgeT& msg)
{
  using vda5050_json_utils::optional_field_traits;
  using vda5050_json_utils::orientation_type_traits;

  using edge_description_trait =
    optional_field_traits<decltype(msg.edge_description)>;
  using max_speed_trait = optional_field_traits<decltype(msg.max_speed)>;
  using max_height_trait = optional_field_traits<decltype(msg.max_height)>;
  using min_height_trait = optional_field_traits<decltype(msg.min_height)>;
  using orientation_trait = optional_field_traits<decltype(msg.orientation)>;
  using orientation_type_trait =
    optional_field_traits<decltype(msg.orientation_type)>;
  using direction_trait = optional_field_traits<decltype(msg.direction)>;
  using rotation_allowed_trait =
    optional_field_traits<decltype(msg.rotation_allowed)>;
  using max_rotation_speed_trait =
    optional_field_traits<decltype(msg.max_rotation_speed)>;
  using trajectory_trait = optional_field_traits<decltype(msg.trajectory)>;
  using length_trait = optional_field_traits<decltype(msg.length)>;

  msg.edge_id = j.at("edgeId").get<std::string>();
  msg.sequence_id = j.at("sequenceId").get<int32_t>();
  msg.start_node_id = j.at("startNodeId").get<std::string>();
  msg.end_node_id = j.at("endNodeId").get<std::string>();
  msg.released = j.at("released").get<bool>();
  msg.actions = j.at("actions");

  if (j.contains("edgeDescription"))
  {
    edge_description_trait::set(
      msg.edge_description, j.at("edgeDescription").get<std::string>());
  }

  if (j.contains("maxSpeed"))
  {
    max_speed_trait::set(msg.max_speed, j.at("maxSpeed").get<double>());
  }

  if (j.contains("maxHeight"))
  {
    max_height_trait::set(msg.max_height, j.at("maxHeight").get<double>());
  }

  if (j.contains("minHeight"))
  {
    min_height_trait::set(msg.min_height, j.at("minHeight").get<double>());
  }

  if (j.contains("orientation"))
  {
    orientation_trait::set(msg.orientation, j.at("orientation").get<double>());
  }

  if (j.contains("orientationType"))
  {
    using inner_t = typename orientation_type_trait::value_type;

    inner_t value = orientation_type_traits<inner_t>::from_string(
      j.at("orientationType").get<std::string>());
    orientation_type_trait::set(msg.orientation_type, value);
  }

  if (j.contains("direction"))
  {
    direction_trait::set(msg.direction, j.at("direction").get<std::string>());
  }

  if (j.contains("rotationAllowed"))
  {
    rotation_allowed_trait::set(
      msg.rotation_allowed, j.at("rotationAllowed").get<bool>());
  }

  if (j.contains("maxRotationSpeed"))
  {
    max_rotation_speed_trait::set(
      msg.max_rotation_speed, j.at("maxRotationSpeed").get<double>());
  }

  if (j.contains("trajectory"))
  {
    trajectory_trait::set(msg.trajectory, j.at("trajectory"));
  }

  if (j.contains("length"))
  {
    length_trait::set(msg.length, j.at("length").get<double>());
  }
}

}  // namespace edge_detail

namespace order_detail {

//=============================================================================
template <typename OrderT>
void to_json(nlohmann::json& j, const OrderT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using zone_set_id_trait = optional_field_traits<decltype(msg.zone_set_id)>;

  to_json(j, msg.header);

  j["orderId"] = msg.order_id;
  j["orderUpdateId"] = msg.order_update_id;
  j["nodes"] = msg.nodes;
  j["edges"] = msg.edges;

  if (zone_set_id_trait::has_value(msg.zone_set_id))
  {
    j["zoneSetId"] = zone_set_id_trait::get(msg.zone_set_id);
  }
}

//=============================================================================
template <typename OrderT>
void from_json(const nlohmann::json& j, OrderT& msg)
{
  using vda5050_json_utils::optional_field_traits;

  using zone_set_id_trait = optional_field_traits<decltype(msg.zone_set_id)>;

  from_json(j, msg.header);

  msg.order_id = j.at("orderId").get<std::string>();
  msg.order_update_id = j.at("orderUpdateId").get<uint32_t>();
  msg.nodes = j.at("nodes");
  msg.edges = j.at("edges");

  if (j.contains("zoneSetId"))
  {
    zone_set_id_trait::set(
      msg.zone_set_id, j.at("zoneSetId").get<std::string>());
  }
}

}  // namespace order_detail

namespace instant_actions_detail {

//=============================================================================
template <typename InstantActionsT>
void to_json(nlohmann::json& j, const InstantActionsT& msg)
{
  to_json(j, msg.header);

  j["actions"] = msg.actions;
}

//=============================================================================
template <typename InstantActionsT>
void from_json(const nlohmann::json& j, InstantActionsT& msg)
{
  from_json(j, msg.header);

  msg.actions = j.at("actions");
}

}  // namespace instant_actions_detail

}  // namespace vda5050_types

//=============================================================================
namespace vda5050_types {

inline void to_json(nlohmann::json& j, const Header& msg)
{
  vda5050_types::header_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Header& msg)
{
  vda5050_types::header_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Connection& msg)
{
  vda5050_types::connection_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Connection& msg)
{
  vda5050_types::connection_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const NodePosition& msg)
{
  vda5050_types::node_position_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, NodePosition& msg)
{
  vda5050_types::node_position_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const NodeState& msg)
{
  vda5050_types::node_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, NodeState& msg)
{
  vda5050_types::node_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ControlPoint& msg)
{
  vda5050_types::control_point_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ControlPoint& msg)
{
  vda5050_types::control_point_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Trajectory& msg)
{
  vda5050_types::trajectory_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Trajectory& msg)
{
  vda5050_types::trajectory_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const EdgeState& msg)
{
  vda5050_types::edge_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, EdgeState& msg)
{
  vda5050_types::edge_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ActionState& msg)
{
  vda5050_types::action_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ActionState& msg)
{
  vda5050_types::action_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ErrorReference& msg)
{
  vda5050_types::error_reference_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ErrorReference& msg)
{
  vda5050_types::error_reference_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Error& msg)
{
  vda5050_types::error_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Error& msg)
{
  vda5050_types::error_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const BatteryState& msg)
{
  vda5050_types::battery_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, BatteryState& msg)
{
  vda5050_types::battery_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const SafetyState& msg)
{
  vda5050_types::safety_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, SafetyState& msg)
{
  vda5050_types::safety_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const AGVPosition& msg)
{
  vda5050_types::agv_position_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, AGVPosition& msg)
{
  vda5050_types::agv_position_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Velocity& msg)
{
  vda5050_types::velocity_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Velocity& msg)
{
  vda5050_types::velocity_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const BoundingBoxReference& msg)
{
  vda5050_types::bounding_box_reference_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, BoundingBoxReference& msg)
{
  vda5050_types::bounding_box_reference_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const LoadDimensions& msg)
{
  vda5050_types::load_dimensions_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, LoadDimensions& msg)
{
  vda5050_types::load_dimensions_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Load& msg)
{
  vda5050_types::load_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Load& msg)
{
  vda5050_types::load_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const InfoReference& msg)
{
  vda5050_types::info_reference_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, InfoReference& msg)
{
  vda5050_types::info_reference_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Info& msg)
{
  vda5050_types::info_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Info& msg)
{
  vda5050_types::info_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const State& msg)
{
  vda5050_types::state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, State& msg)
{
  vda5050_types::state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ActionParameter& msg)
{
  vda5050_types::action_parameter_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ActionParameter& msg)
{
  vda5050_types::action_parameter_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Action& msg)
{
  vda5050_types::action_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Action& msg)
{
  vda5050_types::action_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Node& msg)
{
  vda5050_types::node_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Node& msg)
{
  vda5050_types::node_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Edge& msg)
{
  vda5050_types::edge_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Edge& msg)
{
  vda5050_types::edge_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Order& msg)
{
  vda5050_types::order_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Order& msg)
{
  vda5050_types::order_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const InstantActions& msg)
{
  vda5050_types::instant_actions_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, InstantActions& msg)
{
  vda5050_types::instant_actions_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& /*j*/, const Factsheet& /*msg*/)
{
  // TODO(sauk): Add missing serialization
}

inline void from_json(const nlohmann::json& /*j*/, Factsheet& /*msg*/)
{
  // TODO(sauk): Add missing deserialization
}

inline void to_json(nlohmann::json& /*j*/, const Visualization& /*msg*/)
{
  // TODO(sauk): Add missing serialization
}

inline void from_json(const nlohmann::json& /*j*/, Visualization& /*msg*/)
{
  // TODO(sauk): Add missing deserialization
}

}  // namespace vda5050_types

//=============================================================================
#ifdef ENABLE_ROS2
namespace vda5050_interfaces {

namespace msg {

inline void to_json(nlohmann::json& j, const Header& msg)
{
  vda5050_types::header_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Header& msg)
{
  vda5050_types::header_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Connection& msg)
{
  vda5050_types::connection_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Connection& msg)
{
  vda5050_types::connection_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const NodePosition& msg)
{
  vda5050_types::node_position_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, NodePosition& msg)
{
  vda5050_types::node_position_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const NodeState& msg)
{
  vda5050_types::node_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, NodeState& msg)
{
  vda5050_types::node_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ControlPoint& msg)
{
  vda5050_types::control_point_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ControlPoint& msg)
{
  vda5050_types::control_point_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Trajectory& msg)
{
  vda5050_types::trajectory_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Trajectory& msg)
{
  vda5050_types::trajectory_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const EdgeState& msg)
{
  vda5050_types::edge_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, EdgeState& msg)
{
  vda5050_types::edge_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ActionState& msg)
{
  vda5050_types::action_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ActionState& msg)
{
  vda5050_types::action_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ErrorReference& msg)
{
  vda5050_types::error_reference_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ErrorReference& msg)
{
  vda5050_types::error_reference_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Error& msg)
{
  vda5050_types::error_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Error& msg)
{
  vda5050_types::error_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const BatteryState& msg)
{
  vda5050_types::battery_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, BatteryState& msg)
{
  vda5050_types::battery_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const SafetyState& msg)
{
  vda5050_types::safety_state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, SafetyState& msg)
{
  vda5050_types::safety_state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const AGVPosition& msg)
{
  vda5050_types::agv_position_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, AGVPosition& msg)
{
  vda5050_types::agv_position_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Velocity& msg)
{
  vda5050_types::velocity_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Velocity& msg)
{
  vda5050_types::velocity_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const BoundingBoxReference& msg)
{
  vda5050_types::bounding_box_reference_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, BoundingBoxReference& msg)
{
  vda5050_types::bounding_box_reference_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const LoadDimensions& msg)
{
  vda5050_types::load_dimensions_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, LoadDimensions& msg)
{
  vda5050_types::load_dimensions_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Load& msg)
{
  vda5050_types::load_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Load& msg)
{
  vda5050_types::load_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const InfoReference& msg)
{
  vda5050_types::info_reference_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, InfoReference& msg)
{
  vda5050_types::info_reference_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Info& msg)
{
  vda5050_types::info_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Info& msg)
{
  vda5050_types::info_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const State& msg)
{
  vda5050_types::state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, State& msg)
{
  vda5050_types::state_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const ActionParameter& msg)
{
  vda5050_types::action_parameter_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, ActionParameter& msg)
{
  vda5050_types::action_parameter_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Action& msg)
{
  vda5050_types::action_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Action& msg)
{
  vda5050_types::action_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Node& msg)
{
  vda5050_types::node_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Node& msg)
{
  vda5050_types::node_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Edge& msg)
{
  vda5050_types::edge_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Edge& msg)
{
  vda5050_types::edge_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Order& msg)
{
  vda5050_types::order_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Order& msg)
{
  vda5050_types::order_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const InstantActions& msg)
{
  vda5050_types::instant_actions_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, InstantActions& msg)
{
  vda5050_types::instant_actions_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& /*j*/, const Factsheet& /*msg*/)
{
  // TODO(sauk): Add missing serialization
}

inline void from_json(const nlohmann::json& /*j*/, Factsheet& /*msg*/)
{
  // TODO(sauk): Add missing deserialization
}

inline void to_json(nlohmann::json& /*j*/, const Visualization& /*msg*/)
{
  // TODO(sauk): Add missing serialization
}

inline void from_json(const nlohmann::json& /*j*/, Visualization& /*msg*/)
{
  // TODO(sauk): Add missing deserialization
}

}  // namespace msg

}  // namespace vda5050_interfaces
#endif  // ENABLE_ROS2

#endif  // VDA5050_JSON_UTILS__SERIALIZATION_HPP_
