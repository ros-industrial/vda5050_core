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

#ifndef VDA5050_CORE__TYPES__OPERATING_MODE_HPP_
#define VDA5050_CORE__TYPES__OPERATING_MODE_HPP_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>

namespace vda5050_core {

namespace types {

/// @enum  OperatingMode
/// @brief Current operating mode of the AGV
enum class OperatingMode : uint8_t
{
  /// @brief AGV is under full control of the master control.
  ///        AGV drives and executes actions based on orders from the master
  ///        control.
  AUTOMATIC,

  /// \brief AGV is under control of the master control.
  //         AGV drives and executes actions based on orders from the master
  //         control.
  //         The driving speed is controlled by the HMI (speed can't exceed the
  //         speed of automatic mode).
  //         The steering is under automatic control (non-safe HMI possible).
  SEMIAUTOMATIC,

  /// @brief Master control is not in control of the AGV.
  //         Supervisor doesn't send driving order or actions to the AGV.
  //         HMI can be used to control the steering and velocity and handling
  //         device of the AGV.
  //         Location of the AGV is sent to the master control.
  //         When AGV enters or leaves this mode, it immediately clears all the
  //         orders (safe HMI required)
  MANUAL,

  /// @brief Master control is not in control of the AGV.
  //         Master control doesn't send driving order or actions to the AGV.
  //         Authorized personnel can reconfigure the AGV
  SERVICE,

  /// @brief Master control is not in control of the AGV.
  //         Supervisor doesn't send driving order or actions to the AGV.
  //         The AGV is being taught, e.g., mapping is done by a master control.
  TEACHIN
};

/// \brief Outputs a textual representation of an OperatingMode value to the given stream.
/// \param os The output stream to write to.
/// \param operating_mode The OperatingMode value to be converted to text.
/// \return A reference to the modified output stream.
constexpr std::ostream &operator<<(std::ostream &os, const OperatingMode &operating_mode) {
  switch (operating_mode) {
    case OperatingMode::AUTOMATIC:
      os << "AUTOMATIC";
      break;
    case OperatingMode::SEMIAUTOMATIC:
      os << "SEMIAUTOMATIC";
      break;
    case OperatingMode::MANUAL:
      os << "MANUAL";
      break;
    case OperatingMode::SERVICE:
      os << "SERVICE";
      break;
    case OperatingMode::TEACHIN:
      os << "TEACHIN";
      break;
    default:
      os.setstate(std::ios_base::failbit);
  }
  return os;
}

using json = nlohmann::json;

inline void to_json(json& j, const OperatingMode& mode)
{
  switch (mode)
  {
    case OperatingMode::AUTOMATIC:
      j = "AUTOMATIC";
      break;
    case OperatingMode::SEMIAUTOMATIC:
      j = "SEMIAUTOMATIC";
      break;
    case OperatingMode::MANUAL:
      j = "MANUAL";
      break;
    case OperatingMode::SERVICE:
      j = "SERVICE";
      break;
    case OperatingMode::TEACHIN:
      j = "TEACHIN";
      break;
    default:
      throw std::invalid_argument("Invalid OperatingMode enum value");
  }
}

inline void from_json(const json& j, OperatingMode& mode)
{
  const std::string& str = j.get<std::string>();

  if (str == "AUTOMATIC")
    mode = OperatingMode::AUTOMATIC;
  else if (str == "SEMIAUTOMATIC")
    mode = OperatingMode::SEMIAUTOMATIC;
  else if (str == "MANUAL")
    mode = OperatingMode::MANUAL;
  else if (str == "SERVICE")
    mode = OperatingMode::SERVICE;
  else if (str == "TEACHIN")
    mode = OperatingMode::TEACHIN;
  else
    throw std::invalid_argument("Invalid OperatingMode string: " + str);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__OPERATING_MODE_HPP_
