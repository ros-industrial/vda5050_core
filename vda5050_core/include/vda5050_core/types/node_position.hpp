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

#include <optional>
#include <string>

namespace vda5050_core {

namespace types {

/// \brief Defines a position on a map in world coordinate system
/// Part of the state message
struct NodePosition
{
  /// \brief X-position of the AGV on the map in reference to the
  /// world coordinate system in meters.
  double x = 0.0;

  /// \brief Y-position of the AGV on the map in reference to the
  /// world coordinate system in meters.
  double y = 0.0;

  /// \brief Orientation of the AGV in radians. If provided,
  /// the AGV must assume the orientation on this node
  ///    - If the previous edge, disallows rotation then the AGV
  ///      must rotate on the node
  ///    - If the following edge has a different orientation but
  ///      disallows rotation then the AGV must rotate on the node
  ///      before entering the edge
  /// Valid range: [-PI, PI]
  std::optional<double> theta;

  /// \brief Indicates how accurate the AGV must be to drive over a node
  /// for it to count as traversed
  ///    - If deviation = 0, then the AGV must traverese the node within the
  ///      normal tolerance of the AGV manufacturer
  ///    - If deviation > 0, then the AGV can traverse the node within the
  ///      deviation radius
  std::optional<double> allowed_deviation_x_y;

  /// \brief Indicates how precise the orientation defined in theta has to be met
  /// on the node by the AGV.
  /// Valid range: [0, PI]
  std::optional<double> allowed_deviation_theta;

  /// \brief Unique identification of the map on which the AGV is currently
  /// operating
  std::string map_id;

  /// \brief Additional information on the map
  std::optional<std::string> map_description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const NodePosition& other) const
  {
    if (this->allowed_deviation_theta != other.allowed_deviation_theta)
      return false;
    if (this->allowed_deviation_x_y != other.allowed_deviation_x_y)
      return false;
    if (this->map_description != other.map_description) return false;
    if (this->map_id != other.map_id) return false;
    if (this->theta != other.theta) return false;
    if (this->x != other.x) return false;
    if (this->y != other.y) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const NodePosition& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__NODE_POSITION_HPP_
