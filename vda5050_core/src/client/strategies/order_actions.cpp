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

#include "vda5050_core/client/strategies/order_actions.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"

namespace vda5050_core {

namespace client {

namespace {

// Locate the action state for an action_id within the working state.
types::ActionState* find_action_state(
  types::State& state, const std::string& action_id)
{
  for (auto& action_state : state.action_states)
  {
    if (action_state.action_id == action_id) return &action_state;
  }
  return nullptr;
}

// True while an action is mid-flight and can still be stopped by leaving an edge.
bool is_active(types::ActionStatus status)
{
  return status == types::ActionStatus::INITIALIZING ||
         status == types::ActionStatus::RUNNING ||
         status == types::ActionStatus::PAUSED;
}

}  // namespace

OrderActions::OrderActions(std::shared_ptr<execution::Engine> source)
: source_(std::move(source))
{
}

void OrderActions::init(std::shared_ptr<execution::ContextInterface> context)
{
  if (!context)
  {
    VDA5050_WARN_STREAM("OrderActions: context is null");
    return;
  }

  // The execution resource carries the action states this strategy advances.
  execution_ = context->get_resource<OrderExecutionResource>();
  if (!execution_)
  {
    VDA5050_WARN_STREAM("OrderActions: OrderExecutionResource not available");
  }

  if (!source_)
  {
    VDA5050_WARN_STREAM(
      "OrderActions: source engine is null; no actions fired");
    return;
  }

  // Hook the traversal cascade. Callbacks fire synchronously while the traversal
  // strategy steps, so each event is handled exactly once and in order.
  source_->on<NodeTraversedEvent>(
    [this](std::shared_ptr<NodeTraversedEvent> event) {
      this->on_node_traversed(*event);
    });
  source_->on<EdgeEnteredEvent>(
    [this](std::shared_ptr<EdgeEnteredEvent> event) {
      this->on_edge_entered(*event);
    });
  source_->on<EdgeLeftEvent>([this](std::shared_ptr<EdgeLeftEvent> event) {
    this->on_edge_left(*event);
  });
}

void OrderActions::step(std::shared_ptr<execution::ContextInterface> /*ctx*/)
{
  // Node and edge actions are driven by the source engine's callbacks. Nothing
  // to poll here yet; reserved for instantActions and async action completion.
}

void OrderActions::set_executor(ActionExecutor executor)
{
  if (!executor) return;
  executor_ = std::move(executor);
}

void OrderActions::on_node_traversed(const NodeTraversedEvent& event)
{
  if (!execution_) return;

  const auto order = execution_->get_active_order();
  for (const auto& node : order.nodes)
  {
    if (node.node_id == event.node_id && node.sequence_id == event.sequence_id)
    {
      run_actions(node.actions);
      return;
    }
  }
}

void OrderActions::on_edge_entered(const EdgeEnteredEvent& event)
{
  if (!execution_) return;

  const auto order = execution_->get_active_order();
  for (const auto& edge : order.edges)
  {
    if (edge.edge_id == event.edge_id && edge.sequence_id == event.sequence_id)
    {
      run_actions(edge.actions);
      return;
    }
  }
}

void OrderActions::on_edge_left(const EdgeLeftEvent& event)
{
  if (!execution_) return;

  const auto order = execution_->get_active_order();
  for (const auto& edge : order.edges)
  {
    if (edge.edge_id == event.edge_id && edge.sequence_id == event.sequence_id)
    {
      stop_actions(edge.actions);
      return;
    }
  }
}

void OrderActions::run_actions(const std::vector<types::Action>& actions)
{
  for (const auto& action : actions) run_action(action);
}

void OrderActions::run_action(const types::Action& action)
{
  if (!executor_)
  {
    VDA5050_WARN_STREAM(
      "OrderActions: no action executor registered; action '"
      << action.action_id << "' (" << action.action_type << ") left WAITING");
    return;
  }

  // TODO(actions): enforce blockingType (NONE/SOFT/HARD) before claiming, so a
  // HARD action blocks all others and a SOFT action blocks movement.

  // Claim the action under the lock: only a WAITING action transitions to
  // RUNNING, so re-delivery of the same signal is idempotent and a single
  // action is never executed twice.
  bool claimed = false;
  execution_->update_state([&](types::State& state) {
    auto* action_state = find_action_state(state, action.action_id);
    if (
      action_state &&
      action_state->action_status == types::ActionStatus::WAITING)
    {
      action_state->action_status = types::ActionStatus::RUNNING;
      claimed = true;
    }
  });
  if (!claimed) return;

  // Perform the action outside the lock; the executor is integrator code and
  // may itself read the execution resource.
  const ActionExecution result = executor_(action);

  execution_->update_state([&](types::State& state) {
    auto* action_state = find_action_state(state, action.action_id);
    if (action_state)
    {
      action_state->action_status = result.status;
      action_state->result_description = result.result_description;
    }
  });
}

void OrderActions::stop_actions(const std::vector<types::Action>& actions)
{
  if (actions.empty()) return;

  // Edge actions are time-bound: leaving the edge stops any that are still
  // active. Finished/failed/waiting actions are left untouched.
  execution_->update_state([&](types::State& state) {
    for (const auto& action : actions)
    {
      auto* action_state = find_action_state(state, action.action_id);
      if (action_state && is_active(action_state->action_status))
      {
        action_state->action_status = types::ActionStatus::FINISHED;
        action_state->result_description = "stopped: edge left";
      }
    }
  });
}

}  // namespace client
}  // namespace vda5050_core
