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

#ifndef VDA5050_TYPES__STATE_HPP_
#define VDA5050_TYPES__STATE_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "vda5050_types/action_state.hpp"
#include "vda5050_types/agv_position.hpp"
#include "vda5050_types/battery_state.hpp"
#include "vda5050_types/edge_state.hpp"
#include "vda5050_types/error.hpp"
#include "vda5050_types/header.hpp"
#include "vda5050_types/info.hpp"
#include "vda5050_types/load.hpp"
#include "vda5050_types/node_state.hpp"
#include "vda5050_types/operating_mode.hpp"
#include "vda5050_types/safety_state.hpp"
#include "vda5050_types/velocity.hpp"

namespace vda5050_types {

/// \brief A message containing state information of the AGV published
/// by the AGV on topic /state to inform the master control of
/// the current AGV state
struct State
{
  /// \brief Message header that contains the common fields required across
  /// all messages
  Header header;

  /// \brief Unique order identification of the current order or the
  /// previous finished order.  The ID is kept until a new order is
  /// received. Empty string if no previous ID is available
  std::string order_id;

  /// \brief Order update identification to identify that an order update
  /// has been accepted by the AGV. The default value is 0 if no previous
  /// order update ID is available
  uint32_t order_update_id = 0;

  /// \brief Unique ID of the zone set that the AGV currently uses
  /// for path planning.  Must be the same as the one used in the order,
  /// otherwise the AGV is to reject the order
  std::optional<std::string> zone_set_id;

  /// \brief Node ID of last reached node or, if AGV is currently on a
  /// node, current node. Empty string if no ID is available
  std::string last_node_id;

  /// \brief Sequence ID of the last reached node or, if the AGV is currently
  /// on a node, sequence of current node. 0 if no ID is available
  uint32_t last_node_sequence_id = 0;

  /// \brief Array of nodes that need to be traversed for fulfilling
  /// the order. Empty list if the AGV is idle
  std::vector<NodeState> node_states;

  /// \brief Array of edges that need to be traversed for fulfilling
  /// the order. Empty list if the AGV is idle
  std::vector<EdgeState> edge_states;

  /// \brief Defines the position on a map in world coordinates. Each
  /// floor has its own map.
  std::optional<AGVPosition> agv_position;

  /// \brief The AGVs velocity in vehicle coordinates
  std::optional<Velocity> velocity;

  /// \brief Array for information about the loads the AGV is currently
  /// carrying. If an empty list is received, then the master control
  /// should assume the AGV can reason about its load state and that it
  /// currently does not carry any load
  std::optional<std::vector<Load>> loads;

  /// \brief Indicates the driving and/or rotation movements of the AGV. Does
  /// not include lift movements
  bool driving = false;

  /// \brief Indicates the paused state of the AGV, either because of a
  /// physical button push or an instantAction, and that it can resume
  /// the order
  std::optional<bool> paused;

  /// \brief Request a new base from the master control when the AGV is almost
  /// at the end of its base and about to reduce speed
  std::optional<bool> new_base_request;

  /// \brief Used by line guided vehicles to indicate the distance it has been
  /// driving past the last node.\nDistance is in meters
  std::optional<double> distance_since_last_node;

  /// \brief  Array with a list of current actions and actions to be
  /// finished. This may include actions from previous nodes that are
  /// still in progress. When an action is completed, an updated state
  /// message must be published with the action status set to finished. The
  /// action states are kept until a new order is received
  std::vector<ActionState> action_states;

  /// \brief Contains all battery-related information
  BatteryState battery_state;

  /// \brief Current operating mode of the AGV
  OperatingMode operating_mode = OperatingMode::AUTOMATIC;

  /// \brief Array of error names/types. Errors are kept
  /// until resolution
  std::vector<Error> errors;

  /// \brief Array of information messages for visualization/debugging. There
  /// is no specification when these messages are deleted
  std::optional<std::vector<Info>> information;

  /// \brief Contains all safety-related information
  SafetyState safety_state;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const State& other) const
  {
    if (this->header != other.header) return false;
    if (this->order_id != other.order_id) return false;
    if (this->order_update_id != other.order_update_id) return false;
    if (this->zone_set_id != other.zone_set_id) return false;
    if (this->last_node_id != other.last_node_id) return false;
    if (this->last_node_sequence_id != other.last_node_sequence_id)
      return false;
    if (this->node_states != other.node_states) return false;
    if (this->edge_states != other.edge_states) return false;
    if (this->agv_position != other.agv_position) return false;
    if (this->velocity != other.velocity) return false;
    if (this->loads != other.loads) return false;
    if (this->driving != other.driving) return false;
    if (this->paused != other.paused) return false;
    if (this->new_base_request != other.new_base_request) return false;
    if (this->distance_since_last_node != other.distance_since_last_node)
      return false;
    if (this->action_states != other.action_states) return false;
    if (this->battery_state != other.battery_state) return false;
    if (this->operating_mode != other.operating_mode) return false;
    if (this->errors != other.errors) return false;
    if (this->information != other.information) return false;
    if (this->safety_state != other.safety_state) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const State& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__STATE_HPP_
