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

#ifndef VDA5050_TYPES__EDGE_STATE_HPP_
#define VDA5050_TYPES__EDGE_STATE_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include "vda5050_types/trajectory.hpp"

namespace vda5050_types {

/// @struct ControlPoint
/// @brief  ControlPoint describing a trajectory (NURBS)
struct EdgeState
{
  /// @brief Unique edge identification
  std::string edge_id;

  /// @brief sequenceId to differentiate between multiple edges with
  ///        the same edgeId
  uint32_t sequence_id = 0;

  /// @brief Additional information on the edge
  std::optional<std::string> edge_description;

  /// @brief True indicates that the edge is part of the base.
  ///        False indicates that the edge is part of the horizon.
  bool released = false;

  /// @brief The trajectory is to be communicated as a NURBS.
  ///        Trajectory segments are from the point where the AGV
  ///        starts to enter the edge until the point where it
  ///        reports that the next node was traversed.
  std::optional<Trajectory> trajectory;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__EDGE_STATE_HPP_
