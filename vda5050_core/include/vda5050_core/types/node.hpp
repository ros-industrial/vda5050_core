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
#include <vector>

#include "vda5050_core/types/action.hpp"
#include "vda5050_core/types/node_position.hpp"

namespace vda5050_core {

namespace types {

/// \brief Defines a single node in an Order message.
struct Node
{
  /// \brief Unique node identification
  std::string node_id;

  /// \brief Number to track the sequence of nodes and edges in an order and to
  /// simplify order updates. Distinguishes between nodes that may be passed
  /// more than once in the same order. Runs across all nodes/edges of the same
  /// order and resets when a new orderId is issued.
  uint32_t sequence_id;

  /// \brief "true" indicates that the node is part of the base
  /// "false" indicates that the node is part of the horizon
  bool released;

  /// \brief Array of actions to be executed at this node.
  /// Empty array if no actions are required.
  std::vector<Action> actions;

  /// \brief Node position, required only for vehicle-types that need
  /// node positions. (e.g., not required for line-guided vehicles)
  std::optional<NodePosition> node_position;

  /// \brief Additional information about the node
  std::optional<std::string> node_description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Node& other) const
  {
    if (this->node_id != other.node_id) return false;
    if (this->sequence_id != other.sequence_id) return false;
    if (this->released != other.released) return false;
    if (this->actions != other.actions) return false;
    if (this->node_position != other.node_position) return false;
    if (this->node_description != other.node_description) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Node& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__NODE_HPP_
