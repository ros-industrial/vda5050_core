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

#ifndef VDA5050_CORE__TYPES__STATE_HPP_
#define VDA5050_CORE__TYPES__STATE_HPP_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/agv_position.hpp"
#include "vda5050_core/types/battery_state.hpp"
#include "vda5050_core/types/edge_state.hpp"
#include "vda5050_core/types/error.hpp"
#include "vda5050_core/types/header.hpp"
#include "vda5050_core/types/info.hpp"
#include "vda5050_core/types/load.hpp"
#include "vda5050_core/types/node_state.hpp"
#include "vda5050_core/types/operating_mode.hpp"
#include "vda5050_core/types/safety_state.hpp"
#include "vda5050_core/types/velocity.hpp"

namespace vda5050_core {

namespace types {

/// \struct State
/// \brief  VDA5050 State Struct
struct State
{
  /// \brief headerId of the message. The headerId is defined per topic and incremented by 1 with each sent (but not necessarily received) message.
  Header header;

  /// \brief Unique order identification of the current order or the previous finished order.
  ///        The orderId is kept until a new order is received.
  ///        Empty string (\"\") if no previous orderId is available.
  std::string order_id;

  /// \brief Order Update Identification to identify that an order update has been accepted by the AGV. \"0\" if no previous orderUpdateId is available
  uint32_t order_update_id = 0;

  /// \brief Unique ID of the zone set that the AGV currently uses for path planning.
  ///        Must be the same as the one used in the order, otherwise the AGV is to reject the order.
  ///        Optional: If the AGV does not use zones, this field can be omitted.
  std::optional<std::string> zone_set_id;

  /// \brief nodeID of last reached node or, if AGV is currently on a node, current node (e.g., \"node7\"). Empty string (\"\") if no lastNodeId is available.
  std::string last_node_id;

  /// \brief sequenceId of the last reached node or, if the AGV is currently on a node, sequenceId of current node. \"0\" if no lastNodeSequenceId is available.
  uint32_t last_node_sequence_id = 0;

  /// \brief Array of nodeState-Objects, that need to be traversed for fulfilling the order. Empty list if idle.
  std::vector<NodeState> node_states;

  /// \brief Array of edgeState-Objects, that need to be traversed for fulfilling the order, empty list if idle.
  std::vector<EdgeState> edge_states;

  /// \brief Defines the position on a map in world coordinates. Each floor has its own map.
  std::optional<AgvPosition> agv_position;

  /// \brief The AGVs velocity in vehicle coordinates
  std::optional<Velocity> velocity;

  /// \brief Loads, that are currently handled by the AGV. Optional: If AGV cannot determine load state, leave the array out of the state.
  ///        If the AGV can determine the load state, but the array is empty, the AGV is considered unloaded.
  std::optional<std::vector<Load>> loads;

  /// \brief True: indicates that the AGV is driving and/or rotating. Other movements of the AGV (e.g., lift movements) are not included here.
  ///        False: indicates that the AGV is neither driving nor rotating
  bool driving = false;

  /// \brief True: AGV is currently in a paused state, either because of the push of a physical button on the AGV or because of an instantAction.
  ///        The AGV can resume the order.
  ///        False: The AGV is currently not in a paused state.
  std::optional<bool> paused;

  /// \brief True: AGV is almost at the end of the base and will reduce speed if no new base is transmitted.
  ///        Trigger for master control to send new base
  ///        False: no base update required.
  std::optional<bool> new_base_request;

  /// \brief Used by line guided vehicles to indicate the distance it has been driving past the \"lastNodeId\".\nDistance is in meters.
  std::optional<double> distance_since_last_node;

  /// \brief Contains a list of the current actions and the actions which are yet to be finished. This may include actions from previous nodes that are still in progress
  ///        When an action is completed, an updated state message is published with actionStatus set to finished and if applicable with the corresponding resultDescription.
  ///        The actionStates are kept until a new order is received.
  std::vector<ActionState> action_states;

