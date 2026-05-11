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

#ifndef VDA5050_CORE__TYPES__EDGE_STATE_HPP_
#define VDA5050_CORE__TYPES__EDGE_STATE_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include "vda5050_core/types/trajectory.hpp"

namespace vda5050_core {

namespace types {

/// \brief Provides information about the edge
/// Part of the state message
struct EdgeState
{
  /// \brief Unique edge identification
  std::string edge_id;

  /// \brief A sequence ID to differentiate multiple edges with the same ID
  uint32_t sequence_id = 0;

  /// \brief Additional information on the edge
  std::optional<std::string> edge_description;

  /// \brief Defines if the edge is part of the base or horizon
  ///   - True indicates edge is part of the base
  ///   - False indicates edge is part of the horizon
  bool released = false;

  /// \brief The trajectory in NURBS, defined as starting from the point
  /// the AGV starts to enter the edge until the point where it reports the
  /// next node as traversed
  std::optional<Trajectory> trajectory;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const EdgeState& other) const
  {
    if (this->edge_id != other.edge_id) return false;
    if (this->sequence_id != other.sequence_id) return false;
    if (this->edge_description != other.edge_description) return false;
    if (this->released != other.released) return false;
    if (this->trajectory != other.trajectory) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const EdgeState& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__EDGE_STATE_HPP_
