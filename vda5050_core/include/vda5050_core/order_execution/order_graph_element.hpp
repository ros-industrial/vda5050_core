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

#ifndef VDA5050_CORE__ORDER_EXECUTION__ORDER_GRAPH_ELEMENT_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__ORDER_GRAPH_ELEMENT_HPP_

#include <cstdint>
#include <string>

namespace vda5050_core {

namespace order_graph_element {

/// \brief Parent class representing a graph element (Node or Edge) of a VDA5050 order
class OrderGraphElement
{
public:
  /// \brief OrderGraphElement constructor
  ///
  /// \param sequence_id uint32 indicating the sequence number of this graph element in an order.
  /// \param released Boolean indicating true if this graph element is part of the base, false if it is part of the horizon.
  OrderGraphElement(uint32_t sequence_id, bool released);

  uint32_t sequence_id() const
  {
    return sequence_id_;
  }

  bool released() const
  {
    return released_;
  }

  /// \brief Custom < operator to enable sorting of OrderGraphElement objects by sequence_id
  ///
  /// \param order_graph_element The other OrderGraphElement to be compared to
  ///
  /// \return True if sequenceId of this OrderGraphElement is smaller than the one it is compared to.
  bool operator<(const OrderGraphElement& order_graph_element) const
  {
    return sequence_id_ < order_graph_element.sequence_id();
  }

protected:
  /// \brief uint32 indicating the sequence number of this graph element in an order.
  uint32_t sequence_id_;

  /// \brief Boolean indicating true if this graph element is part of the base, false if it is part of the horizon.
  bool released_;

  /// TODO (shawnkchan) add array of actions as another common attribute.
};

}  // namespace order_graph_element
}  // namespace vda5050_core
#endif  // VDA5050_CORE__ORDER_EXECUTION__ORDER_GRAPH_ELEMENT_HPP_