  /// \brief Contains all battery-related information.
  BatteryState battery_state;

  /// \brief Current operating mode of the AGV
  OperatingMode operating_mode = OperatingMode::AUTOMATIC;

  /// \brief Type/name of error.
  std::vector<Error> errors;

  /// \brief Array of info-objects. An empty array indicates, that the AGV has no information.
  ///        This should only be used for visualization or debugging – it must not be used for logic in master control.
  std::optional<std::vector<Info>> information;

  /// \brief Contains all safety-related information.
  SafetyState safety_state;

  /// \brief Compares two State objects for equality.
  /// \param other The State instance to compare with.
  /// \return True if all fields are equal, otherwise false.
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

  /// \brief Compares two State objects for inequality.
  /// \param other The State instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const State& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const State& d)
{
  to_json(j, d.header);
  j["actionStates"] = d.action_states;
  if (d.agv_position.has_value())
  {
    j["agvPosition"] = *d.agv_position;
  }
  j["batteryState"] = d.battery_state;
  if (d.distance_since_last_node.has_value())
  {
    j["distanceSinceLastNode"] = *d.distance_since_last_node;
  }
  j["driving"] = d.driving;
  j["edgeStates"] = d.edge_states;
  j["errors"] = d.errors;
  if (d.information.has_value())
  {
    j["information"] = *d.information;
  }
  j["lastNodeId"] = d.last_node_id;
  j["lastNodeSequenceId"] = d.last_node_sequence_id;
  if (d.loads.has_value())
  {
    j["loads"] =
      *d.loads;  // Keep possible "null" loads since they could represent an arbitrary load
  }
  if (d.new_base_request.has_value())
  {
    j["newBaseRequest"] = *d.new_base_request;
  }
  j["nodeStates"] = d.node_states;
  j["operatingMode"] = d.operating_mode;
  j["orderId"] = d.order_id;
  j["orderUpdateId"] = d.order_update_id;
  if (d.paused.has_value())
  {
    j["paused"] = *d.paused;
  }
  j["safetyState"] = d.safety_state;
  if (d.velocity.has_value())
  {
    j["velocity"] = *d.velocity;
  }
  if (d.zone_set_id.has_value())
  {
    j["zoneSetId"] = *d.zone_set_id;
  }
}

inline void from_json(const json& j, State& d)
{
  from_json(j, d.header);
  d.action_states = j.at("actionStates").get<std::vector<ActionState>>();
  if (j.contains("agvPosition"))
  {
    d.agv_position = j.at("agvPosition");
  }
  d.battery_state = j.at("batteryState");
  if (j.contains("distanceSinceLastNode"))
  {
    d.distance_since_last_node = j.at("distanceSinceLastNode");
  }
  d.driving = j.at("driving");
  d.edge_states = j.at("edgeStates").get<std::vector<EdgeState>>();
  d.errors = j.at("errors").get<std::vector<Error>>();
  if (j.contains("information"))
  {
    d.information = j.at("information").get<std::vector<Info>>();
  }
  d.last_node_id = j.at("lastNodeId");
  d.last_node_sequence_id = j.at("lastNodeSequenceId");
  if (j.contains("loads"))
  {
    d.loads = j.at("loads").get<std::vector<Load>>();
  }
  if (j.contains("newBaseRequest"))
  {
    d.new_base_request = j.at("newBaseRequest");
  }
  d.node_states = j.at("nodeStates").get<std::vector<NodeState>>();
  d.operating_mode = j.at("operatingMode");
  d.order_id = j.at("orderId");
  d.order_update_id = j.at("orderUpdateId");
  if (j.contains("paused"))
  {
    d.paused = j.at("paused");
  }
  d.safety_state = j.at("safetyState");
  if (j.contains("velocity"))
  {
    d.velocity = j.at("velocity");
  }
  if (j.contains("zoneSetId"))
  {
    d.zone_set_id = j.at("zoneSetId");
  }
}
}  // namespace types

}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__STATE_HPP_
