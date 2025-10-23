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

#ifndef VDA5050_CORE__TYPES__NODE_HPP_
#define VDA5050_CORE__TYPES__NODE_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "vda5050_core/types/action.hpp"
#include "vda5050_core/types/node_position.hpp"

namespace vda5050_core {

namespace types {

/// @struct Node
/// @brief  A Node represents a specific location or point in the map
struct Node
{
  /// \brief Unique node identification.
  std::string node_id;

  /// \brief Number to track the sequence of nodes and edges in an
  ///        order and to simplify order updates. The main purpose is
  ///        to distinguish between a node which is passed more than
  ///        once within one orderId. The variable sequence_id runs
  ///        across all nodes and edges of the same order and is reset
  ///        when a new orderId is issued.
  uint32_t sequence_id = 0;

  /// \brief Additional information on the node
  std::optional<std::string> node_description;

  /// \brief True indicates that the node is part of the base.
  /// False indicates that the node is part of the horizon.
  bool released = false;

  /// \brief Node position Optional for vehicle-types that do not
  /// require the node position (e.g. line-guided vehicles).
  std::optional<NodePosition> node_position;

  /// \brief Array of actions to be executed in node.Empty array if no
  ///        actions required. An action triggered by a node will
  ///        persist until changed in another node unless restricted by
  ///        durationType/durationValue.
  std::vector<Action> actions;

  /// \brief Compares two Node objects for equality.
  /// \param other The Node instance to compare with.
  /// \return True all fields are equal; otherwise false.
  inline bool operator==(const Node& other) const
  {
    if (this->actions != other.actions) return false;
    if (this->node_description != other.node_description) return false;
    if (this->node_id != other.node_id) return false;
    if (this->node_position != other.node_position) return false;
    if (this->released != other.released) return false;
    if (this->sequence_id != other.sequence_id) return false;

    return true;
  }

  /// \brief Compares two Node objects for inequality.
  /// \param other The Node instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const Node& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Node& d)
{
  j["actions"] = d.actions;
  if (d.node_description.has_value())
  {
    j["nodeDescription"] = *d.node_description;
  }
  j["nodeId"] = d.node_id;
  if (d.node_position.has_value())
  {
    j["nodePosition"] = *d.node_position;
  }
  j["released"] = d.released;
  j["sequenceId"] = d.sequence_id;
}

inline void from_json(const json& j, Node& d)
{
  d.actions = j.at("actions").get<std::vector<Action>>();
  if (j.contains("nodeDescription"))
  {
    d.node_description = j.at("nodeDescription");
  }
  d.node_id = j.at("nodeId");
  if (j.contains("nodePosition"))
  {
    d.node_position = j.at("nodePosition");
  }
  d.released = j.at("released");
  d.sequence_id = j.at("sequenceId");
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__NODE_HPP_
