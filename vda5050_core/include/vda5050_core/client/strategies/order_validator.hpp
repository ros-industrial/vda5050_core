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

#ifndef VDA5050_CORE__CLIENT__STRATEGIES__ORDER_VALIDATOR_HPP_
#define VDA5050_CORE__CLIENT__STRATEGIES__ORDER_VALIDATOR_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "vda5050_core/client/contexts/agv_context.hpp"
#include "vda5050_core/types/error.hpp"
#include "vda5050_core/types/order.hpp"

namespace vda5050_core {

namespace client {

/// \brief Outcome of the VDA5050 order-acceptance decision.
enum class AcceptanceOutcome
{
  /// \brief Order is valid and should be applied to the execution state.
  Accepted,
  /// \brief Order is invalid; report `errors` and keep the previous order.
  Rejected,
  /// \brief Duplicate resend (same orderId + orderUpdateId); discard silently.
  Ignored
};

/// \brief Result of order-acceptance validation.
struct AcceptanceResult
{
  AcceptanceOutcome outcome = AcceptanceOutcome::Rejected;

  /// \brief VDA5050 errors to report when `outcome == Rejected`. Each carries
  /// the error_type, errorReferences and warning level required by the spec.
  std::vector<types::Error> errors;

  bool accepted() const
  {
    return outcome == AcceptanceOutcome::Accepted;
  }
  bool rejected() const
  {
    return outcome == AcceptanceOutcome::Rejected;
  }
  bool ignored() const
  {
    return outcome == AcceptanceOutcome::Ignored;
  }
};

/// \brief Gatekeeper that runs the VDA5050 order-acceptance decision flow
/// before an order enters the execution plane.
///
/// Structural validity reuses `order_utils::is_valid_graph`. Position- and
/// capability-dependent checks (reachable start node, supported fields/actions)
/// are injectable hooks that default to permissive, so the robot configuration
/// layer can supply concrete checks without changing the decision flow.
class OrderValidator
{
public:
  /// \brief Hook: can the AGV begin the order at this start node (standing on
  /// it or within its deviation range)? Defaults to always-accept; inject a
  /// position-aware check via set_reachability_check().
  using ReachabilityCheck = std::function<bool(const types::Node&)>;

  /// \brief Hook: returns references to any order fields/actions/maps the AGV
  /// cannot process; an empty result means fully supported. Defaults to
  /// accept-all; inject a factsheet-aware check via set_capability_check().
  using CapabilityCheck =
    std::function<std::vector<types::ErrorReference>(const types::Order&)>;

  OrderValidator();

  /// \brief Override the start-node reachability hook (no-op if `check` empty).
  void set_reachability_check(ReachabilityCheck check);

  /// \brief Override the capability/field-support hook (no-op if `check` empty).
  void set_capability_check(CapabilityCheck check);

  /// \brief Evaluate an incoming order against context state and protocol limits.
  ///
  /// \param incoming_order Order payload received from master control.
  /// \param context Thread-safe order context with execution tracking state.
  /// \return Acceptance decision; on rejection `errors` holds the VDA5050 errors.
  AcceptanceResult validate_order(
    const types::Order& incoming_order,
    const std::shared_ptr<AGVContext>& context) const;

private:
  ReachabilityCheck reachable_;
  CapabilityCheck capable_;
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__STRATEGIES__ORDER_VALIDATOR_HPP_
