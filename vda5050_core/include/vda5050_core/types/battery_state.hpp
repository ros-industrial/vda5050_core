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

#ifndef VDA5050_CORE__TYPES__BATTERY_STATE_HPP_
#define VDA5050_CORE__TYPES__BATTERY_STATE_HPP_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>

namespace vda5050_core {

namespace types {

/// \struct BatteryState
/// \brief Contains all battery-related
///information.
struct BatteryState
{
  /// \brief State of Charge:
  double battery_charge = 0.0;

  /// \brief Battery Voltage.
  std::optional<double> battery_voltage;

  /// \brief State describing the battery's health.
  /// \note  Valid range: [0, 100], unit: %
  std::optional<int8_t> battery_health;

  /// \brief “true”: charging in progress.
  ///        “false”: AGV is currently not charging.
  bool charging = false;

  /// \brief Estimated reach with current state of
  ///        charge.
  /// \note  Valid range: [0, uint32.max]
  std::optional<uint32_t> reach;

  /// \brief Compares two BatteryState objects for equality.
  /// \param other The BatteryState instance to compare with.
  /// \return True if all compared fields are equal, otherwise false.
  inline bool operator==(const BatteryState& other) const
  {
    if (battery_charge != other.battery_charge) return false;
    if (battery_voltage != other.battery_voltage) return false;
    if (battery_health != other.battery_health) return false;
    if (charging != other.charging) return false;
    if (reach != other.reach) return false;
    return true;
  }

  /// \brief Compares two BatteryState objects for inequality.
  /// \param other The BatteryState instance to compare with.
  /// \return True if any compared field differs, otherwise false.
  inline bool operator!=(const BatteryState& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const BatteryState& state)
{
  j = json{
    {"battery_charge", state.battery_charge}, {"charging", state.charging}};

  if (state.battery_voltage.has_value())
    j["battery_voltage"] = state.battery_voltage.value();

  if (state.battery_health.has_value())
    j["battery_health"] = state.battery_health.value();

  if (state.reach.has_value()) j["reach"] = state.reach.value();
}

inline void from_json(const json& j, BatteryState& state)
{
  j.at("battery_charge").get_to(state.battery_charge);
  j.at("charging").get_to(state.charging);

  if (j.contains("battery_voltage") && !j.at("battery_voltage").is_null())
    state.battery_voltage = j.at("battery_voltage").get<double>();
  else
    state.battery_voltage = std::nullopt;

  if (j.contains("battery_health") && !j.at("battery_health").is_null())
    state.battery_health = j.at("battery_health").get<int8_t>();
  else
    state.battery_health = std::nullopt;

  if (j.contains("reach") && !j.at("reach").is_null())
    state.reach = j.at("reach").get<uint32_t>();
  else
    state.reach = std::nullopt;
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__BATTERY_STATE_HPP_
