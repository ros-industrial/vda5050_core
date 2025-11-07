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

#include <vda5050_types/action_state.hpp>
#include <vda5050_types/connection.hpp>
#include <vda5050_types/edge_state.hpp>
#include <vda5050_types/error.hpp>
#include <vda5050_types/error_reference.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/node_position.hpp>
#include <vda5050_types/node_state.hpp>
#include <vda5050_types/safety_state.hpp>
#include <vda5050_types/state.hpp>
#include <vda5050_types/trajectory.hpp>

#ifdef ENABLE_ROS2
#include <vda5050_msgs/msg/action_state.hpp>
#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/edge_state.hpp>
#include <vda5050_msgs/msg/error.hpp>
#include <vda5050_msgs/msg/error_reference.hpp>
#include <vda5050_msgs/msg/header.hpp>
#include <vda5050_msgs/msg/node_position.hpp>
#include <vda5050_msgs/msg/node_state.hpp>
#include <vda5050_msgs/msg/safety_state.hpp>
#include <vda5050_msgs/msg/state.hpp>
#include <vda5050_msgs/msg/trajectory.hpp>
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
  using theta_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.theta)>;
  using allowed_deviation_x_y_trait = vda5050_json_utils::optional_field_traits<
    decltype(msg.allowed_deviation_x_y)>;
  using allowed_deviation_theta_trait =
    vda5050_json_utils::optional_field_traits<
      decltype(msg.allowed_deviation_theta)>;
  using map_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.map_description)>;

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
  using theta_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.theta)>;
  using allowed_deviation_x_y_trait = vda5050_json_utils::optional_field_traits<
    decltype(msg.allowed_deviation_x_y)>;
  using allowed_deviation_theta_trait =
    vda5050_json_utils::optional_field_traits<
      decltype(msg.allowed_deviation_theta)>;
  using map_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.map_description)>;

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
  using node_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.node_description)>;
  using node_position_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.node_position)>;

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
  using node_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.node_description)>;
  using node_position_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.node_position)>;

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
  using edge_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.edge_description)>;
  using trajectory_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.trajectory)>;

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
  using edge_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.edge_description)>;
  using trajectory_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.trajectory)>;

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
  using action_type_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.action_type)>;
  using action_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.action_description)>;
  using result_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.result_description)>;

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
  using action_type_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.action_type)>;
  using action_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.action_description)>;
  using result_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.result_description)>;
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
  using error_reference_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.error_references)>;
  using error_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.error_description)>;

  j["errorType"] = msg.error_type;

  j["errorLevel"] =
    error_level_traits<decltype(msg.error_level)>::to_string(msg.error_level);

  if (error_reference_trait::has_value(msg.error_references))
  {
    j["errorReferences"] = error_reference_trait::get(msg.error_references);
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
  using error_reference_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.error_references)>;
  using error_description_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.error_description)>;

  msg.error_type = j.at("errorType").get<std::string>();

  msg.error_level = error_level_traits<decltype(msg.error_level)>::from_string(
    j.at("errorLevel").get<std::string>());

  if (j.contains("errorReferences"))
  {
    error_reference_trait::set(msg.error_references, j.at("errorReferences"));
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
  using battery_voltage_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.battery_voltage)>;
  using battery_health_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.battery_health)>;
  using reach_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.reach)>;

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
  using battery_voltage_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.battery_voltage)>;
  using battery_health_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.battery_health)>;
  using reach_trait =
    vda5050_json_utils::optional_field_traits<decltype(msg.reach)>;

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

namespace state_detail {

//=============================================================================
template <typename StateT>
void to_json(nlohmann::json& j, const StateT& msg)
{
  using vda5050_json_utils::operating_mode_traits;

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
}

//=============================================================================
template <typename StateT>
void from_json(const nlohmann::json& j, StateT& msg)
{
  using vda5050_json_utils::operating_mode_traits;

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
}

}  // namespace state_detail

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

inline void to_json(nlohmann::json& j, const State& msg)
{
  vda5050_types::state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, State& msg)
{
  vda5050_types::state_detail::from_json(j, msg);
}

}  // namespace vda5050_types

//=============================================================================
#ifdef ENABLE_ROS2
namespace vda5050_msgs {

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

inline void to_json(nlohmann::json& j, const State& msg)
{
  vda5050_types::state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, State& msg)
{
  vda5050_types::state_detail::from_json(j, msg);
}

}  // namespace msg

}  // namespace vda5050_msgs
#endif  // ENABLE_ROS2

#endif  // VDA5050_JSON_UTILS__SERIALIZATION_HPP_
