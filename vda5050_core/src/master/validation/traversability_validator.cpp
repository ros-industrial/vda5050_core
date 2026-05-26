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

#include "vda5050_core/master/validation/traversability_validator.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include "vda5050_core/errors/error_codes.hpp"
#include "vda5050_core/errors/error_factory.hpp"

namespace vda5050_core::master {

namespace {

using ::vda5050_core::errors::create_error;
using ::vda5050_core::errors::TraversabilityValidationError;
using ::vda5050_core::order_utils::ValidationResult;
using ::vda5050_core::types::AGVAction;
using ::vda5050_core::types::ErrorReference;

using AddErrorFn =
  std::function<void(const std::string&, std::vector<ErrorReference>)>;

const AGVAction* find_agv_action(
  const vda5050_core::types::Factsheet& fs, const std::string& action_type)
{
  const auto& actions = fs.protocol_features.agv_actions;
  auto it = std::find_if(
    actions.begin(), actions.end(),
    [&](const AGVAction& a) { return a.action_type == action_type; });
  return (it == actions.end()) ? nullptr : &(*it);
}

void validate_action_against_factsheet(
  const vda5050_core::types::Action& action,
  vda5050_core::types::ActionScope expected_scope,
  const vda5050_core::types::Factsheet& factsheet, const AddErrorFn& add_error)
{
  const AGVAction* agv_action = find_agv_action(factsheet, action.action_type);
  if (agv_action == nullptr)
  {
    add_error(
      "Action.action_type '" + action.action_type +
        "' is not supported by AGV (not present in factsheet.agv_actions).",
      {});
    return;
  }

  const auto& scopes = agv_action->action_scopes;
  if (std::find(scopes.begin(), scopes.end(), expected_scope) == scopes.end())
  {
    add_error(
      "Action.action_type '" + action.action_type +
        "' does not declare the required scope for its placement.",
      {});
  }

  if (
    action.action_parameters.has_value() &&
    agv_action->action_parameters.has_value())
  {
    const auto& declared = agv_action->action_parameters.value();
    for (const auto& p : action.action_parameters.value())
    {
      auto it = std::find_if(
        declared.begin(), declared.end(),
        [&](const vda5050_core::types::ActionParameterFactsheet& d) {
          return d.key == p.key;
        });
      if (it == declared.end())
      {
        add_error(
          "Action parameter key '" + p.key +
            "' not declared by AGV for action_type '" + action.action_type +
            "'.",
          {});
      }
    }
  }
}

// Reachability (state-driven; runs without a factsheet).
void validate_reachability(
  const PreSendContext& ctx, const vda5050_core::types::Order& order,
  const AddErrorFn& add_error)
{
  if (order.nodes.empty())
  {
    return;  // the graph validator already rejects this
  }

  const auto& first = order.nodes.front();

  if (!ctx.last_state.has_value())
  {
    add_error(
      "Cannot determine reachability: AGV has not yet reported any State.", {});
    return;
  }
  const auto& state = ctx.last_state.value();

  if (state.last_node_id == first.node_id) return;

  if (!first.node_position.has_value() || !state.agv_position.has_value())
  {
    add_error(
      "Cannot determine reachability: AGV is not on first node and "
      "either node_position or agv_position is missing.",
      {{::vda5050_core::errors::RefNodeId, first.node_id}});
    return;
  }

  const auto& np = first.node_position.value();
  const auto& ap = state.agv_position.value();

  // No declared deviation => must be exactly on the node.
  const double allowed = np.allowed_deviation_x_y.value_or(0.0);

  const double dx = ap.x - np.x;
  const double dy = ap.y - np.y;
  const double distance = std::hypot(dx, dy);

  if (distance > allowed)
  {
    add_error(
      "AGV is not within the first node's allowed_deviation_x_y "
      "(distance=" +
        std::to_string(distance) + " m, allowed=" + std::to_string(allowed) +
        " m).",
      {{::vda5050_core::errors::RefNodeId, first.node_id}});
  }
}

void check_limit(
  const std::optional<uint32_t>& limit, std::size_t actual,
  const std::string& field_name, const AddErrorFn& add_error)
{
  if (limit.has_value() && actual > limit.value())
  {
    add_error(
      "Order exceeds factsheet limit for " + field_name +
        " (size=" + std::to_string(actual) +
        ", max=" + std::to_string(limit.value()) + ").",
      {});
  }
}

// Fallback node-position tolerance when a Map node declares none.
constexpr double kDefaultMapPositionDeviation = 0.5;

// Pre: ctx.loaded_map non-null (guaranteed by pre_send's no-map gate).
void validate_map_integrity_locked(
  const PreSendContext& ctx, const vda5050_core::types::Order& order,
  const AddErrorFn& add_error)
{
  const auto& m = *ctx.loaded_map;

  for (const auto& node : order.nodes)
  {
    const auto* mn = m.find_node(node.node_id);
    if (mn == nullptr)
    {
      add_error(
        "Order node_id '" + node.node_id +
          "' is not present in the master's loaded map.",
        {{::vda5050_core::errors::RefNodeId, node.node_id}});
      continue;
    }
    if (!node.node_position.has_value()) continue;
    const auto& np = node.node_position.value();
    const double allowed =
      mn->allowed_deviation_xy.value_or(kDefaultMapPositionDeviation);
    const double dx = np.x - mn->x;
    const double dy = np.y - mn->y;
    const double distance = std::hypot(dx, dy);
    if (distance > allowed)
    {
      add_error(
        "Order node '" + node.node_id +
          "' nodePosition is outside the map's allowed_deviation_xy "
          "(distance=" +
          std::to_string(distance) + " m, allowed=" + std::to_string(allowed) +
          " m).",
        {{::vda5050_core::errors::RefNodeId, node.node_id}});
    }
  }

  for (const auto& edge : order.edges)
  {
    const auto* me = m.find_edge(edge.edge_id);
    if (me == nullptr)
    {
      add_error(
        "Order edge_id '" + edge.edge_id +
          "' is not present in the master's loaded map.",
        {{::vda5050_core::errors::RefEdgeId, edge.edge_id}});
      continue;
    }
    const bool forward =
      (me->start_node_id == edge.start_node_id &&
       me->end_node_id == edge.end_node_id);
    const bool reverse =
      me->bidirectional && (me->start_node_id == edge.end_node_id &&
                            me->end_node_id == edge.start_node_id);
    if (!forward && !reverse)
    {
      add_error(
        "Order edge '" + edge.edge_id + "' endpoints (" + edge.start_node_id +
          "->" + edge.end_node_id + ") do not match the map's edge (" +
          me->start_node_id + "->" + me->end_node_id +
          (me->bidirectional ? ", bidirectional" : ", one-way") + ").",
        {{::vda5050_core::errors::RefEdgeId, edge.edge_id}});
    }
  }
}

}  // namespace

ValidationResult validate_traversability(
  const PreSendContext& ctx, const vda5050_core::types::Order& order)
{
  ValidationResult res;

  auto add_error =
    [&](const std::string& description, std::vector<ErrorReference> refs) {
      refs.push_back({::vda5050_core::errors::RefOrderId, order.order_id});
      res.errors.push_back(
        create_error(TraversabilityValidationError, description, refs));
    };

  validate_reachability(ctx, order, add_error);

  if (ctx.loaded_map != nullptr)
  {
    validate_map_integrity_locked(ctx, order, add_error);
  }

  // Capability + limit checks need a factsheet.
  if (!ctx.last_factsheet.has_value()) return res;

  const auto& fs = ctx.last_factsheet.value();
  const auto& limits = fs.protocol_limits.max_array_lens;

  check_limit(limits.order_nodes, order.nodes.size(), "order_nodes", add_error);
  check_limit(limits.order_edges, order.edges.size(), "order_edges", add_error);

  for (const auto& node : order.nodes)
  {
    check_limit(
      limits.node_actions, node.actions.size(), "node_actions", add_error);
    for (const auto& action : node.actions)
    {
      check_limit(
        limits.actions_actions_parameters,
        action.action_parameters.has_value() ? action.action_parameters->size()
                                             : 0,
        "actions_actions_parameters", add_error);
      validate_action_against_factsheet(
        action, vda5050_core::types::ActionScope::NODE, fs, add_error);
    }
  }

  for (const auto& edge : order.edges)
  {
    check_limit(
      limits.edge_actions, edge.actions.size(), "edge_actions", add_error);
    for (const auto& action : edge.actions)
    {
      check_limit(
        limits.actions_actions_parameters,
        action.action_parameters.has_value() ? action.action_parameters->size()
                                             : 0,
        "actions_actions_parameters", add_error);
      validate_action_against_factsheet(
        action, vda5050_core::types::ActionScope::EDGE, fs, add_error);
    }
  }

  return res;
}

ValidationResult validate_traversability(
  const PreSendContext& ctx, const vda5050_core::types::InstantActions& actions)
{
  ValidationResult res;

  auto add_error =
    [&](const std::string& description, std::vector<ErrorReference> refs) {
      res.errors.push_back(
        create_error(TraversabilityValidationError, description, refs));
    };

  // No graph => no reachability; capability/limits need a factsheet.
  if (!ctx.last_factsheet.has_value()) return res;

  const auto& fs = ctx.last_factsheet.value();
  const auto& limits = fs.protocol_limits.max_array_lens;

  check_limit(
    limits.instant_actions, actions.actions.size(), "instant_actions",
    add_error);

  for (const auto& action : actions.actions)
  {
    check_limit(
      limits.actions_actions_parameters,
      action.action_parameters.has_value() ? action.action_parameters->size()
                                           : 0,
      "actions_actions_parameters", add_error);
    validate_action_against_factsheet(
      action, vda5050_core::types::ActionScope::INSTANT, fs, add_error);
  }

  return res;
}

}  // namespace vda5050_core::master
