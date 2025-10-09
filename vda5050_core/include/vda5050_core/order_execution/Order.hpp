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

#include <string>
#include <vector>
#include <cstdint>

#include "order_execution/Edge.hpp"
#include "order_execution/Node.hpp"

namespace vda5050_core {

namespace order {

class Order
{
public:
  Order(std::string order_id, uint32_t order_update_id, std::vector<node::Node> nodes, std::vector<edge::Edge> edges);

  std::string order_id()
  {
    return order_id_;
  }

  uint32_t order_update_id()
  {
    return order_update_id_;
  }

  std::vector<node::Node>& nodes()
  {
    return nodes_;
  }

  std::vector<edge::Edge>& edges()
  {
    return edges_;
  }

  std::vector<order_graph_element::OrderGraphElement>& graph()
  {
    return graph_;
  }

  std::vector<order_graph_element::OrderGraphElement>& base()
  {
    return base_;
  }
  
  std::vector<order_graph_element::OrderGraphElement>& horizon()
  {
    return horizon_;
  }

  /// TODO: Is it safe to assume that the base is guaranteed to end with a Node? What should be responsible for error checking?
  /// \brief Get the base's last node. Note that base is guaranteed to end with a Node object.
  ///
  /// \return The last node of the order's base
  const node::Node& decision_point() const
  {
   return static_cast<const node::Node&>(base_.back());
  }

  /// \brief Update the Order's current horizon
  void set_horizon(std::vector<order_graph_element::OrderGraphElement>& new_horizon);

  /// @brief Stitch this order with another order and update the order_update_id if stitching is successful
  /// @param order 
  void stitch_and_set_order_update_id(order::Order order);
  
private:
  std::string order_id_;
  uint32_t order_update_id_;
  std::vector<node::Node> nodes_;
  std::vector<edge::Edge> edges_;
  std::vector<order_graph_element::OrderGraphElement> graph_;
  std::vector<order_graph_element::OrderGraphElement> base_;
  std::vector<order_graph_element::OrderGraphElement> horizon_;

  /// @brief Populate the graph_ member variable
  void populate_graph();

  /// \brief Idempotent function to populate the base_ member variable with all released nodes and edges. 
  void populate_base();
  
  /// \brief Idempotent function to populate the horizon_ member variable with all unreleased nodes and edges
  void populate_horizon();

  /// \brief Stitch this order with another order.
  void stitch_order(order::Order order);

  /// \brief Set a new order_update_id.
  /// \param order_update_id the new order_update_id.
  void set_order_update_id(uint32_t order_update_id);

};

}  // namespace order
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ORDER_EXECUTION__ORDER_HPP_
