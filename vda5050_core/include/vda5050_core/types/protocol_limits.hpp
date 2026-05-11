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

#ifndef VDA5050_CORE__TYPES__PROTOCOL_LIMITS_HPP_
#define VDA5050_CORE__TYPES__PROTOCOL_LIMITS_HPP_

#include "vda5050_core/types/max_array_lens.hpp"
#include "vda5050_core/types/max_string_lens.hpp"
#include "vda5050_core/types/timing.hpp"

namespace vda5050_core {

namespace types {

/// \brief Message describing the protocol limitations of the AGV.
/// If a parameter is not defined or set to zero then there is no explicit limit
/// for that parameter
struct ProtocolLimits
{
  /// \brief Maximum lengths of strings
  MaxStringLens max_string_lens;

  /// \brief Maximum lengths of arrays
  MaxArrayLens max_array_lens;

  /// \brief Timing information
  Timing timing;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const ProtocolLimits& other) const
  {
    if (this->max_string_lens != other.max_string_lens) return false;
    if (this->max_array_lens != other.max_array_lens) return false;
    if (this->timing != other.timing) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const ProtocolLimits& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__PROTOCOL_LIMITS_HPP_
