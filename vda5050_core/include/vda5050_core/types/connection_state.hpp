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

#ifndef VDA5050_CORE__TYPES__CONNECTION_STATE_HPP_
#define VDA5050_CORE__TYPES__CONNECTION_STATE_HPP_

namespace vda5050_core {

namespace types {

/// \brief Enum values for connectionState
enum class ConnectionState
{
  /// \brief The connection between the broker and AGV is active
  ONLINE,

  /// \brief The connection between AGV and broker has gone offline in a
  /// coordinated way
  OFFLINE,

  /// \brief The connection between AGV and broker has ended unexpectedly
  /// (e.g. used in MQTT last-will-message)
  CONNECTIONBROKEN
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__CONNECTION_STATE_HPP_
