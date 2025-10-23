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

#include <nlohmann/json.hpp>

#include "vda5050_core/types/edge.hpp"
#include "vda5050_core/types/header.hpp"
#include "vda5050_core/types/node.hpp"

namespace vda5050_core {

namespace types {

/// @struct  Order
/// @brief A graph of nodes and edges, represents the complete mission
///        sent from the master control system
struct Order
{
  /// Message header
  Header header;

  /// \brief Order identification. This is to be used to identify
  ///        multiple order messages that belong to the same order.
  std::string order_id;

  /// \brief orderUpdate identification. Is unique per orderId. If an order
  ///        update is rejected, this field is to be passed in the rejection
  ///        message
  uint32_t order_update_id = 0;

  /// \brief Unique identifier of the zone set that the AGV has to use for
  ///        navigation or that was used by MC for planning.
  std::optional<std::string> zone_set_id;

  /// \brief Array of nodes objects to be traversed for fulfilling the order.
  ///        One node is enough for a valid order. Leave edge list empty for
  ///        that case.
  std::vector<Node> nodes;

  /// \brief Array of edges to be traversed for fulfilling the order. May
  ///        be empty in case only one node is used for an order.
  std::vector<Edge> edges;

  /// \brief Compares two Order objects for equality.
  /// \param other The Order instance to compare with.
  /// \return True all fields are equal; otherwise false.
  inline bool operator==(const Order& other) const
  {
    if (this->edges != other.edges) return false;
    if (this->header != other.header) return false;
    if (this->nodes != other.nodes) return false;
    if (this->order_id != other.order_id) return false;
    if (this->order_update_id != other.order_update_id) return false;
    if (this->zone_set_id != other.zone_set_id) return false;

    return true;
  }

  /// \brief Compares two Order objects for inequality.
  /// \param other The Order instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const Order& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Order& d)
{
  to_json(j, d.header);
  j["edges"] = d.edges;
  j["nodes"] = d.nodes;
  j["orderId"] = d.order_id;
  j["orderUpdateId"] = d.order_update_id;
  if (d.zone_set_id.has_value())
  {
    j["zoneSetId"] = *d.zone_set_id;
  }
}

inline void from_json(const json& j, Order& d)
{
  from_json(j, d.header);
  d.edges = j.at("edges").get<std::vector<Edge>>();
  d.nodes = j.at("nodes").get<std::vector<Node>>();
  d.order_id = j.at("orderId");
  d.order_update_id = j.at("orderUpdateId");
  if (j.contains("zoneSetId"))
  {
    d.zone_set_id = j.at("zoneSetId");
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ORDER_HPP_
