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

#ifndef VDA5050_CORE__TYPES__BLOCKING_TYPE_HPP_
#define VDA5050_CORE__TYPES__BLOCKING_TYPE_HPP_

#include <ostream>

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

/// \enum BlockingType
/// \brief  Enums for BlockingType
enum class BlockingType
{
  /// \brief “NONE” – allows driving and other actions
  NONE,
  /// \brief “SOFT” – allows other actions, but not driving
  SOFT,
  /// \brief “HARD” – is the only allowed action at that time.
  HARD
};

/// \brief Stream output operator for BlockingType.
/// \param os The output stream to write to.
/// \param blocking_type The BlockingType value to convert to a string.
/// \return A reference to the output stream.
constexpr std::ostream& operator<<(
  std::ostream& os, const BlockingType& blocking_type)
{
  switch (blocking_type)
  {
    case BlockingType::NONE:
      os << "NONE";
      break;
    case BlockingType::SOFT:
      os << "SOFT";
      break;
    case BlockingType::HARD:
      os << "HARD";
      break;
    default:
      os.setstate(std::ios_base::failbit);
  }
  return os;
}

using json = nlohmann::json;

inline void to_json(json& j, const BlockingType& d)
{
  switch (d)
  {
    case BlockingType::SOFT:
      j = "SOFT";
      break;
    case BlockingType::HARD:
      j = "HARD";
      break;
    case BlockingType::NONE:
      j = "NONE";
      break;
    default:
      j = "UNKNOWN";
      break;
  }
}

inline void from_json(const json& j, BlockingType& d)
{
  auto str = j.get<std::string>();
  if (str == "SOFT")
  {
    d = BlockingType::SOFT;
  }
  else if (str == "HARD")
  {
    d = BlockingType::HARD;
  }
  else if (str == "NONE")
  {
    d = BlockingType::NONE;
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__BLOCKING_TYPE_HPP_
