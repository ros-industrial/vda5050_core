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

#ifndef VDA5050_CORE__TYPES__INFO_LEVEL_HPP_
#define VDA5050_CORE__TYPES__INFO_LEVEL_HPP_

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

/// \enum InfoLevel
/// \brief Information for debugging or visualization
enum class InfoLevel
{
  /// \brief used for debugging
  DEBUG,

  /// \brief used for visualization
  INFO
};

/// \brief Outputs a textual representation of an InfoLevel value to the given stream.
/// \param os The output stream to write to.
/// \param info_level The InfoLevel value to be converted to text.
/// \return A reference to the modified output stream.
constexpr std::ostream& operator<<(
  std::ostream& os, const InfoLevel& info_level)
{
  switch (info_level)
  {
    case InfoLevel::DEBUG:
      os << "DEBUG";
      break;
    case InfoLevel::INFO:
      os << "INFO";
      break;
    default:
      os.setstate(std::ios_base::failbit);
  }
  return os;
}

using json = nlohmann::json;

inline void to_json(json& j, const InfoLevel& level)
{
  switch (level)
  {
    case InfoLevel::DEBUG:
      j = "DEBUG";
      break;
    case InfoLevel::INFO:
      j = "INFO";
      break;
  }
}

inline void from_json(const json& j, InfoLevel& level)
{
  const std::string& s = j.get<std::string>();
  if (s == "DEBUG")
    level = InfoLevel::DEBUG;
  else if (s == "INFO")
    level = InfoLevel::INFO;
  else
    throw std::invalid_argument("Invalid InfoLevel value: " + s);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__INFO_LEVEL_HPP_
