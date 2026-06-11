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

#ifndef VDA5050_CORE__CLIENT__STRATEGIES__ORDER_TRAVERSAL_HPP_
#define VDA5050_CORE__CLIENT__STRATEGIES__ORDER_TRAVERSAL_HPP_

#include <memory>

#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/strategy_interface.hpp"

namespace vda5050_core {

namespace client {

/// \brief Walks the released base of an accepted order.
///
/// On each `NodeReachedUpdate` it runs the traversal cascade against the
/// `OrderExecutionResource`: set lastNode, drop the reached node's state and the
/// incoming edge's state, then dispatch the next released node via a
/// `NavigateToNodeEvent`. It stops at the decision point (the last released base
/// node) and never enters the horizon, raising `new_base_request` when the
/// released base runs low.
///
/// Action execution and blockingType concurrency are handled by the actions
/// strategy; this strategy advances node/edge state and dispatches navigation.
class OrderTraversal : public execution::StrategyInterface
{
public:
  OrderTraversal();

  void init(std::shared_ptr<execution::ContextInterface> context) override;

  void step(std::shared_ptr<execution::ContextInterface> context) override;
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__STRATEGIES__ORDER_TRAVERSAL_HPP_
