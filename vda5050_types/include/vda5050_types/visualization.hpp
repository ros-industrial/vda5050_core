/**
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

#ifndef VDA5050_TYPES__VISUALIZATION_HPP_
#define VDA5050_TYPES__VISUALIZATION_HPP_

#include <optional>
#include "vda5050_types/agv_position.hpp"
#include "vda5050_types/header.hpp"
#include "vda5050_types/velocity.hpp"

namespace vda5050_types {

/// \brief A message containing AGV position and/or velocity for visualization
/// Published by the AGV on topic /visualization for near-real time
/// position update.
struct Visualization
{
  /// \brief Message header that contains the common fields required across all messages
  Header header;

  /// \brief Position of the AGV
  std::optional<AGVPosition> agv_position;

  /// \brief Velocity of the AGV in vehicle coordinates
  std::optional<Velocity> velocity;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Visualization& other) const
  {
    if (this->header != other.header) return false;
    if (this->agv_position != other.agv_position) return false;
    if (this->velocity != other.velocity) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Visualization& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__VISUALIZATION_HPP_
