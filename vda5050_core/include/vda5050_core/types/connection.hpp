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

#ifndef VDA5050_CORE__TYPES__CONNECTION_HPP_
#define VDA5050_CORE__TYPES__CONNECTION_HPP_

#include "vda5050_core/types/connection_state.hpp"
#include "vda5050_core/types/header.hpp"

namespace vda5050_core {

namespace types {

/// \brief A message containing the connection information
/// published on the /connection topic
struct Connection
{
  /// \brief The header of the message
  Header header;

  /// \brief State of the connection of the AGV
  ConnectionState connection_state = ConnectionState::OFFLINE;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Connection& other) const
  {
    return header == other.header && connection_state == other.connection_state;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Connection& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__CONNECTION_HPP_
