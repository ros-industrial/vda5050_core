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

#ifndef VDA5050_CORE__TYPES__ORDER_HPP_
#define VDA5050_CORE__TYPES__ORDER_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/edge.hpp"
#include "vda5050_core/types/header.hpp"
#include "vda5050_core/types/node.hpp"

namespace vda5050_core {

namespace types {

/// \brief Defines the message used to communicate from master control to the AGV
struct Order
{
  /// \brief Message header that contains the common fields required across all messages
  Header header;

  /// \brief Order identification used to identify multiple order messages that belongs
  /// to the same order.
  std::string order_id;

  /// \brief Order update identification used to identify multiple order messages that
  /// belong to the same order.
  uint32_t order_update_id;

  /// \brief An array of Node messages to be traversed for fulfilling the order.
  /// A valid order consists of at least one Node.
  /// For the case of only one Node, leave the Edge array empty.
  std::vector<Node> nodes;

  /// \brief An array of Edge objects to be traversed for fulfilling the order.
  std::vector<Edge> edges;

  /// \brief A unique identifier of the zone set that the AGV has to use for navigation
  /// or that was used by master control for planning.
  /// If no zones are used, leave empty.
  std::optional<std::string> zone_set_id;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Order& other) const
  {
    if (this->header != other.header) return false;
    if (this->order_id != other.order_id) return false;
    if (this->order_update_id != other.order_update_id) return false;
    if (this->nodes != other.nodes) return false;
    if (this->edges != other.edges) return false;
    if (this->zone_set_id != other.zone_set_id) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Order& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ORDER_HPP_
