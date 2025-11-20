/**
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

#ifndef VDA5050_TYPES__VEHICLE_CONFIG_HPP_
#define VDA5050_TYPES__VEHICLE_CONFIG_HPP_

#include <optional>
#include <vector>
#include "vda5050_types/network.hpp"
#include "vda5050_types/version_info.hpp"

namespace vda5050_types {

/// \brief Details the software and hardware versions running on the vehicle,
/// as well as a brief summary of network information.
struct VehicleConfig
{
  /// \brief Array of key-value pair objects containing software and hardware information.
  std::optional<std::vector<VersionInfo>> versions;

  /// \brief Information about the vehicle's network connection.
  /// The listed information shall not be updated while the vehicle is operating.
  std::optional<Network> network;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const VehicleConfig& other) const
  {
    if (this->versions != other.versions) return false;
    if (this->network != other.network) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const VehicleConfig& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__VEHICLE_CONFIG_HPP_
