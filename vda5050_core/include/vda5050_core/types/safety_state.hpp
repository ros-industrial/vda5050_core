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

#ifndef VDA5050_CORE__TYPES__SAFETY_STATE_HPP_
#define VDA5050_CORE__TYPES__SAFETY_STATE_HPP_

#include <nlohmann/json.hpp>

#include "vda5050_core/types/e_stop.hpp"

namespace vda5050_core {

namespace types {

/// \struct SafetyState
/// \brief  Contains all safety-related information
struct SafetyState
{
  /// \brief Acknowledge-Type of eStop:
  ///        AUTOACK: auto-acknowledgeable e-stop is activated, e.g., by bumper or protective field.
  ///        MANUAL: e-stop hast to be acknowledged manually at the vehicle.
  ///        REMOTE: facility e-stop has to be acknowledged remotely.
  ///        NONE: no e-stop activated.
  EStop e_stop = EStop::NONE;

  /// \brief Protective field violation. True: field is violated. False: field is not violated.
  bool field_violation = false;

  ///
  /// \brief Equality operator
  ///
  /// \param other the other object to compare to
  /// \return is equal?
  ///
  inline bool operator==(const SafetyState &other) const {
    if (this->e_stop != other.e_stop) return false;
    if (this->field_violation != other.field_violation) return false;

    return true;
  }

  ///
  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  /// \return is not equal?
  ///
  inline bool operator!=(const SafetyState &other) const { return !this->operator==(other); }
};

using json = nlohmann::json;

inline void to_json(json& j, const SafetyState& s)
{
  j = json{{"e_stop", s.e_stop}, {"field_violation", s.field_violation}};
}

inline void from_json(const json& j, SafetyState& s)
{
  j.at("e_stop").get_to(s.e_stop);
  j.at("field_violation").get_to(s.field_violation);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__SAFETY_STATE_HPP_
