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

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/blocking_type.hpp"
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

// blockingType of an action looked up by id in the accepted order. Defaults to
// NONE when the id is unknown (the least restrictive, fail-open choice).
types::BlockingType blocking_type_of(
  const types::Order& order, const std::string& action_id)
{
  for (const auto& node : order.nodes)
  {
    for (const auto& action : node.actions)
    {
      if (action.action_id == action_id) return action.blocking_type;
    }
  }
  for (const auto& edge : order.edges)
  {
    for (const auto& action : edge.actions)
    {
      if (action.action_id == action_id) return action.blocking_type;
    }
  }
  return types::BlockingType::NONE;
}

// Decide whether `action` may start right now given the actions already active
// and whether the AGV is driving.
//   - a HARD action active blocks everything;
//   - a HARD action needs exclusivity (no other action active);
//   - a SOFT/HARD action must not start while driving.
bool can_start(
  const types::Action& action, const types::State& state,
  const types::Order& order)
{
  bool any_active = false;
  bool hard_active = false;
  for (const auto& action_state : state.action_states)
  {
    if (!is_active(action_state.action_status)) continue;
    any_active = true;
    if (
      blocking_type_of(order, action_state.action_id) ==
      types::BlockingType::HARD)
    {
      hard_active = true;
    }
  }

  if (hard_active) return false;
  if (action.blocking_type == types::BlockingType::HARD && any_active)
  {
    return false;
  }
  if (action.blocking_type != types::BlockingType::NONE && state.driving)
  {
    return false;
  }
  return true;
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
  // Actions are triggered by the source engine's callbacks, but an action
  // deferred by blockingType (e.g. SOFT while driving, or anything behind a HARD
  // action) needs retrying when that condition clears without a fresh traversal
  // event. The Handler calls step() every spin, so retry the pending queue here.
  if (execution_ && !pending_.empty()) pump();
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
      enqueue(node.actions);
      pump();
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
      enqueue(edge.actions);
      pump();
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
      // Stopping an edge action may free a blocking action that was deferred.
      pump();
      return;
    }
  }
}

void OrderActions::enqueue(const std::vector<types::Action>& actions)
{
  for (const auto& action : actions)
  {
    const bool already_queued = std::any_of(
      pending_.begin(), pending_.end(), [&](const types::Action& queued) {
        return queued.action_id == action.action_id;
      });
    if (!already_queued) pending_.push_back(action);
  }
}

void OrderActions::pump()
{
  // Each iteration either drops a stale entry or starts one action, so pending_
  // strictly shrinks and the loop terminates. Starting an action that finishes
  // can unblock a HARD action, so we re-evaluate against fresh state each pass.
  while (true)
  {
    auto state = execution_->get_state();
    const auto order = execution_->get_active_order();

    // Drop entries that are no longer WAITING (already started elsewhere, or
    // never seeded), keeping re-delivery of the same signal idempotent.
    pending_.erase(
      std::remove_if(
        pending_.begin(), pending_.end(),
        [&](const types::Action& action) {
          const auto* action_state = find_action_state(state, action.action_id);
          return !action_state ||
                 action_state->action_status != types::ActionStatus::WAITING;
        }),
      pending_.end());

    const auto next = std::find_if(
      pending_.begin(), pending_.end(), [&](const types::Action& action) {
        return can_start(action, state, order);
      });
    if (next == pending_.end()) break;  // nothing eligible right now

    const types::Action action = *next;
    pending_.erase(next);
    start_action(action);
  }
}

void OrderActions::start_action(const types::Action& action)
{
  if (!executor_)
  {
    VDA5050_WARN_STREAM(
      "OrderActions: no action executor registered; action '"
      << action.action_id << "' (" << action.action_type << ") left WAITING");
    return;
  }

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
