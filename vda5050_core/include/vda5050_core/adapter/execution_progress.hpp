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

#ifndef VDA5050_CORE__ADAPTER__EXECUTION_PROGRESS_HPP_
#define VDA5050_CORE__ADAPTER__EXECUTION_PROGRESS_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "vda5050_core/types/edge_state.hpp"
#include "vda5050_core/types/node_state.hpp"

namespace vda5050_core {

namespace adapter {

struct ExecutionProgress
{
  std::string order_id;
  uint32_t order_update_id;

  std::vector<types::NodeState> node_states;
  std::vector<types::EdgeState> edge_states;

  std::size_t current_node_index = 0;
};

}  // namespace adapter

}  // namespace vda5050_core

#endif  // VDA5050_CORE__ADAPTER__EXECUTION_PROGRESS_HPP_
