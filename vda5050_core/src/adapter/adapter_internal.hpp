/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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

#ifndef VDA5050_CORE__ADAPTER__ADAPTER_INTERNAL_HPP_
#define VDA5050_CORE__ADAPTER__ADAPTER_INTERNAL_HPP_

#include <cstdint>
#include <mutex>
#include <string>

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/types/node.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"
namespace vda5050_core {

namespace adapter {

struct AgvState
{
  mutable std::mutex mutex;
  types::State state;
};


struct OrderUpdate : public execution::Initialize<OrderUpdate, execution::UpdateBase>
{
  types::Order order;

  explicit OrderUpdate(types::Order order) : order(std::move(order))
  {
    // Nothing to do here ...
  }
};

struct NodeDispatchEvent
: public execution::Initialize<NodeDispatchEvent, execution::EventBase>
{
  types::Node node;

  explicit NodeDispatchEvent(types::Node node) : node(std::move(node))
  {
    // Nothing to do here ...
  }
};

struct NodeAckUpdate : public execution::Initialize<NodeAckUpdate, execution::UpdateBase>
{
  std::string node_id;
  uint32_t sequence_id;

  NodeAckUpdate(std::string node_id, uint32_t sequence_id)
  : node_id(std::move(node_id)), sequence_id(sequence_id)
  {
    // Nothing to do here ...
  }
};

}  // namespace adapter
}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__ADAPTER_INTERNAL_HPP_
