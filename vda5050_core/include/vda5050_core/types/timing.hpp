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

#ifndef VDA5050_CORE__TYPES__TIMING_HPP_
#define VDA5050_CORE__TYPES__TIMING_HPP_

#include <optional>

namespace vda5050_core {

namespace types {

/// \brief Timing information.
struct Timing
{
  /// \brief Minimum interval sending order messages to the AGV [s]
  float min_order_interval = 0.0;

  /// \brief Minimum interval for sending state messages [s]
  float min_state_interval = 0.0;

  /// \brief Default interval for sending state messages. If not defined, the default value
  /// from the main document is used [s]
  std::optional<float> default_state_interval;

  /// \brief Default interval for sending messages on visualization topic [s]
  std::optional<float> visualization_interval;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Timing& other) const
  {
    if (this->min_order_interval != other.min_order_interval) return false;
    if (this->min_state_interval != other.min_state_interval) return false;
    if (this->default_state_interval != other.default_state_interval)
      return false;
    if (this->visualization_interval != other.visualization_interval)
      return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Timing& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__TIMING_HPP_
