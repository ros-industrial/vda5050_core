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

#ifndef VDA5050_CORE__CLIENT__EVENTS__NODE_REACHED_HPP_
#define VDA5050_CORE__CLIENT__EVENTS__NODE_REACHED_HPP_

#include <cstdint>
#include <string>
#include <utility>

#include "vda5050_core/execution/base.hpp"

namespace vda5050_core {

namespace client {

/// \brief Emitted by the traversal strategy once a node has been reached.
///
/// Internal, validated counterpart of the inbound `NodeReachedUpdate`: the
/// update is the raw driver signal, this event is emitted only after the
/// traversal strategy confirms the node as the next expected one and advances
/// the base. The action strategy hooks it to trigger the node's actions, keeping
/// action triggering in lockstep with traversal.
struct NodeReachedEvent
: public execution::Initialize<NodeReachedEvent, execution::EventBase>
{
  std::string node_id;
  uint32_t sequence_id = 0;

  NodeReachedEvent(std::string node_id, uint32_t sequence_id)
  : node_id(std::move(node_id)), sequence_id(sequence_id)
  {
  }
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__EVENTS__NODE_REACHED_HPP_
