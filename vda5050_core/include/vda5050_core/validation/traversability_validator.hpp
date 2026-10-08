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

#ifndef VDA5050_CORE__VALIDATION__TRAVERSABILITY_VALIDATOR_HPP_
#define VDA5050_CORE__VALIDATION__TRAVERSABILITY_VALIDATOR_HPP_

#include "vda5050_core/errors/validation_result.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/validation/pre_send_validator.hpp"

namespace vda5050_core {

namespace validation {

/// \brief Validate that the AGV is on, or within the deviation range of, the
/// order's first node. Applies to new orders only.
vda5050_core::errors::ValidationResult validate_reachability(
  const PreSendContext& ctx, const vda5050_core::types::Order& order);

/// \brief Validate the order's nodes and edges against the loaded layout:
/// existence, map_id, and edge direction. Warns when no layout is loaded.
vda5050_core::errors::ValidationResult validate_graph_integrity(
  const PreSendContext& ctx, const vda5050_core::types::Order& order);

}  // namespace validation
}  // namespace vda5050_core

#endif  // VDA5050_CORE__VALIDATION__TRAVERSABILITY_VALIDATOR_HPP_
