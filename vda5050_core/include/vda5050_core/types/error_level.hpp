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

#ifndef VDA5050_CORE__TYPES__ERROR_LEVEL_HPP_
#define VDA5050_CORE__TYPES__ERROR_LEVEL_HPP_

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

/// \enum ErrorLevel
/// \brief Enum {'WARNING', 'FATAL'}
///        'WARNING': AGV is ready to start
///        (e.g., maintenance cycle expiration
///        warning).
///        'FATAL': AGV is not in running
///        condition, user intervention required
///        (e.g., laser scanner is contaminated).
enum class ErrorLevel
{
  /// \brief AGV is ready to start (e.g. maintenance
  ///          cycle expiration warning)
  WARNING,
  /// \brief AGV is not in running condition, user
  ///        intervention required (e.g. laser scanner
  ///        is contaminated)
  FATAL
};

using json = nlohmann::json;

inline void to_json(json& j, const ErrorLevel& level)
{
  switch (level)
  {
    case ErrorLevel::WARNING:
      j = "WARNING";
      break;
    case ErrorLevel::FATAL:
      j = "FATAL";
      break;
  }
}

inline void from_json(const json& j, ErrorLevel& level)
{
  const std::string& s = j.get<std::string>();
  if (s == "WARNING")
    level = ErrorLevel::WARNING;
  else if (s == "FATAL")
    level = ErrorLevel::FATAL;
  else
    throw std::invalid_argument("Invalid ErrorLevel value: " + s);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ERROR_LEVEL_HPP_
