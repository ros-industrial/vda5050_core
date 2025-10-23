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

#ifndef VDA5050_CORE__TYPES__E_STOP_HPP_
#define VDA5050_CORE__TYPES__E_STOP_HPP_

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

/// \enum   EStop
/// \brief  Acknowledge-Type of eStop:
///         AUTOACK: auto-acknowledgeable e-stop is activated, e.g., by bumper or protective field.
///         MANUAL: e-stop hast to be acknowledged manually at the vehicle.
///         REMOTE: facility e-stop has to be acknowledged remotely.
///         NONE: no e-stop activated.
enum class EStop
{
  /// \brief Auto-acknowledgeable e-stop is activated e.g. by bumper or
  ///        protective field
  AUTOACK,

  /// \brief E-stop has to be acknowledged manually at the vehicle.
  MANUAL,

  /// \brief Facility e-stop has to be acknowledged remotely
  REMOTE,

  /// \brief No e-stop activated
  NONE
};

/// \brief Outputs a textual representation of an EStop value to the given stream.
/// \param os The output stream to write to.
/// \param e_stop The EStop value to be converted to text.
/// \return A reference to the modified output stream.
constexpr std::ostream& operator<<(std::ostream& os, const EStop& e_stop)
{
  switch (e_stop)
  {
    case EStop::AUTOACK:
      os << "AUTOACK";
      break;
    case EStop::MANUAL:
      os << "MANUAL";
      break;
    case EStop::REMOTE:
      os << "REMOTE";
      break;
    case EStop::NONE:
      os << "NONE";
      break;
    default:
      os.setstate(std::ios_base::failbit);
  }
  return os;
}

using json = nlohmann::json;

inline void to_json(json& j, const EStop& estop)
{
  switch (estop)
  {
    case EStop::AUTOACK:
      j = "AUTOACK";
      break;
    case EStop::MANUAL:
      j = "MANUAL";
      break;
    case EStop::REMOTE:
      j = "REMOTE";
      break;
    case EStop::NONE:
      j = "NONE";
      break;
  }
}

inline void from_json(const json& j, EStop& estop)
{
  const std::string& s = j.get<std::string>();
  if (s == "AUTOACK")
    estop = EStop::AUTOACK;
  else if (s == "MANUAL")
    estop = EStop::MANUAL;
  else if (s == "REMOTE")
    estop = EStop::REMOTE;
  else if (s == "NONE")
    estop = EStop::NONE;
  else
    throw std::invalid_argument("Invalid EStop value: " + s);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__E_STOP_HPP_
