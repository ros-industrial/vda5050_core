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

#ifndef VDA5050_CORE__TYPES__NODE_STATE_HPP_
#define VDA5050_CORE__TYPES__NODE_STATE_HPP_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "vda5050_core/types/node_position.hpp"

namespace vda5050_core {

namespace types {

/// \struct NodeState
/// \brief  Array of nodeState-Objects, that need to be traversed for fulfilling the order. Empty list if idle.
struct NodeState
{
  /// \brief Unique node identification
  std::string node_id;

  /// \brief sequenceId to discern multiple nodes with same nodeId
  uint32_t sequence_id = 0;

  /// \brief Additional information on the node
  std::optional<std::string> node_description;

  /// \brief Node position. The object is defined in chapter 5.4 Topic: Order (from master control to AGV).
  ///        Optional:Master control has this information. Can be sent additionally, e.g., for debugging purposes..
  std::optional<NodePosition> node_position;

  /// \brief  "True: indicates that the node is part of the base. False: indicates that the node is part of the horizon.
  bool released = false;

  /// \brief Compares two NodeState objects for equality.
  /// \param other The NodeState instance to compare with.
  /// \return True if node_description, node_id, node_position, released, and sequence_id are equal, otherwise false.
  inline bool operator==(const NodeState& other) const
  {
    if (this->node_description != other.node_description) return false;
    if (this->node_id != other.node_id) return false;
    if (this->node_position != other.node_position) return false;
    if (this->released != other.released) return false;
    if (this->sequence_id != other.sequence_id) return false;

    return true;
  }

  /// \brief Compares two NodeState objects for inequality.
  /// \param other The NodeState instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const NodeState& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const NodeState& n)
{
  j = json{
    {"node_id", n.node_id},
    {"sequence_id", n.sequence_id},
    {"released", n.released}};

  if (n.node_description.has_value())
  {
    j["node_description"] = n.node_description.value();
  }

  if (n.node_position.has_value())
  {
    j["node_position"] = n.node_position.value();
  }
}

inline void from_json(const json& j, NodeState& n)
{
  j.at("node_id").get_to(n.node_id);
  j.at("sequence_id").get_to(n.sequence_id);
  j.at("released").get_to(n.released);

  if (j.contains("node_description") && !j.at("node_description").is_null())
  {
    n.node_description = j.at("node_description").get<std::string>();
  }

  if (j.contains("node_position") && !j.at("node_position").is_null())
  {
    n.node_position = j.at("node_position").get<NodePosition>();
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__NODE_STATE_HPP_
