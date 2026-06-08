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

#ifndef VDA5050_CORE__CLIENT__STRATEGIES__ORDER_ACTIONS_HPP_
#define VDA5050_CORE__CLIENT__STRATEGIES__ORDER_ACTIONS_HPP_

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/client/events/edge_entered.hpp"
#include "vda5050_core/client/events/edge_left.hpp"
#include "vda5050_core/client/events/node_traversed.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/engine.hpp"
#include "vda5050_core/execution/strategy_interface.hpp"
#include "vda5050_core/types/action.hpp"
#include "vda5050_core/types/action_status.hpp"

namespace vda5050_core {

namespace client {

/// \brief Outcome an executor reports after being asked to perform an action.
///
/// `status` is the resulting `actionStatus`: FINISHED/FAILED for a one-shot
/// action that completed synchronously, or RUNNING for a long-running action
/// that was started and whose completion is reported later (asynchronous
/// completion is a future extension). `result_description` is copied into the
/// matching `actionState`.
struct ActionExecution
{
  types::ActionStatus status = types::ActionStatus::FINISHED;
  std::optional<std::string> result_description;
};

/// \brief Hook that actually performs a single VDA5050 action.
///
/// Receives the full `Action` (type, id, parameters, blockingType) so the robot
/// configuration layer can dispatch on `action_type`. The core cannot know how
/// to perform a robot-specific action, so without a registered executor the
/// strategy leaves actions WAITING and warns.
using ActionExecutor = std::function<ActionExecution(const types::Action&)>;

/// \brief Triggers and tracks an order's node/edge actions.
///
/// Subscribes to the traversal cascade emitted by `OrderTraversal`
/// (`NodeTraversedEvent`, `EdgeEnteredEvent`, `EdgeLeftEvent`) and drives the
/// matching `actionState`s in the `OrderExecutionResource`:
/// - on node traversed: run the node's actions (WAITING -> RUNNING -> result);
/// - on edge entered: start the edge's actions;
/// - on edge left: stop the edge's still-running (time-bound) actions.
///
/// The full `Action` objects (type, parameters, blockingType) are read from the
/// persisted accepted order via `get_active_order()`; the state arrays only
/// carry `actionState`s. Work is driven synchronously from the source engine's
/// callbacks, so each event is handled exactly once in cascade order.
///
/// blockingType (NONE/SOFT/HARD) concurrency control and instantAction handling
/// (pause/resume/cancel) are not yet enforced and are tracked as follow-ups.
class OrderActions : public execution::StrategyInterface
{
public:
  /// \brief Construct against the engine that emits the traversal cascade.
  ///
  /// \param source The engine carrying the `OrderTraversal` events, i.e.
  ///   `order_traversal->engine()`. Subscriptions are registered in `init()`.
  explicit OrderActions(std::shared_ptr<execution::Engine> source);

  /// \brief Resolve the execution resource and subscribe to the source engine.
  void init(std::shared_ptr<execution::ContextInterface> context) override;

  /// \brief No-op: actions are driven by the source engine's callbacks.
  ///
  /// Reserved for future polling work (e.g. instantActions, asynchronous action
  /// completion).
  void step(std::shared_ptr<execution::ContextInterface> context) override;

  /// \brief Register the hook that performs actions (no-op if `executor` empty).
  void set_executor(ActionExecutor executor);

private:
  /// \brief Run a node's actions when it is traversed.
  void on_node_traversed(const NodeTraversedEvent& event);

  /// \brief Start an edge's actions when it is entered.
  void on_edge_entered(const EdgeEnteredEvent& event);

  /// \brief Stop an edge's still-running actions when it is left.
  void on_edge_left(const EdgeLeftEvent& event);

  /// \brief Run each action through the executor, claiming WAITING ones only.
  void run_actions(const std::vector<types::Action>& actions);

  /// \brief Claim, execute and record the outcome of a single action.
  void run_action(const types::Action& action);

  /// \brief Finish any of these actions that are still running (edge left).
  void stop_actions(const std::vector<types::Action>& actions);

  std::shared_ptr<execution::Engine> source_;
  std::shared_ptr<OrderExecutionResource> execution_;
  ActionExecutor executor_;
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__STRATEGIES__ORDER_ACTIONS_HPP_
