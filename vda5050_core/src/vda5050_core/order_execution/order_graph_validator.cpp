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

#include <iostream>
#include <queue>
#include <vector>

#include "vda5050_core/order_execution/edge.hpp"
#include "vda5050_core/order_execution/node.hpp"
#include "vda5050_core/order_execution/order_graph_validator.hpp"

namespace vda5050_core {
namespace order_graph_validator {

/// TODO (shawnkchan) maybe this should take in an Order object instead so that we can specify which orderId is errornous if something goes wrong
/// TODO Refactor this to take in an Order object instead, will make validation logic easier
OrderGraphValidator::OrderGraphValidator() {}

bool OrderGraphValidator::is_valid_graph(
  std::vector<vda5050_core::node::Node>& nodes,
  std::vector<vda5050_core::edge::Edge>& edges)
{
  /// TODO: (shawnkchan) Check if it is even possible to send an Order with zero nodes (should Order class just protect against this?). If not possible, get rid of this.
  /// Does not seem to be any guarantee that this list is not empty, so will leave this check here.
  if (nodes.empty())
  {
    std::cerr << "Graph Validation Error: nodes vector is empty.";
    return false;
  }
  else
  {
    start_node_id = nodes.front().node_id();
    end_node_id = nodes.back().node_id();
  }

  if (nodes.size() == 1)
  {
    return true;
  }

  /// check for validity based on vector sizes
  if (!(nodes.size() - 1 == edges.size()))
  {
    std::cerr << "Graph Validation Error: Difference in number of nodes and "
                 "edges is not 1."
              << '\n';
    return false;
  }

  if (!is_in_traversal_order(nodes, edges))
  {
    std::cerr << "Nodes and edges are not in traversal order." << '\n';
    return false;
  }

  if (!is_valid_edges(edges))
  {
    std::cerr << "Graph Validation Error: startNodeId and endNodeId of an edge "
                 "does not match the startNodeId and endNodeId of this order."
              << '\n';
    return false;
  }

  return true;
}

bool OrderGraphValidator::is_in_traversal_order(
  std::vector<vda5050_core::node::Node>& nodes,
  std::vector<vda5050_core::edge::Edge>& edges)
{
  std::queue<vda5050_core::node::Node> node_queue;
  std::queue<vda5050_core::edge::Edge> edge_queue;

  for (vda5050_core::node::Node n : nodes)
  {
    node_queue.push(n);
  }

  for (vda5050_core::edge::Edge e : edges)
  {
    edge_queue.push(e);
  }

  vda5050_core::node::Node first_node{node_queue.front()};
  node_queue.pop();
  uint32_t latest_sequence_id{first_node.sequence_id()};
  while (node_queue.size() != 0 && edge_queue.size() != 0)
  {
    if (node_queue.size() > edge_queue.size())
    {
      vda5050_core::node::Node n{node_queue.front()};
      if (latest_sequence_id == n.sequence_id() - 1)
      {
        node_queue.pop();
        latest_sequence_id = n.sequence_id();
      }

      else
      {
        return false;
      }
    }

    /// if there are an equal number of edges and nodes, then check the next edge in the queue
    else if (node_queue.size() == edge_queue.size())
    {
      vda5050_core::edge::Edge e{edge_queue.front()};
      if (latest_sequence_id == e.sequence_id() - 1)
      {
        edge_queue.pop();
        latest_sequence_id = e.sequence_id();
      }

      else
      {
        return false;
      }
    }
  }
  return true;
}

bool OrderGraphValidator::is_valid_edges(
  std::vector<vda5050_core::edge::Edge>& edges)
{
  for (vda5050_core::edge::Edge e : edges)
  {
    if (
      e.start_node_id() != start_node_id &&
      e.end_node_id() != end_node_id)
    {
      return false;
    }
  }
  return true;
}

}  // namespace order_graph_validator
}  // namespace vda5050_core
