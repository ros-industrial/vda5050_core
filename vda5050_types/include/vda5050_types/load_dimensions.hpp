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

#ifndef VDA5050_TYPES__LOAD_DIMENSIONS_HPP_
#define VDA5050_TYPES__LOAD_DIMENSIONS_HPP_

#include <optional>

namespace vda5050_types {

/// \brief Dimensions of the load's bounding box in meters
/// Part of the state message
struct LoadDimensions
{
  /// \brief Absolute length of the loads bounding box in meter
  double length = 0.0;

  /// \brief Absolute width of the loads bounding box in meter
  double width = 0.0;

  /// \brief Absolute height of the loads bounding box in meter
  std::optional<double> height;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const LoadDimensions& other) const
  {
    if (this->length != other.length) return false;
    if (this->width != other.width) return false;
    if (this->height != other.height) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const LoadDimensions& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__LOAD_DIMENSIONS_HPP_
