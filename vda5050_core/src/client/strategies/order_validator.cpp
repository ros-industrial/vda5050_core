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

#include "vda5050_core/client/strategies/order_validator.hpp"

#include <string>
#include <utility>
#include <vector>

#include "vda5050_core/errors/error_codes.hpp"
#include "vda5050_core/errors/error_factory.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/order_utils/order_graph_validator.hpp"
#include "vda5050_core/types/action_status.hpp"

namespace vda5050_core {

namespace client {

namespace {

// Builds a VDA5050 Error, always appending the orderId / orderUpdateId refs.
types::Error make_error(
  const std::string& type, const std::string& description,
  const types::Order& order, std::vector<types::ErrorReference> refs = {})
{
  refs.push_back({errors::RefOrderId, order.order_id});
  refs.push_back(
    {errors::RefOrderUpdateId, std::to_string(order.order_update_id)});
  return errors::create_error(type, description, refs);
}

AcceptanceResult rejected(types::Error error)
{
  VDA5050_WARN_STREAM(
    "OrderValidator rejected order: " << error.error_type << " - "
                                      << error.error_description.value_or(""));
  AcceptanceResult res;
  res.outcome = AcceptanceOutcome::Rejected;
  res.errors.push_back(std::move(error));
  return res;
}

AcceptanceResult rejected(std::vector<types::Error> errors)
{
  AcceptanceResult res;
  res.outcome = AcceptanceOutcome::Rejected;
  res.errors = std::move(errors);
  return res;
}

AcceptanceResult ignored()
{
  VDA5050_INFO_STREAM("OrderValidator ignored duplicate order update");
  AcceptanceResult res;
  res.outcome = AcceptanceOutcome::Ignored;
  return res;
}

AcceptanceResult accepted()
{
  AcceptanceResult res;
  res.outcome = AcceptanceOutcome::Accepted;
  return res;
}

// The vehicle is "busy" if it still has nodes left
// to traverse (base or horizon) or any action that is not yet FINISHED/FAILED.
bool is_busy(const std::shared_ptr<OrderContext>& context)
{
  if (!context->get_node_states().empty()) return true;

  for (const auto& action : context->get_action_states())
  {
    if (
      action.action_status != types::ActionStatus::FINISHED &&
      action.action_status != types::ActionStatus::FAILED)
    {
      return true;
    }
  }
  return false;
}

}  // namespace

OrderValidator::OrderValidator()
: reachable_([](const types::Node&) { return true; }),
  capable_(
    [](const types::Order&) { return std::vector<types::ErrorReference>{}; })
{
}

void OrderValidator::set_reachability_check(ReachabilityCheck check)
{
  if (check) reachable_ = std::move(check);
}

void OrderValidator::set_capability_check(CapabilityCheck check)
{
  if (check) capable_ = std::move(check);
}

AcceptanceResult OrderValidator::validate_order(
  const types::Order& incoming_order,
  const std::shared_ptr<OrderContext>& context) const
{
  // If context is null, return a validation error.
  if (!context)
  {
    return rejected(errors::create_error(
      errors::ValidationError, "OrderContext is null", {}));
  }

  // Step 1: structural / format validity of the order graph.
  if (auto graph = order_utils::is_valid_graph(incoming_order); !graph)
  {
    return rejected(std::move(graph.errors));
  }

  // Capability / map / optional-field support. The AGV must
  // reject (not silently ignore) fields or actions it cannot process.
  if (auto unsupported = capable_(incoming_order); !unsupported.empty())
  {
    return rejected(make_error(
      errors::OrderError,
      "Order references fields or actions the AGV cannot process",
      incoming_order, std::move(unsupported)));
  }

  const std::string current_id = context->get_active_order_id();
  const uint32_t current_update_id = context->get_active_order_update_id();

  // Step 2: brand new order, or an update of the active one?
  if (incoming_order.order_id != current_id)
  {
    // New order
    // Step 3: reject if the vehicle is still executing or holding a horizon.
    if (is_busy(context))
    {
      return rejected(make_error(
        errors::ValidationError,
        "Cannot accept a new order while the vehicle is still executing or "
        "holding a horizon",
        incoming_order));
    }

    // Step 4: the start node must be reachable from the current position.
    if (
      !incoming_order.nodes.empty() &&
      !reachable_(incoming_order.nodes.front()))
    {
      return rejected(make_error(
        errors::ValidationError,
        "Start node of the new order is not reachable from current position",
        incoming_order,
        {{errors::RefNodeId, incoming_order.nodes.front().node_id}}));
    }

    return accepted();
  }

  // Update of the active order (same orderId)
  // Step 6: identical orderUpdateId => duplicate resend; discard silently.
  if (incoming_order.order_update_id == current_update_id)
  {
    return ignored();
  }

  // Step 5: lower orderUpdateId => deprecated update. Report it
  // and keep executing the previous order. Any strictly greater id is a valid
  // newer update -- there is no requirement that it increment by exactly one.
  if (incoming_order.order_update_id < current_update_id)
  {
    return rejected(make_error(
      errors::OrderUpdateError,
      "Order update is deprecated (older than the active orderUpdateId)",
      incoming_order));
  }

  // Steps 7/8: the update must continue from the decision point. We anchor on
  // the last reached node tracked in OrderContext (nodeId + sequenceId).
  if (incoming_order.nodes.empty())
  {
    return rejected(make_error(
      errors::ValidationError,
      "Order update must contain at least one node for stitching",
      incoming_order));
  }

  const auto& stitch_node = incoming_order.nodes.front();
  if (
    stitch_node.node_id != context->get_last_node_id() ||
    stitch_node.sequence_id != context->get_last_node_sequence_id())
  {
    return rejected(make_error(
      errors::OrderUpdateError,
      "Order update stitch node does not match the last known base node",
      incoming_order,
      {{errors::RefNodeId, stitch_node.node_id},
       {errors::RefSequenceId, std::to_string(stitch_node.sequence_id)}}));
  }

  return accepted();
}

}  // namespace client
}  // namespace vda5050_core
