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

#ifndef VDA5050_TYPES__MAX_ARRAY_LENS_HPP_
#define VDA5050_TYPES__MAX_ARRAY_LENS_HPP_

#include <cstdint>

namespace vda5050_types {

/// \brief This message defines the maximum lengths of arrays processable by the AGV
/// according to the VDA5050 specification.
struct MaxArrayLens
{
  /// \brief Maximum number of nodes per order processable by the AGV.
  uint32_t order_nodes;

  /// \brief Maximum number of edges per order processable by the AGV.
  uint32_t order_edges;

  /// \brief Maximum number of actions per node processable by the AGV.
  uint32_t node_actions;

  /// \brief Maximum number of actions per edge processable by the AGV.
  uint32_t edge_actions;

  /// \brief Maximum number of parameters per action processable by the AGV.
  uint32_t actions_actions_parameters;

  /// \brief Maximum number of instant actions per message processable by the AGV.
  uint32_t instant_actions;

  /// \brief Maximum number of knots per trajectory processable by the AGV.
  uint32_t trajectory_knot_vector;

  /// \brief Maximum number of control points per trajectory processable by the AGV.
  uint32_t trajectory_control_points;

  /// \brief Maximum number of nodeStates sent by the AGV,
  /// maximum number of nodes in base of AGV.
  uint32_t state_node_states;

  /// \brief Maximum number of edgeStates sent by the AGV,
  /// maximum number of edges in base of AGV.
  uint32_t state_edge_states;

  /// \brief Maximum number of load objects sent by the AGV.
  uint32_t state_loads;

  /// \brief Maximum number of actionStates sent by the AGV.
  uint32_t state_action_states;

  /// \brief Maximum number of errors sent by the AGV in one state message.
  uint32_t state_errors;

  /// \brief Maximum number of information objects sent by the AGV in one state message.
  uint32_t state_information;

  /// \brief Maximum number of error references sent by the AGV for each error.
  uint32_t error_error_references;

  /// \brief Maximum number of info references sent by the AGV for each information.
  uint32_t information_info_references;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const MaxArrayLens& other) const
  {
    if (this->order_nodes != other.order_nodes) return false;
    if (this->order_edges != other.order_edges) return false;
    if (this->node_actions != other.node_actions) return false;
    if (this->edge_actions != other.edge_actions) return false;
    if (this->actions_actions_parameters != other.actions_actions_parameters)
      return false;
    if (this->instant_actions != other.instant_actions) return false;
    if (this->trajectory_knot_vector != other.trajectory_knot_vector)
      return false;
    if (this->trajectory_control_points != other.trajectory_control_points)
      return false;
    if (this->state_node_states != other.state_node_states) return false;
    if (this->state_edge_states != other.state_edge_states) return false;
    if (this->state_loads != other.state_loads) return false;
    if (this->state_action_states != other.state_action_states) return false;
    if (this->state_errors != other.state_errors) return false;
    if (this->state_information != other.state_information) return false;
    if (this->error_error_references != other.error_error_references)
      return false;
    if (this->information_info_references != other.information_info_references)
      return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const MaxArrayLens& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__MAX_ARRAY_LENS_HPP_
