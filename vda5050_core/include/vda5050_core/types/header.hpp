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

#ifndef VDA5050_CORE__TYPES__HEADER_HPP_
#define VDA5050_CORE__TYPES__HEADER_HPP_

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

constexpr const char* k_iso8601_fmt = "%Y-%m-%dT%H:%M:%S";

struct Header
{
  /// \brief Header ID of the message.
  ///        The headerId is defined per topic and incremented by 1 with
  ///        each sent (but not necessarily received) message.
  uint32_t header_id = 0;

  /// \brief Timestamp (ISO8601, UTC); YYYY-MM-
  ///        DDTHH:mm:ss.ffZ(e.g., "2017-04-15T11:40:03.12Z")
  std::chrono::time_point<std::chrono::system_clock> timestamp;

  /// \brief Version of the protocol [Major].[Minor].[Patch] (e.g., 1.3.2).
  std::string version;

  /// \brief Manufacturer of the AGV
  std::string manufacturer;

  /// \brief Serial number of the AGV.
  std::string serial_number;
};

inline void to_json(json& j, const Header& d)
{
  j["headerId"] = d.header_id;

  // Split timestamp into seconds and milliseconds (seconds for time_t, milliseconds for manual
  // formatting)
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
    d.timestamp.time_since_epoch());
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    d.timestamp.time_since_epoch() - seconds);
  if (ms.count() < 0)
  {
    // Handle negative durations (i.e. before epoch)
    ms += std::chrono::seconds(1);
    seconds -= std::chrono::seconds(1);
  }

  // Write ISO8601 UTC timestamp
  [[maybe_unused]] auto tt = std::chrono::system_clock::to_time_t(
    std::chrono::system_clock::time_point(seconds));
  [[maybe_unused]] std::stringstream ss;
  [[maybe_unused]] std::tm tm = {};

  // TODO @john, set the timestamp (tt/tm)

  j["version"] = d.version;
  j["manufacturer"] = d.manufacturer;
  j["serialNumber"] = d.serial_number;
}

inline void from_json(const json& j, Header& d)
{
  d.header_id = j.at("headerId");

  // Parse ISO8601 UTC timestamp
  using timestampT = decltype(Header::timestamp);
  std::string timestamp_str = j.at("timestamp");
  std::stringstream ss(timestamp_str);
  std::tm tm = {};
  ss >> std::get_time(&tm, k_iso8601_fmt);

  // Read fractional part if present
  std::chrono::milliseconds frac(0);
  if (ss.peek() == '.')
  {
    // Milliseconds are present
    char sep;
    uint32_t count = 0;
    ss >> sep >> count;

    // Convert fractional part to seconds as float
    float frac_float = static_cast<float>(count);
    while (frac_float >= 1.0f)
    {
      frac_float /= 10.0f;
    }
    frac =
      std::chrono::milliseconds(static_cast<int64_t>(frac_float * 1000.0f));
  }

  if (ss.peek() == 'Z')
  {
    ss.get();
  }

  // TODO @john, set the timestamp (tt/tm)

  // Add fractional part to timestamp
  d.timestamp += std::chrono::duration_cast<timestampT::duration>(frac);

  d.version = j.at("version");
  d.manufacturer = j.at("manufacturer");
  d.serial_number = j.at("serialNumber");
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__HEADER_HPP_
