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
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "vda5050_core/types/trajectory.hpp"

namespace vda5050_core {

namespace types {

/// \struct ControlPoint
/// \brief  ControlPoint describing a trajectory (NURBS)
struct EdgeState
{
  /// \brief Unique edge identification
  std::string edge_id;

  /// \brief sequenceId to differentiate between multiple edges with
  ///        the same edgeId
  uint32_t sequence_id = 0;

  /// \brief Additional information on the edge
  std::optional<std::string> edge_description;

  /// \brief True indicates that the edge is part of the base.
  ///        False indicates that the edge is part of the horizon.
  bool released = false;

  /// \brief The trajectory is to be communicated as a NURBS.
  ///        Trajectory segments are from the point where the AGV
  ///        starts to enter the edge until the point where it
  ///        reports that the next node was traversed.
  std::optional<Trajectory> trajectory;
};

using json = nlohmann::json;

inline void to_json(json& j, const EdgeState& state)
{
  j = json{
    {"edge_id", state.edge_id},
    {"sequence_id", state.sequence_id},
    {"released", state.released}};

  if (state.edge_description.has_value())
  {
    j["edge_description"] = state.edge_description.value();
  }

  if (state.trajectory.has_value())
  {
    j["trajectory"] = state.trajectory.value();
  }
}

inline void from_json(const json& j, EdgeState& state)
{
  j.at("edge_id").get_to(state.edge_id);
  j.at("sequence_id").get_to(state.sequence_id);
  j.at("released").get_to(state.released);

  if (j.contains("edge_description") && !j.at("edge_description").is_null())
  {
    state.edge_description = j.at("edge_description").get<std::string>();
  }
  else
  {
    state.edge_description = std::nullopt;
  }

  if (j.contains("trajectory") && !j.at("trajectory").is_null())
  {
    state.trajectory = j.at("trajectory").get<Trajectory>();
  }
  else
  {
    state.trajectory = std::nullopt;
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__EDGE_STATE_HPP_
