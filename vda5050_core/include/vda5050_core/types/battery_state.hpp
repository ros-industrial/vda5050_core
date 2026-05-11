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
#include <optional>

namespace vda5050_core {

namespace types {

/// \brief Provides information related to the battery
/// Part of the state message
struct BatteryState
{
  /// \brief State of charge in percent (%). If the AGV only provides values
  /// for good or bad battery levels, these will be indicated as
  /// 20% (bad) and 80% (good)
  double battery_charge = 0.0;

  /// \brief Value of battery voltage
  std::optional<double> battery_voltage;

  /// \brief State of health of the battery in percent
  std::optional<int8_t> battery_health;

  /// \brief Charging state of the AGV. True if charging is in progress
  bool charging = false;

  /// \brief Estimated reach with current state of charge in meters
  std::optional<uint32_t> reach;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const BatteryState& other) const
  {
    if (battery_charge != other.battery_charge) return false;
    if (battery_voltage != other.battery_voltage) return false;
    if (battery_health != other.battery_health) return false;
    if (charging != other.charging) return false;
    if (reach != other.reach) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const BatteryState& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__BATTERY_STATE_HPP_
