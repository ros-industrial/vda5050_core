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

#ifndef VDA5050_TYPES__SAFETY_STATE_HPP_
#define VDA5050_TYPES__SAFETY_STATE_HPP_

#include "vda5050_types/e_stop.hpp"

namespace vda5050_types {

/// \brief Provides information about the safety state of the AGV
/// Part of the state message
struct SafetyState
{
  /// \brief Acknowledgement type of the emergency stop
  EStop e_stop = EStop::NONE;

  /// \brief Protective field violation
  ///   - True if field is violated
  ///   - False if field is not violated
  bool field_violation = false;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const SafetyState& other) const
  {
    if (this->e_stop != other.e_stop) return false;
    if (this->field_violation != other.field_violation) return false;

    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const SafetyState& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__SAFETY_STATE_HPP_
