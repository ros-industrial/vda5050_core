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

#include "vda5050_core/master/validation/pre_send_validator.hpp"

#include <string>

#include "vda5050_core/errors/error_codes.hpp"
#include "vda5050_core/errors/error_factory.hpp"

namespace vda5050_core::master {

vda5050_core::order_utils::ValidationResult validate_pre_send(
  const PreSendContext& ctx)
{
  vda5050_core::order_utils::ValidationResult res;

  auto add_error = [&](const std::string& description) {
    res.errors.push_back(vda5050_core::errors::create_error(
      vda5050_core::errors::PreSendValidationError, description, {}));
  };

  // No-map gate: can't validate node/edge ids without a loaded topology.
  if (ctx.loaded_map == nullptr)
  {
    res.errors.push_back(vda5050_core::errors::create_error(
      vda5050_core::errors::MapValidationError,
      "Master has no map loaded — call load_map_from_config() before "
      "publishing.",
      {}));
    return res;
  }

  if (ctx.connection_status != vda5050_core::types::ConnectionState::ONLINE)
  {
    add_error("AGV connection_status is not ONLINE");
  }

  if (
    ctx.operational_state == AGVState::ERROR ||
    ctx.operational_state == AGVState::STATE_UNKNOWN)
  {
    add_error(
      std::string("AGV operational_state is ") +
      (ctx.operational_state == AGVState::ERROR ? "ERROR" : "STATE_UNKNOWN"));
  }

  if (!ctx.last_state.has_value())
  {
    add_error("AGV has not yet reported any State");
    return res;
  }

  if (
    ctx.last_state->operating_mode !=
    vda5050_core::types::OperatingMode::AUTOMATIC)
  {
    add_error("AGV operating_mode is not AUTOMATIC");
  }

  if (
    !ctx.last_state->agv_position.has_value() ||
    !ctx.last_state->agv_position->position_initialized)
  {
    add_error("AGV position is not initialized");
  }

  return res;
}

}  // namespace vda5050_core::master
