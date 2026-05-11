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

#ifndef VDA5050_CORE__TYPES__ENVELOPE2D_HPP_
#define VDA5050_CORE__TYPES__ENVELOPE2D_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/polygon_point.hpp"

namespace vda5050_core {

namespace types {

/// \brief AGV envelope curves in 2D.
struct Envelope2d
{
  /// \brief Name of the envelope curve set.
  std::string set;

  /// \brief Envelope curve as an x/y-polygon. Polygon is assumed as closed
  /// and shall be non-self-intersecting.
  std::vector<PolygonPoint> polygon_points;

  /// \brief Free-form text: description of envelope curve set.
  std::optional<std::string> description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Envelope2d& other) const
  {
    if (this->set != other.set) return false;
    if (this->polygon_points != other.polygon_points) return false;
    if (this->description != other.description) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Envelope2d& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ENVELOPE2D_HPP_
