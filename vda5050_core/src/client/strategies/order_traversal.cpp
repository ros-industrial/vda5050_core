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

#include "vda5050_core/client/strategies/order_traversal.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/client/events/edge_entered.hpp"
#include "vda5050_core/client/events/edge_left.hpp"
#include "vda5050_core/client/events/navigate_to_node.hpp"
#include "vda5050_core/client/events/node_reached.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/updates/node_reached.hpp"
#include "vda5050_core/execution/event_queue.hpp"
#include "vda5050_core/logger/logger.hpp"

namespace vda5050_core {

namespace client {

namespace {

// Raise new_base_request once this few released nodes remain in the base.
constexpr std::size_t kLowBaseThreshold = 1;

// The next node to drive to, plus the edge to traverse to reach it.
struct Dispatch
{
  types::NodeState target;
  std::optional<types::EdgeState> via_edge;
};

// Identifies an edge for the edge-transition events.
struct EdgeRef
{
  std::string edge_id;
  uint32_t sequence_id = 0;
};

// What the cascade advanced: the traversed node (always present), the edge that
// was left (if any), and the next dispatch (absent when stopped at the decision
// point or the order is complete). The entered edge is the dispatch's via_edge.
struct TraversalOutcome
{
  std::string node_id;
  uint32_t sequence_id = 0;
  std::optional<EdgeRef> left_edge;
  std::optional<Dispatch> dispatch;
};

// Runs the VDA5050 traversal cascade against the working state. Returns the
// outcome when a node was actually traversed, or nullopt when the signal is
// ignored (unknown/stale node, or reported out of order).
std::optional<TraversalOutcome> apply_node_reached(
  types::State& state, const NodeReachedUpdate& reached)
{
  // Find the reached node (matched on sequence_id and node_id) in the base.
  auto node_it = std::find_if(
    state.node_states.begin(), state.node_states.end(),
    [&](const types::NodeState& n) {
      return n.sequence_id == reached.sequence_id &&
             n.node_id == reached.node_id;
    });
  if (node_it == state.node_states.end())
  {
    // Unknown or stale node; nothing to advance.
    return std::nullopt;
  }

  // Traversal is sequential: only advance when the reported node is the next
  // expected one (no lower sequence_id still pending). Reaching anything else
  // would orphan the nodes in between, so it is ignored.
  const bool is_next_expected = std::none_of(
    state.node_states.begin(), state.node_states.end(),
    [&](const types::NodeState& n) {
      return n.sequence_id < reached.sequence_id;
    });
  if (!is_next_expected) return std::nullopt;

  TraversalOutcome outcome;
  outcome.node_id = reached.node_id;
  outcome.sequence_id = reached.sequence_id;

  // Mark the node as the last reached node, then drop its state.
  state.last_node_id = reached.node_id;
  state.last_node_sequence_id = reached.sequence_id;
  state.node_states.erase(node_it);

  // Leaving the incoming edge (sequence_id == reached - 1): record then drop it.
  // reached - 1 is because the edge is the one before the reached node.
  // e.g edge = 3, node = 4, so the incoming edge is 3.
  if (reached.sequence_id > 0)
  {
    const uint32_t incoming_edge_seq = reached.sequence_id - 1;
    auto incoming_it = std::find_if(
      state.edge_states.begin(), state.edge_states.end(),
      [&](const types::EdgeState& e) {
        return e.sequence_id == incoming_edge_seq;
      });
    if (incoming_it != state.edge_states.end())
    {
      outcome.left_edge =
        EdgeRef{incoming_it->edge_id, incoming_it->sequence_id};
      state.edge_states.erase(incoming_it);
    }
  }

  // The next node is the lowest sequence_id strictly ahead of the reached one.
  auto next_it = state.node_states.end();
  for (auto it = state.node_states.begin(); it != state.node_states.end(); ++it)
  {
    if (it->sequence_id <= reached.sequence_id) continue;
    if (
      next_it == state.node_states.end() ||
      it->sequence_id < next_it->sequence_id)
    {
      next_it = it;
    }
  }
  if (next_it == state.node_states.end())
  {
    state.new_base_request = std::nullopt;
    return outcome;  // node traversed; nothing ahead -> order/base complete
  }

  // Count the number of released nodes ahead of the reached node.
  // Raise new_base_request when the released base ahead is running low.
  const std::size_t released_ahead = static_cast<std::size_t>(std::count_if(
    state.node_states.begin(), state.node_states.end(),
    [&](const types::NodeState& n) {
      return n.released && n.sequence_id > reached.sequence_id;
    }));
  if (released_ahead <= kLowBaseThreshold)
  {
    state.new_base_request = true;
  }
  else
  {
    // Base is sufficiently long again (e.g. after an order update extended it).
    state.new_base_request = false;
  }

  // Stop at the decision point: never drive into the horizon.
  if (!next_it->released) return outcome;

  // Find the edge leading into the next node (sequence_id == next - 1).
  std::optional<types::EdgeState> via_edge;
  if (next_it->sequence_id > 0)
  {
    const uint32_t via_seq = next_it->sequence_id - 1;
    auto edge_it = std::find_if(
      state.edge_states.begin(), state.edge_states.end(),
      [&](const types::EdgeState& e) { return e.sequence_id == via_seq; });
    if (edge_it != state.edge_states.end()) via_edge = *edge_it;
  }

  outcome.dispatch = Dispatch{*next_it, std::move(via_edge)};
  return outcome;
}

}  // namespace

OrderTraversal::OrderTraversal() = default;

void OrderTraversal::init(
  std::shared_ptr<execution::ContextInterface> /*context*/)
{
  // Nothing to initialise; node-reached signals are read from the context.
}

void OrderTraversal::step(std::shared_ptr<execution::ContextInterface> context)
{
  auto order_context = std::dynamic_pointer_cast<AGVContext>(context);
  if (!order_context)
  {
    VDA5050_WARN_STREAM("OrderTraversal: context is not an AGVContext");
    return;
  }

  auto reached = order_context->get_update<NodeReachedUpdate>();
  if (!reached) return;  // no node-reached signal yet

  auto execution = order_context->get_resource<OrderExecutionResource>();
  if (!execution) return;

  // NOTE: get_state()/set_state() is a non-atomic read-modify-write.
  // This is safe while the handler runs state-writing strategies sequentially.
  types::State state = execution->get_state();
  std::optional<TraversalOutcome> outcome = apply_node_reached(state, *reached);
  execution->set_state(std::move(state));

  if (!outcome) return;  // signal ignored; nothing traversed

  // Announce the traversal so the action strategy can trigger node/edge actions.
  // Order follows the VDA5050 cascade: node traversed, incoming edge left, then
  // the next edge entered. emit() only enqueues, so step() is called after each
  // to deliver that event before the next is emitted (one event per step()).
  engine()->emit<NodeReachedEvent>(
    execution::Priority::NORMAL, outcome->node_id, outcome->sequence_id);
  engine()->step();
  if (outcome->left_edge)
  {
    engine()->emit<EdgeLeftEvent>(
      execution::Priority::NORMAL, outcome->left_edge->edge_id,
      outcome->left_edge->sequence_id);
    engine()->step();
  }
  if (outcome->dispatch)
  {
    engine()->emit<NavigateToNodeEvent>(
      execution::Priority::NORMAL, outcome->dispatch->target,
      outcome->dispatch->via_edge);
    engine()->step();
    if (outcome->dispatch->via_edge)
    {
      engine()->emit<EdgeEnteredEvent>(
        execution::Priority::NORMAL, outcome->dispatch->via_edge->edge_id,
        outcome->dispatch->via_edge->sequence_id);
      engine()->step();
    }
  }
}

}  // namespace client
}  // namespace vda5050_core
