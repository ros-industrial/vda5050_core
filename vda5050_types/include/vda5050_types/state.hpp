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

/// @struct State
/// @brief  VDA5050 State Struct
struct State
{
  /// @brief headerId of the message. The headerId is defined per topic and incremented by 1 with each sent (but not necessarily received) message.
  Header header;

  /// @brief Unique order identification of the current order or the previous finished order.
  ///        The orderId is kept until a new order is received.
  ///        Empty string (\"\") if no previous orderId is available.
  std::string order_id;

  /// @brief Order Update Identification to identify that an order update has been accepted by the AGV. \"0\" if no previous orderUpdateId is available
  uint32_t order_update_id = 0;

  /// @brief Unique ID of the zone set that the AGV currently uses for path planning.
  ///        Must be the same as the one used in the order, otherwise the AGV is to reject the order.
  ///        Optional: If the AGV does not use zones, this field can be omitted.
  std::optional<std::string> zone_set_id;

  /// @brief nodeID of last reached node or, if AGV is currently on a node, current node (e.g., \"node7\"). Empty string (\"\") if no lastNodeId is available.
  std::string last_node_id;

  /// @brief sequenceId of the last reached node or, if the AGV is currently on a node, sequenceId of current node. \"0\" if no lastNodeSequenceId is available.
  uint32_t last_node_sequence_id = 0;

  /// @brief Array of nodeState-Objects, that need to be traversed for fulfilling the order. Empty list if idle.
  std::vector<NodeState> node_states;

  /// @brief Array of edgeState-Objects, that need to be traversed for fulfilling the order, empty list if idle.
  std::vector<EdgeState> edge_states;

  /// @brief Defines the position on a map in world coordinates. Each floor has its own map.
  std::optional<AgvPosition> agv_position;

  /// @brief The AGVs velocity in vehicle coordinates
  std::optional<Velocity> velocity;

  /// @brief Loads, that are currently handled by the AGV. Optional: If AGV cannot determine load state, leave the array out of the state.
  ///        If the AGV can determine the load state, but the array is empty, the AGV is considered unloaded.
  std::optional<std::vector<Load>> loads;

  /// @brief True: indicates that the AGV is driving and/or rotating. Other movements of the AGV (e.g., lift movements) are not included here.
  ///        False: indicates that the AGV is neither driving nor rotating
  bool driving = false;

  /// @brief True: AGV is currently in a paused state, either because of the push of a physical button on the AGV or because of an instantAction.
  ///        The AGV can resume the order.
  ///        False: The AGV is currently not in a paused state.
  std::optional<bool> paused;

  /// @brief True: AGV is almost at the end of the base and will reduce speed if no new base is transmitted.
  ///        Trigger for master control to send new base
  ///        False: no base update required.
  std::optional<bool> new_base_request;

  /// @brief Used by line guided vehicles to indicate the distance it has been driving past the \"lastNodeId\".\nDistance is in meters.
  std::optional<double> distance_since_last_node;

  /// @brief Contains a list of the current actions and the actions which are yet to be finished. This may include actions from previous nodes that are still in progress
  ///        When an action is completed, an updated state message is published with actionStatus set to finished and if applicable with the corresponding resultDescription.
  ///        The actionStates are kept until a new order is received.
  std::vector<ActionState> action_states;

  /// @brief Contains all battery-related information.
  BatteryState battery_state;

  /// @brief Current operating mode of the AGV
  OperatingMode operating_mode = OperatingMode::AUTOMATIC;

  /// @brief Type/name of error.
  std::vector<Error> errors;

  /// @brief Array of info-objects. An empty array indicates, that the AGV has no information.
  ///        This should only be used for visualization or debugging – it must not be used for logic in master control.
  std::optional<std::vector<Info>> information;

  /// @brief Contains all safety-related information.
  SafetyState safety_state;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__STATE_HPP
