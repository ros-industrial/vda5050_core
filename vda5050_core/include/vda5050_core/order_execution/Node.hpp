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

#ifndef VDA5050_CORE__ORDER_EXECUTION__NODE_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__NODE_HPP_

#include <cstdint>
#include <string>

#include "vda5050_core/order_execution/OrderGraphElement.hpp"

namespace vda5050_core {

namespace node {

/// \brief Class that represents a VDA5050 Node
class Node : public order_graph_element::OrderGraphElement
{
public:
  using Ptr = std::shared_ptr<Node>; 
  /// \brief Node constructor
  ///
  /// \param sequence_id uint32 indicating the sequence number of this node in an order
  /// \param released Boolean indicating true if this node is part of the base, false if it is part of the horizon
  /// \param node_id String that uniquely identifies this node.
  Node(uint32_t sequence_id, bool released, std::string node_id);

  std::string node_id() const
  {
    return node_id_;
  }

private:
  /// \brief String that uniquely identifies this node.
  std::string node_id_;
};

}  // namespace node
}  // namespace vda5050_core
#endif  // VDA5050_CORE__ORDER_EXECUTION__NODE_HPP_
