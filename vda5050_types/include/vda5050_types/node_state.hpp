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

/// @struct NodeState
/// @brief  Array of nodeState-Objects, that need to be traversed for fulfilling the order. Empty list if idle.
struct NodeState
{
  /// @brief Unique node identification
  std::string node_id;

  /// @brief sequenceId to discern multiple nodes with same nodeId
  uint32_t sequence_id = 0;

  /// @brief Additional information on the node
  std::optional<std::string> node_description;

  /// @brief Node position. The object is defined in chapter 5.4 Topic: Order (from master control to AGV).
  ///        Optional:Master control has this information. Can be sent additionally, e.g., for debugging purposes..
  std::optional<NodePosition> node_position;

  /// @brief  "True: indicates that the node is part of the base. False: indicates that the node is part of the horizon.
  bool released = false;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__NODE_STATE_HPP_
