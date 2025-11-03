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

#ifndef VDA5050_CORE__ORDER_EXECUTION__ORDER_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__ORDER_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "vda5050_core/order_execution/edge.hpp"
#include "vda5050_core/order_execution/node.hpp"

namespace vda5050_core {

namespace order {

/// \brief Class that represents a VDA5050 Order
class Order
{
public:
  /// \brief Order constructor
  ///
  /// \param order_id String that uniquely identifies this order
  /// \param order_update_id unit32 designating the orderUpdateId of this order
  /// \param nodes A vector of Node objects belonging to this order
  /// \param edges A vector of Edge objects belonging to this order
  Order(
    std::string order_id, uint32_t order_update_id,
    std::vector<node::Node> nodes, std::vector<edge::Edge> edges);

  std::string order_id() const
  {
    return order_id_;
  }

  uint32_t order_update_id() const
  {
    return order_update_id_;
  }

  const std::vector<node::Node>& nodes() const
  {
    return nodes_;
  }

  const std::vector<edge::Edge>& edges() const
  {
    return edges_;
  }

  const std::vector<order_graph_element::OrderGraphElement>& graph() const
  {
    return graph_;
  }

  const std::vector<order_graph_element::OrderGraphElement>& base() const
  {
    return base_;
  }

  const std::vector<order_graph_element::OrderGraphElement>& horizon() const
  {
    return horizon_;
  }
 
  const node::Node& decision_point() const
  {
    return decision_point_;
  }

  /// \brief Stitch this order with another order and update the order_update_id if stitching is successful.
  ///
  /// \param order The incoming order to stitch to this order.
  void stitch_and_set_order_update_id(order::Order order);

private:
  /// \brief The orderId of this order.
  std::string order_id_;

  /// \brief The orderUpdateId of this order.
  uint32_t order_update_id_;

  /// \brief The nodes contained within this order.
  std::vector<node::Node> nodes_;

  /// \brief The edges contained within this order.
  std::vector<edge::Edge> edges_;

  /// \brief The graph created by the nodes and edges of this order.
  std::vector<order_graph_element::OrderGraphElement> graph_;

  /// \brief The base of this order. Contains the released nodes and edges sorted in ascending sequenceId
  std::vector<order_graph_element::OrderGraphElement> base_;

  /// \brief The horizion of this order. Contains the unreleased nodes and edges sorted in ascending sequenceId
  std::vector<order_graph_element::OrderGraphElement> horizon_;

  /// \brief The last node in this order's base, or the last released node according to sequenceId
  node::Node decision_point_;

  /// \brief Populate the graph_ member variable.
  void populate_graph();

  /// \brief Idempotent function to populate the base_ member variable with all released nodes and edges.
  void populate_base();

  /// \brief Idempotent function to populate the horizon_ member variable with all unreleased nodes and edges.
  void populate_horizon();

  /// \brief Stitch this order with another order.
  void stitch_order(order::Order order);

  /// \brief Set a new order_update_id.
  ///
  /// \param order_update_id the new order_update_id.
  void set_order_update_id(uint32_t order_update_id);

  /// \brief Set decision_point_ to the last released node in the order.
  ///
  /// \param nodes The nodes of this order.
  node::Node set_decision_point(std::vector<node::Node> nodes);
};

}  // namespace order
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ORDER_EXECUTION__ORDER_HPP_
