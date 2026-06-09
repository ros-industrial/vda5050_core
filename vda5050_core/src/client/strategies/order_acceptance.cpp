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

#include "vda5050_core/client/strategies/order_acceptance.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/types/action_status.hpp"

namespace vda5050_core {

namespace client {

namespace {

types::NodeState to_node_state(const types::Node& node)
{
  types::NodeState state;
  state.node_id = node.node_id;
  state.sequence_id = node.sequence_id;
  state.node_description = node.node_description;
  state.node_position = node.node_position;
  state.released = node.released;
  return state;
}

types::EdgeState to_edge_state(const types::Edge& edge)
{
  types::EdgeState state;
  state.edge_id = edge.edge_id;
  state.sequence_id = edge.sequence_id;
  state.edge_description = edge.edge_description;
  state.released = edge.released;
  state.trajectory = edge.trajectory;
  return state;
}

types::ActionState to_action_state(const types::Action& action)
{
  types::ActionState state;
  state.action_id = action.action_id;
  state.action_type = action.action_type;
  state.action_description = action.action_description;
  state.action_status = types::ActionStatus::WAITING;
  return state;
}

void append_node(types::State& state, const types::Node& node)
{
  state.node_states.push_back(to_node_state(node));
  for (const auto& action : node.actions)
  {
    state.action_states.push_back(to_action_state(action));
  }
}

void append_edge(types::State& state, const types::Edge& edge)
{
  state.edge_states.push_back(to_edge_state(edge));
  for (const auto& action : edge.actions)
  {
    state.action_states.push_back(to_action_state(action));
  }
}

// New order: reset the execution snapshot and populate it from the order.
void apply_new_order(types::State& state, const types::Order& order)
{
  state.node_states.clear();
  state.edge_states.clear();
  state.action_states.clear();
  state.order_id = order.order_id;
  state.order_update_id = order.order_update_id;

  for (const auto& node : order.nodes) append_node(state, node);
  for (const auto& edge : order.edges) append_edge(state, edge);
}

// Order update: keep the running base, drop the old horizon, append new states.
void apply_order_update(types::State& state, const types::Order& order)
{
  state.order_update_id = order.order_update_id;

  // Drop the previous horizon (unreleased) node/edge states; the update
  // re-supplies them. The first node of the update is the re-sent stitch node
  // (already present in the base), so it is skipped when appending.
  state.node_states.erase(
    std::remove_if(
      state.node_states.begin(), state.node_states.end(),
      [](const types::NodeState& n) { return !n.released; }),
    state.node_states.end());
  state.edge_states.erase(
    std::remove_if(
      state.edge_states.begin(), state.edge_states.end(),
      [](const types::EdgeState& e) { return !e.released; }),
    state.edge_states.end());

  for (size_t i = 1; i < order.nodes.size(); ++i)
  {
    append_node(state, order.nodes[i]);
  }
  for (const auto& edge : order.edges) append_edge(state, edge);
}

}  // namespace

OrderAcceptance::OrderAcceptance() = default;

void OrderAcceptance::init(
  std::shared_ptr<execution::ContextInterface> /*context*/)
{
  // Nothing to initialise; the validator carries its own (permissive) hooks.
}

void OrderAcceptance::step(std::shared_ptr<execution::ContextInterface> context)
{
  auto order_context = std::dynamic_pointer_cast<AGVContext>(context);
  if (!order_context)
  {
    VDA5050_WARN_STREAM("OrderAcceptance: context is not an AGVContext");
    return;
  }

  // The inbound order is whatever the protocol adapter (or a test) last pushed.
  auto update = order_context->get_update<OrderUpdate>();
  if (!update) return;  // nothing to process yet

  const types::Order& order = update->order;
  const AcceptanceResult result =
    validator_.validate_order(order, order_context);

  auto execution = order_context->get_resource<OrderExecutionResource>();
  if (!execution) return;

  // State is mutated via a read-modify-write on the resource's snapshot
  // (get_state -> mutate -> set_state). OrderAcceptance is the sole writer of
  // order/node state, so the read-write window carries no lost-update risk.
  switch (result.outcome)
  {
    case AcceptanceOutcome::Accepted: {
      types::State state = execution->get_state();
      const bool is_update = (order.order_id == state.order_id);
      if (is_update)
      {
        apply_order_update(state, order);
      }
      else
      {
        apply_new_order(state, order);
      }
      execution->set_state(std::move(state));
      execution->set_executing_order(true);
      break;
    }
    case AcceptanceOutcome::Rejected: {
      // Errors are kept in state until a new order is accepted.
      types::State state = execution->get_state();
      for (const auto& error : result.errors)
      {
        state.errors.push_back(error);
      }
      execution->set_state(std::move(state));
      break;
    }
    case AcceptanceOutcome::Ignored:
      break;  // duplicate resend; leave the execution state untouched
  }
}

}  // namespace client
}  // namespace vda5050_core
