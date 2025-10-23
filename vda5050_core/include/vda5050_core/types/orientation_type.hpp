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

#ifndef VDA5050_CORE__TYPES__ORIENTATION_TYPE_HPP_
#define VDA5050_CORE__TYPES__ORIENTATION_TYPE_HPP_

#include <cstdint>
#include <ostream>

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

enum class OrientationType : uint8_t
{
  /// \brief tangential to the edge.
  TANGENTIAL,

  /// \brief relative to the global project specific map coordinate system
  GLOBAL
};

/// \brief Stream output operator for OrientationType.
/// \param os The output stream to write to.
/// \param orientation_type The OrientationType value to convert to a string.
/// \return A reference to the output stream.
constexpr std::ostream& operator<<(
  std::ostream& os, const OrientationType& orientation_type)
{
  switch (orientation_type)
  {
    case OrientationType::TANGENTIAL:
      os << "TANGENTIAL";
      break;
    case OrientationType::GLOBAL:
      os << "GLOBAL";
      break;
    default:
      os.setstate(std::ios_base::failbit);
  }
  return os;
}

using json = nlohmann::json;

inline void to_json(json& j, const OrientationType& d)
{
  switch (d)
  {
    case OrientationType::GLOBAL:
      j = "GLOBAL";
      break;
    case OrientationType::TANGENTIAL:
      [[fallthrough]];
    default:
      j = "TANGENTIAL";
      break;
  }
}
inline void from_json(const json& j, OrientationType& d)
{
  auto str = j.get<std::string>();
  if (str == "TANGENTIAL")
  {
    d = OrientationType::TANGENTIAL;
  }
  else if (str == "GLOBAL")
  {
    d = OrientationType::GLOBAL;
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ORIENTATION_TYPE_HPP_
