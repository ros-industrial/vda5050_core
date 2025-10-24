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

#ifndef VDA5050_TYPES__HEADER_HPP_
#define VDA5050_TYPES__HEADER_HPP_

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>

namespace vda5050_types {

constexpr const char* ISO8601_FORMAT = "%Y-%m-%dT%H:%M:%S";

struct Header
{
  /// @brief Header ID of the message.
  ///        The headerId is defined per topic and incremented by 1 with
  ///        each sent (but not necessarily received) message.
  uint32_t header_id = 0;

  /// @brief Timestamp (ISO8601, UTC); YYYY-MM-
  ///        DDTHH:mm:ss.ffZ(e.g., "2017-04-15T11:40:03.12Z")
  std::chrono::time_point<std::chrono::system_clock> timestamp;

  /// @brief Version of the protocol [Major].[Minor].[Patch] (e.g., 1.3.2).
  std::string version;

  /// @brief Manufacturer of the AGV
  std::string manufacturer;

  /// @brief Serial number of the AGV.
  std::string serial_number;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__HEADER_HPP_
