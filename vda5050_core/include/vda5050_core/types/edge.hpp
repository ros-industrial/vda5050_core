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

#include "vda5050_core/types/action.hpp"
#include "vda5050_core/types/orientation_type.hpp"
#include "vda5050_core/types/trajectory.hpp"

namespace vda5050_core {

namespace types {

/// \brief Defines a directional connection between two nodes.
struct Edge
{
  /// \brief Unique edge identification.
  std::string edge_id;

  /// \brief Number to track the sequence of nodes and edges in an order,
  /// simplifying updates. Distinguishes between nodes/edges that may be
  /// revisited in the same order. Resets when a new orderId is issued.
  uint32_t sequence_id;

  /// \brief The nodeId of the start node for this edge.
  std::string start_node_id;

  /// \brief The nodeId of the end node for this edge.
  std::string end_node_id;

  /// \brief Indicates if the edge is part of the base or horizon.
  /// "true" indicates that edge belongs to the base.
  /// "false" indicates that edge belongs to the horizon.
  bool released;

  /// \brief Array of actionIds to be executed on the edge.
  /// Array is empty if no actions are required.
  /// An action triggered by an edge will only be active for the time that
  /// the AGV is traversing it. When the AGV leaves the edge, the action will
  /// stop and the state before entering the edge will be restored.
  std::vector<Action> actions;

  /// \brief Additional information about the edge.
  std::optional<std::string> edge_description;

  /// \brief Permitted maximum speed [m/s] on the edge.
  /// Speed is defined by the fastest measurement of the vehicle.
  std::optional<double> max_speed;

  /// \brief Permitted maximum height [m] of the vehicle including load
  /// on the edge.
  std::optional<double> max_height;

  /// \brief Permitted minimal height [m] of the load handling device
  /// on the edge.
  std::optional<double> min_height;

  /// \brief Orientation of the AGV on the edge [rad].
  /// If the AGV starts in a different orientation, rotate the vehicle on
  /// the edge to the desired orientation if rotationAllowed is true.
  /// If rotationAllowed is false, rotate before entering the edge.
  std::optional<double> orientation;

  /// \brief Defines if the orientation field is interpreted relative to the
  /// global project specific map coordinate system or tangential to the edge.
  /// If tangential to the edge, 0.0 = forwards, Pi = backwards.
  /// Default value: TANGENTIAL
  std::optional<OrientationType> orientation_type;

  /// \brief Sets direction at junctions for line-guided or wire-guided
  /// vehicles. To be defined initially (vehicle-individual).
  /// Eg: straight, left, right.
  std::optional<std::string> direction;

  /// \brief "true" indicates that rotation is allowed on the edge.
  /// "false" indicates that rotation is not allowed on the edge.
  /// No limit, if not set.
  std::optional<bool> rotation_allowed;

  /// \brief The maximum rotation speed [rad/s].
  /// No limit, if not set.
  std::optional<double> max_rotation_speed;

  /// \brief The Trajectory object for this edge as a Non-Uniform Rational
  /// B-Spline (NURBS). Defines the path that the AGV should move between
  /// the start node and end node of the edge. This field can be omitted if
  /// the AGV cannot process trajectories or if the AGV plans its own
  /// trajectory.
  std::optional<Trajectory> trajectory;

  /// \brief Length of the path from the start node to the end node [m].
  /// Used by line-guided AGVs to decrease their speed before reaching a stop
  /// position.
  std::optional<double> length;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Edge& other) const
  {
    if (this->edge_id != other.edge_id) return false;
    if (this->sequence_id != other.sequence_id) return false;
    if (this->start_node_id != other.start_node_id) return false;
    if (this->end_node_id != other.end_node_id) return false;
    if (this->released != other.released) return false;
    if (this->actions != other.actions) return false;
    if (this->edge_description != other.edge_description) return false;
    if (this->max_speed != other.max_speed) return false;
    if (this->max_height != other.max_height) return false;
    if (this->min_height != other.min_height) return false;
    if (this->orientation != other.orientation) return false;
    if (this->orientation_type != other.orientation_type) return false;
    if (this->direction != other.direction) return false;
    if (this->rotation_allowed != other.rotation_allowed) return false;
    if (this->max_rotation_speed != other.max_rotation_speed) return false;
    if (this->trajectory != other.trajectory) return false;
    if (this->length != other.length) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Edge& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__EDGE_HPP_
