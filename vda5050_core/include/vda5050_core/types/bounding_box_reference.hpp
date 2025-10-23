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

#ifndef VDA5050_CORE__TYPES__BOUNDING_BOX_REFERENCE_HPP_
#define VDA5050_CORE__TYPES__BOUNDING_BOX_REFERENCE_HPP_

#include <nlohmann/json.hpp>
#include <optional>

namespace vda5050_core {

namespace types {

/// \struct BoundingBoxReference
/// \brief  Point of reference for the location of the bounding box.
///         The point of reference is always the center of the bounding box bottom surface (at height = 0)
///         and is described in coordinates of the AGV coordinate system.
struct BoundingBoxReference
{
  /// \brief X-coordinate of the point of reference.
  double x = 0.0;

  /// \brief Y-coordinate of the point of reference.
  double y = 0.0;

  /// \brief Z-coordinate of the point of reference.
  double z = 0.0;

  /// \brief Orientation of the loads bounding box. Important for tugger, trains, etc.
  std::optional<double> theta;

  /// \brief Compares two BoundingBoxReference objects for equality.
  /// \param other The BoundingBoxReference instance to compare with.
  /// \return True if all position and orientation fields are equal, otherwise false.
  inline bool operator==(const BoundingBoxReference& other) const noexcept(true)
  {
    if (this->x != other.x) return false;
    if (this->y != other.y) return false;
    if (this->z != other.z) return false;
    if (this->theta != other.theta) return false;
    return true;
  }

  /// \brief Compares two BoundingBoxReference objects for inequality.
  /// \param other The BoundingBoxReference instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const BoundingBoxReference& other) const noexcept(true)
  {
    return !(this->operator==(other));
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const BoundingBoxReference& ref)
{
  j = json{{"x", ref.x}, {"y", ref.y}, {"z", ref.z}};

  if (ref.theta.has_value())
  {
    j["theta"] = ref.theta.value();
  }
}

inline void from_json(const json& j, BoundingBoxReference& ref)
{
  j.at("x").get_to(ref.x);
  j.at("y").get_to(ref.y);
  j.at("z").get_to(ref.z);

  if (j.contains("theta") && !j.at("theta").is_null())
  {
    ref.theta = j.at("theta").get<double>();
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__BOUNDING_BOX_REFERENCE_HPP_
