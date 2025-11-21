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

#ifndef VDA5050_TYPES__NODE_STATE_HPP_
#define VDA5050_TYPES__NODE_STATE_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include "vda5050_types/node_position.hpp"

namespace vda5050_types {

/// \brief Provides information about the node to be traversed
/// for fulfilling the order
/// Part of the state message
struct NodeState
{
  /// \brief Unique node identification
  std::string node_id;

  /// \brief A sequence identification to differentiate
  /// multiple nodes with the same ID
  uint32_t sequence_id = 0;

  /// \brief Additional information on the node
  std::optional<std::string> node_description;

  /// \brief Position of the node on the active AGV map in
  /// world coordinate system
  std::optional<NodePosition> node_position;

  /// \brief Defines if the node is part of the base or horizon
  ///    - True indicates node is part of the base
  ///    - False indicates node is part of the horizon
  bool released = false;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const NodeState& other) const
  {
    if (this->node_description != other.node_description) return false;
    if (this->node_id != other.node_id) return false;
    if (this->node_position != other.node_position) return false;
    if (this->released != other.released) return false;
    if (this->sequence_id != other.sequence_id) return false;

    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const NodeState& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__NODE_STATE_HPP_
