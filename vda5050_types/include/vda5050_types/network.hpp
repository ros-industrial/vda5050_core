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

#ifndef VDA5050_TYPES__NETWORK_HPP_
#define VDA5050_TYPES__NETWORK_HPP_

#include <optional>
#include <string>
#include <vector>

namespace vda5050_types {

/// \brief Information about the vehicle's network configuration.
struct Network
{
  /// \brief Array of Domain Name Servers (DNS) used by the vehicle.
  std::optional<std::vector<std::string>> dns_servers;

  /// \brief Array of Network Time Protocol (NTP) servers used by the vehicle.
  std::optional<std::vector<std::string>> ntp_servers;

  /// \brief A priori assigned IP address used to communicate with the MQTT broker.
  /// Note: This IP address should not be modified/changed during operations.
  std::optional<std::string> local_ip_address;

  /// \brief The subnet mask used in the network configuration corresponding to the local IP address.
  std::optional<std::string> netmask;

  /// \brief The default gateway used by the vehicle, corresponding to the local IP address.
  std::optional<std::string> default_gateway;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Network& other) const
  {
    if (this->dns_servers != other.dns_servers) return false;
    if (this->ntp_servers != other.ntp_servers) return false;
    if (this->local_ip_address != other.local_ip_address) return false;
    if (this->netmask != other.netmask) return false;
    if (this->default_gateway != other.default_gateway) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Network& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__NETWORK_HPP_
