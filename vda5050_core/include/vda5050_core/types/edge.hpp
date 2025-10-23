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

#ifndef VDA5050_CORE__TYPES__EDGE_HPP_
#define VDA5050_CORE__TYPES__EDGE_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "vda5050_core/types/action.hpp"
#include "vda5050_core/types/orientation_type.hpp"
#include "vda5050_core/types/trajectory.hpp"

namespace vda5050_core {

namespace types {

/// \struct Edge
/// \brief  Directional connection between two nodes
struct Edge
{
  /// \brief Unique edge identification.
  std::string edge_id;

  /// \brief Sequence number to track the order of nodes and edges.
  ///        The variable runs across all nodes and edges of the same order
  ///        and is reset when a new orderId is issued.
  uint32_t sequence_id = 0;

  /// \brief Additional information on the edge.
  std::optional<std::string> edge_description;

  /// \brief Indicates whether the edge is part of the base or the horizon.
  ///        - `true`: Edge is part of the base.
  ///        - `false`: Edge is part of the horizon.
  bool released = false;

  /// \brief nodeId of the first node within the order.
  std::string start_node_id;

  /// \brief nodeId of the last node within the order.
  std::string end_node_id;

  /// \brief Permitted maximum speed on the edge [m/s].
  ///        Speed is defined by the fastest measurement of the vehicle.
  std::optional<double> max_speed;

  /// \brief Permitted maximum height of the vehicle (including load) on the edge [m].
  std::optional<double> max_height;

  /// \brief Permitted minimum height of the load handling device on the edge [m].
  std::optional<double> min_height;

  /// \brief Orientation of the AGV on the edge [rad].
  ///        The value of orientationType determines how this is interpreted:
  ///        - `GLOBAL`: relative to the global project-specific map coordinate system.
  ///        - `TANGENTIAL`: tangential to the edge (0 = forwards, π = backwards).
  ///        If no trajectory is defined, the rotation is applied along the direct path between nodes.
  ///        If a trajectory exists, it is applied to that trajectory.
  ///        If rotationAllowed is `false`, rotation must occur before entering the edge;
  ///        otherwise, the order must be rejected.
  std::optional<double> orientation;

  /// \brief Defines how the orientation is interpreted.
  ///        Enum: `GLOBAL` (relative to map) or `TANGENTIAL` (tangential to the edge).
  ///        Default: `TANGENTIAL`.
  std::optional<OrientationType> orientation_type;

  /// \brief Defines the travel direction at junctions for line- or wire-guided vehicles.
  ///        Examples: `"left"`, `"right"`, `"straight"`.
  std::optional<std::string> direction;

  /// \brief Indicates whether rotation is allowed on this edge.
  ///        - `true`: Rotation allowed.
  ///        - `false`: Rotation not allowed.
  ///        Optional — no limit if not set.
  std::optional<bool> rotation_allowed;

  /// \brief Maximum permitted rotation speed [rad/s].
  ///        Optional — no limit if not set.
  std::optional<double> max_rotation_speed;

  /// \brief Trajectory object defining the path between start and end nodes (NURBS format).
  ///        Optional — can be omitted if the AGV plans its own trajectory or cannot process one.
  std::optional<Trajectory> trajectory;

  /// \brief Length of the path between start and end nodes [m].
  ///        Used by line-guided AGVs to reduce speed before reaching a stop position.
  std::optional<double> length;

  /// \brief List of actions to be executed on this edge.
  ///        The actions are active only while the AGV traverses this edge.
  ///        Empty list if no actions are required.
  std::vector<Action> actions;

  /// \brief Equality operator.
  /// \param other The Edge instance to compare with.
  /// \return True if all member variables are equal; otherwise false.
  inline bool operator==(const Edge& other) const
  {
    if (this->actions != other.actions) return false;
    if (this->direction != other.direction) return false;
    if (this->edge_description != other.edge_description) return false;
    if (this->edge_id != other.edge_id) return false;
    if (this->end_node_id != other.end_node_id) return false;
    if (this->length != other.length) return false;
    if (this->max_height != other.max_height) return false;
    if (this->max_rotation_speed != other.max_rotation_speed) return false;
    if (this->max_speed != other.max_speed) return false;
    if (this->min_height != other.min_height) return false;
    if (this->orientation != other.orientation) return false;
    if (this->orientation_type != other.orientation_type) return false;
    if (this->released != other.released) return false;
    if (this->rotation_allowed != other.rotation_allowed) return false;
    if (this->sequence_id != other.sequence_id) return false;
    if (this->start_node_id != other.start_node_id) return false;
    if (this->trajectory != other.trajectory) return false;

    return true;
  }

  /// \brief Inequality operator.
  /// \param other The Edge instance to compare with.
  /// \return True if any member variable differs; otherwise false.
  inline bool operator!=(const Edge& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Edge& d)
{
  j["actions"] = d.actions;

  if (d.direction.has_value()) j["direction"] = *d.direction;
  if (d.edge_description.has_value())
    j["edgeDescription"] = *d.edge_description;

  j["edgeId"] = d.edge_id;
  j["endNodeId"] = d.end_node_id;

  if (d.length.has_value()) j["length"] = *d.length;
  if (d.max_height.has_value()) j["maxHeight"] = *d.max_height;
  if (d.max_rotation_speed.has_value())
    j["maxRotationSpeed"] = *d.max_rotation_speed;
  if (d.max_speed.has_value()) j["maxSpeed"] = *d.max_speed;
  if (d.min_height.has_value()) j["minHeight"] = *d.min_height;
  if (d.orientation.has_value()) j["orientation"] = *d.orientation;
  if (d.orientation_type.has_value())
    j["orientationType"] = *d.orientation_type;

  j["released"] = d.released;

  if (d.rotation_allowed.has_value())
    j["rotationAllowed"] = *d.rotation_allowed;

  j["sequenceId"] = d.sequence_id;
  j["startNodeId"] = d.start_node_id;

  if (d.trajectory.has_value()) j["trajectory"] = *d.trajectory;
}

inline void from_json(const json& j, Edge& d)
{
  d.actions = j.at("actions").get<std::vector<Action>>();

  if (j.contains("direction")) d.direction = j.at("direction");
  if (j.contains("edgeDescription"))
    d.edge_description = j.at("edgeDescription");

  d.edge_id = j.at("edgeId");
  d.end_node_id = j.at("endNodeId");

  if (j.contains("length")) d.length = j.at("length");
  if (j.contains("maxHeight")) d.max_height = j.at("maxHeight");
  if (j.contains("maxRotationSpeed"))
    d.max_rotation_speed = j.at("maxRotationSpeed");
  if (j.contains("maxSpeed")) d.max_speed = j.at("maxSpeed");
  if (j.contains("minHeight")) d.min_height = j.at("minHeight");
  if (j.contains("orientation")) d.orientation = j.at("orientation");
  if (j.contains("orientationType"))
    d.orientation_type = j.at("orientationType");

  d.released = j.at("released");

  if (j.contains("rotationAllowed"))
    d.rotation_allowed = j.at("rotationAllowed");

  d.sequence_id = j.at("sequenceId");
  d.start_node_id = j.at("startNodeId");

  if (j.contains("trajectory")) d.trajectory = j.at("trajectory");
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__EDGE_HPP_
