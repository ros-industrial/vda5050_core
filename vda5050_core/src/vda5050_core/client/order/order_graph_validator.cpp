/**
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

#include <algorithm>
#include <unordered_map>

#include "vda5050_core/client/order/order_graph_validator.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_types/edge.hpp"
#include "vda5050_types/error_level.hpp"

namespace vda5050_core {
namespace order {

//=============================================================================
ValidationResult OrderGraphValidator::is_valid_graph(
  const vda5050_types::Order& order) noexcept(false)
{
  ValidationResult res;
  const std::string order_id = order.order_id;
  const std::string update_id = std::to_string(order.order_update_id);

  auto add_error =
    [&](
      const std::string& msg,
      const std::vector<vda5050_types::ErrorReference>& refs = {}) {
      std::vector<vda5050_types::ErrorReference> all_refs = {
        {"order.order_id", order_id}, {"order.order_update_id", update_id}};
      all_refs.insert(all_refs.end(), refs.begin(), refs.end());
      VDA5050_ERROR("Order Validation Error: {}", msg);
      res.errors.push_back(vda5050_types::Error{
        "Order Validation Error", std::move(all_refs), msg,
        vda5050_types::ErrorLevel::WARNING});
    };

  // check if there are exusting nodes, stop if empty
  if (order.nodes.empty())
  {
    add_error("order does not have any nodes!");
    return res;
  }

  // check if number of order is n, and number of edge is n-1
  if (order.nodes.size() != order.edges.size() + 1)
  {
    add_error(
      "Invalid graph. Found " + std::to_string(order.nodes.size()) +
      " nodes and " + std::to_string(order.edges.size()) +
      " edges. Expected exactly " + std::to_string(order.nodes.size() - 1) +
      " edges.");
    return res;
  }

  // check if order_update_id is 0, (means first update of its order)
  // reject if first node sequence id is > 0
  if (order.order_update_id == 0 && order.nodes.front().sequence_id != 0)
  {
    add_error(
      "Initial order (update_id=0) must start with sequence id 0, but found " +
        std::to_string(order.nodes.front().sequence_id),
      {{"node.node_id", order.nodes.front().node_id},
       {"node.sequence_id", std::to_string(order.nodes.front().sequence_id)}});
    return res;
  }

  bool horizon_reached = false;
  std::optional<uint32_t> last_base_seq;

  // iterate through nodes and edges
  for (size_t i(0u); i < order.nodes.size(); ++i)
  {
    const auto& curr_node = order.nodes[i];

    // check if node sequence id is even
    if (curr_node.sequence_id & 1u)
    {
      add_error(
        "Order Node sequence contains an odd sequence id",
        {{"node.sequence_id", std::to_string(curr_node.sequence_id)}});
    }

    // check if base/horizon separation
    if (curr_node.released)
    {
      if (horizon_reached)
        add_error(
          "Order contains a base sequence id after a horizon sequence id");
      last_base_seq = curr_node.sequence_id;
    }
    else
    {
      horizon_reached = true;
    }

    // validate edges
    // check an edge if curr_node is not last
    if (i < order.edges.size())
    {
      const auto& edge = order.edges[i];

      // continuity check: edge sequence_id must be node sequence_id + 1
      if (edge.sequence_id != curr_node.sequence_id + 1)
      {
        add_error(
          "Missing sequence id or unsorted data. Expected edge " +
          std::to_string(curr_node.sequence_id + 1) + " but found " +
          std::to_string(edge.sequence_id));
      }

      // check if edge is odd
      if (!(edge.sequence_id & 1u))
      {
        add_error(
          "Order Edge sequence contains an even sequence id",
          {{"edge.sequence_id", std::to_string(edge.sequence_id)}});
      }

      // check if start node id of edge is the current node_id
      if (edge.start_node_id != curr_node.node_id)
      {
        add_error(
          "Edge start_node_id does not match the preceding node ID",
          {{"edge.edge_id", edge.edge_id},
           {"node.node_id", curr_node.node_id}});
      }

      // check if end node id of edge is the next node_id
      const auto& next_node = order.nodes[i + 1];

      if (edge.end_node_id != next_node.node_id)
      {
        add_error(
          "Edge end_node_id does not match the following node ID",
          {{"edge.edge_id", edge.edge_id},
           {"next_node.node_id", next_node.node_id}});
      }

      // next node sequence_id must be Edge sequence_id + 1
      if (next_node.sequence_id != edge.sequence_id + 1)
      {
        add_error(
          "Missing sequence id or unsorted data. Expected node " +
          std::to_string(edge.sequence_id + 1) + " but found " +
          std::to_string(next_node.sequence_id));
      }

      // edge base/horizon check, cannot have base (released node) after
      // horizon (unreleased node)
      if (edge.released)
      {
        if (horizon_reached)
          add_error(
            "Order contains a base sequence id after a horizon sequence id");
        last_base_seq = edge.sequence_id;
      }
      else
      {
        horizon_reached = true;
      }
    }
  }

  // If the last thing released was edge, order is invalid.
  if (last_base_seq && (*last_base_seq & 1u))
  {
    add_error(
      "The base (released graph) ends with an edge; it must end with a node.");
  }

  return res;
}

//=============================================================================
ValidationResult OrderGraphValidator::is_valid_order_update(
  const vda5050_types::Order& base_order,
  const vda5050_types::Order& next_order) noexcept(false)
{
  ValidationResult res;

  const std::string base_order_id = base_order.order_id;
  const std::string base_order_update_id =
    std::to_string(base_order.order_update_id);

  const std::string next_order_id = next_order.order_id;
  const std::string next_order_update_id =
    std::to_string(next_order.order_update_id);

  auto add_error =
    [&](
      const std::string& msg,
      const std::vector<vda5050_types::ErrorReference>& refs = {}) {
      std::vector<vda5050_types::ErrorReference> all_refs = {
        {"base_order.order_id", base_order_id},
        {"next_order.id", next_order_id}};
      all_refs.insert(all_refs.end(), refs.begin(), refs.end());
      VDA5050_ERROR("Order Validation Error: {}", msg);
      res.errors.push_back(vda5050_types::Error{
        "Order Validation Error", std::move(all_refs), msg,
        vda5050_types::ErrorLevel::WARNING});
    };
  // check validity of next order
  ValidationResult next_res = OrderGraphValidator::is_valid_graph(next_order);
  if (!next_res.errors.empty())
  {
    // Append errors from the sub-validation to the result
    res.errors.insert(
      res.errors.end(), next_res.errors.begin(), next_res.errors.end());
    add_error("The incoming update order itself is invalid.");
    return res;
  }
  // check validitiy of current order
  ValidationResult base_res = OrderGraphValidator::is_valid_graph(base_order);
  if (!base_res.errors.empty())
  {
    add_error("The internal base order is invalid state. Cannot append.");
    return res;
  }

  // check if order id matches
  if (base_order.order_id != next_order.order_id)
  {
    add_error(
      "Order IDs do not match (" + base_order.order_id + " vs " +
      next_order.order_id + ")");
    return res;
  }

  // order update id must be increasing
  if (next_order.order_update_id < base_order.order_update_id)
  {
    add_error(
      "Update ID must be greater than base. Base: " +
      std::to_string(base_order.order_update_id) +
      ", Next: " + std::to_string(next_order.order_update_id));
    return res;
  }

  // The first node of the order update corresponds
  // to the last shared base node of the previous order message.
  // @ VDA 5050 Version 2.1.0, January 2025 (Page 16)

  // find the last released node of base order
  auto it = std::find_if(
    base_order.nodes.rbegin(), base_order.nodes.rend(),
    [](const auto& node) { return node.released; });

  // check if there's a released node
  // maybe put to is_valid_graph()
  if (it == base_order.nodes.rend())
  {
    add_error("Base order has no released nodes to stitch onto.");
    return res;
  }

  const auto& last_released_node = *it;
  const auto& next_first_node = next_order.nodes.front();

  // nnode must match (node_id and sequence_id)
  if (last_released_node != next_first_node)
  {
    add_error(
      "Graph Discontinuity: Base last released node is '" +
      last_released_node.node_id + "' but Update starts at node '" +
      next_first_node.node_id + "'");
  }

  return res;
}
//=============================================================================

}  // namespace order
}  // namespace vda5050_core
