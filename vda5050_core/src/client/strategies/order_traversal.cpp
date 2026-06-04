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

#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/client/events/navigate_to_node.hpp"
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

// Runs the VDA5050 traversal cascade against the working state. Returns the
// next dispatch when traversal should continue, or nullopt when the AGV must
// stop (order complete, or the next node is part of the horizon).
std::optional<Dispatch> apply_node_reached(
  types::State& state, const NodeReachedUpdate& reached)
{
  auto node_it = std::find_if(  // Find the reached node
    state.node_states.begin(), state.node_states.end(),
    [&](const types::NodeState& n) {
      return n.sequence_id ==
               reached.sequence_id &&  // Match sequence_id and node_id
             n.node_id == reached.node_id;
    });
  if (
    node_it ==
    state.node_states.end())  // If the node is not found, return nullopt
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

  // Mark the node as the last reached node, then drop its state.
  state.last_node_id = reached.node_id;
  state.last_node_sequence_id = reached.sequence_id;
  state.node_states.erase(node_it);

  // Leaving the incoming edge (sequence_id == reached - 1): drop its state.
  // reached - 1 is because the edge is the one before the reached node.
  // e.g edge = 3, node = 4, so the incoming edge is 3.
  if (reached.sequence_id > 0)
  {
    const uint32_t incoming_edge_seq = reached.sequence_id - 1;
    state.edge_states.erase(  // Remove the incoming edge
      std::remove_if(
        state.edge_states.begin(), state.edge_states.end(),
        [&](const types::EdgeState& e) {
          return e.sequence_id == incoming_edge_seq;
        }),
      state.edge_states.end());
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
    return std::nullopt;  // nothing ahead -> order/base fully traversed
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
  if (!next_it->released) return std::nullopt;

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

  return Dispatch{*next_it, std::move(via_edge)};
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
  std::optional<Dispatch> dispatch = apply_node_reached(state, *reached);
  execution->set_state(std::move(state));

  if (dispatch)
  {
    engine()->emit<NavigateToNodeEvent>(
      execution::Priority::NORMAL, dispatch->target, dispatch->via_edge);
    // emit() only enqueues; step() delivers the event to registered handlers.
    engine()->step();
  }
}

}  // namespace client
}  // namespace vda5050_core
