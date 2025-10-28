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

#include <cstdint>
#include <string>

#include "vda5050_core/order_execution/edge.hpp"
#include "vda5050_core/order_execution/order_graph_element.hpp"

namespace vda5050_core {
namespace edge {

Edge::Edge(
  uint32_t sequence_id, bool released, std::string edge_id,
  std::string start_node_id, std::string end_node_id)
: order_graph_element::OrderGraphElement(sequence_id, released),
  edge_id_{edge_id},
  start_node_id_{start_node_id},
  end_node_id_{end_node_id}
{
}

}  // namespace edge
}  // namespace vda5050_core
