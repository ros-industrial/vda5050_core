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

#ifndef VDA5050_CORE__ORDER_EXECUTION__ORDER_GRAPH_VALIDATOR_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__ORDER_GRAPH_VALIDATOR_HPP_

#include <vector>

#include "vda5050_core/order_execution/edge.hpp"
#include "vda5050_core/order_execution/node.hpp"

namespace vda5050_core {
namespace order_graph_validator {

/// \brief Utility class with functions to perform validity checks on the graph contained in a VDA5050 Order message
class OrderGraphValidator
{
public:
  OrderGraphValidator();

  /// \brief Checks that the nodes and edges in a VDA5050 Order form a valid graph according to the VDA5050 specification sheet.
  ///
  /// \return True if nodes and edges create a valid graph, false otherwise
  bool is_valid_graph(
    std::vector<vda5050_core::node::Node>& nodes,
    std::vector<vda5050_core::edge::Edge>& edges);

private:
  // std::vector<vda5050_core::node::Node>& nodes;
  // std::vector<vda5050_core::edge::Edge>& edges;

  std::string start_node_id;
  std::string end_node_id;

  /// \brief Checks that the nodes and edges contained in a VDA5050 Order are arranged according to their sequenceId
  ///
  /// \return True if nodes and edges are arranged according to their sequenceId, false otherwise
  bool is_in_traversal_order(
    std::vector<vda5050_core::node::Node>& nodes,
    std::vector<vda5050_core::edge::Edge>& edges);

  /// \brief Checks that startNodeId and endNodeId of all edges in a VDA5050 Order match with its the start and end nodeIds
  ///
  /// \return True if all edges' startNodeId and endNodId match, false otherwise
  bool is_valid_edges(std::vector<vda5050_core::edge::Edge>& edges);
};

}  // namespace order_graph_validator
}  // namespace vda5050_core
#endif  // VDA5050_CORE__ORDER_EXECUTION__ORDER_GRAPH_VALIDATOR_HPP_
