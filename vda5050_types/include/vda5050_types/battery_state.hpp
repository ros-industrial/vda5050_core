/*
 * Copyright (C) 2025 ROS-Industrial Consortium Asia Pacific
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

#ifndef VDA5050_TYPES__BATTERY_STATE_HPP_
#define VDA5050_TYPES__BATTERY_STATE_HPP_

#include <cstdint>
#include <optional>

namespace vda5050_types {

/// @struct BatteryState
/// @brief Contains all battery-related
///information.
struct BatteryState
{
  /// @brief State of Charge:
  double battery_charge = 0.0;

  /// @brief Battery Voltage.
  std::optional<double> battery_voltage;

  /// @brief State describing the battery's health.
  /// @note  Valid range: [0, 100], unit: %
  std::optional<int8_t> battery_health;

  /// @brief “true”: charging in progress.
  ///        “false”: AGV is currently not charging.
  bool charging = false;

  /// @brief Estimated reach with current state of
  ///        charge.
  /// @note  Valid range: [0, uint32.max]
  std::optional<uint32_t> reach;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__BATTERY_STATE_HPP_
