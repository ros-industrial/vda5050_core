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

#ifndef VDA5050_CORE__TYPES__NODE_POSITION_HPP_
#define VDA5050_CORE__TYPES__NODE_POSITION_HPP_

#include <nlohmann/json.hpp>
#include <optional>

namespace vda5050_core {

namespace types {

/// \struct NodePosition
/// \brief  Node position. The object is defined in chapter 5.4 Topic: Order (from master control to AGV).
///         Optional:Master control has this information.
///         Can be sent additionally, e.g., for debugging purposes.
struct NodePosition
{
  /// \brief X-position on the map in reference to the
  ///        map coordinate system in meters.
  double x = 0.0;

  /// \brief Y-position on the map in reference to the
  ///        map coordinate system in meters.
  double y = 0.0;

  /// \brief Range : [-PI... PI]
  ///        Absolute orientation of the AGV on the node.
  ///        Optional: vehicle can plan the path by itself.
  ///        If defined, the AGV has to assume the theta
  ///        angle on this node.
  //         If previous edge disallows rotation, the AGV
  //         shall rotate on the node.
  //         If following edge has a differing orientation
  //         defined but disallows rotation, the AGV is to
  //         rotate on the node to the edges desired
  //         rotation before entering the edge.
  std::optional<double> theta;

  /// \brief Indicates how precisely an AGV shall match
  ///        the position of a node for it to be considered traversed.
  ///
  ///        If = 0.0: no deviation is allowed (no
  ///        deviation means within the normal
  ///        tolerance of the AGV manufacturer).
  ///        If > 0.0: allowed deviation radius in meters.
  ///        If the AGV passes a node within the
  ///        deviation radius, the node can be
  ///        considered traversed.
  std::optional<double> allowed_deviation_x_y;

  /// \brief Range: [0.0 … Pi] Indicates how precise the orientation
  ///        defined in theta has to be met on the node
  ///        by the AGV.
  ///        Indicates how precise the orientation
  ///        defined in theta has to be met on the node
  ///        by the AGV.
  ///        The lowest acceptable angle is theta -
  ///        allowedDeviationTheta and the highest
  ///        acceptable angle is theta +
  ///        allowedDeviationTheta.
  std::optional<double> allowed_deviation_theta;

  /// \brief Unique identification of the map on which
  ///        the position is referenced.
  ///        Each map has the same project-specific
  ///        global origin of coordinates.
  ///        When an AGV uses an elevator, e.g.,
  ///        leading from a departure floor to a target
  ///        floor, it will disappear off the map of the
  ///        departure floor and spawn in the related lift
  ///        node on the map of the target floor.
  std::string mapId;

  /// \brief Additional information on the map
  std::optional<std::string> map_description;

  /// \brief Compares two NodePosition objects for equality.
  /// \param other The NodePosition instance to compare with.
  /// \return True if allowed_deviation_theta, allowed_deviation_x_y, map_description, mapId, theta, x, and y are equal, otherwise false
  inline bool operator==(const NodePosition& other) const
  {
    if (this->allowed_deviation_theta != other.allowed_deviation_theta)
      return false;
    if (this->allowed_deviation_x_y != other.allowed_deviation_x_y)
      return false;
    if (this->map_description != other.map_description) return false;
    if (this->mapId != other.mapId) return false;
    if (this->theta != other.theta) return false;
    if (this->x != other.x) return false;
    if (this->y != other.y) return false;

    return true;
  }

  /// \brief Compares two NodePosition objects for inequality.
  /// \param other The NodePosition instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const NodePosition& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const NodePosition& n)
{
  j = json{{"x", n.x}, {"y", n.y}, {"mapId", n.mapId}};

  if (n.theta.has_value())
  {
    j["theta"] = n.theta.value();
  }

  if (n.allowed_deviation_x_y.has_value())
  {
    j["allowedDeviationXY"] = n.allowed_deviation_x_y.value();
  }

  if (n.allowed_deviation_theta.has_value())
  {
    j["allowedDeviationTheta"] = n.allowed_deviation_theta.value();
  }

  if (n.map_description.has_value())
  {
    j["mapDescription"] = n.map_description.value();
  }
}

inline void from_json(const json& j, NodePosition& n)
{
  j.at("x").get_to(n.x);
  j.at("y").get_to(n.y);
  j.at("mapId").get_to(n.mapId);

  if (j.contains("theta") && !j.at("theta").is_null())
  {
    n.theta = j.at("theta").get<double>();
  }

  if (
    j.contains("allowedDeviationXY") &&
    !j.at("allowedDeviationXY").is_null())
  {
    n.allowed_deviation_x_y = j.at("allowedDeviationXY").get<double>();
  }

  if (
    j.contains("allowedDeviationTheta") &&
    !j.at("allowedDeviationTheta").is_null())
  {
    n.allowed_deviation_theta = j.at("allowedDeviationTheta").get<double>();
  }

  if (j.contains("mapDescription") && !j.at("mapDescription").is_null())
  {
    n.map_description = j.at("mapDescription").get<std::string>();
  }
}

}  // namespace types

}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__NODE_POSITION_HPP_
