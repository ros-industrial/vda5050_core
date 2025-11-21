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

#ifndef VDA5050_TYPES__LOAD_SET_HPP_
#define VDA5050_TYPES__LOAD_SET_HPP_

#include <optional>
#include <string>
#include <vector>
#include "vda5050_types/bounding_box_reference.hpp"
#include "vda5050_types/load_dimensions.hpp"

namespace vda5050_types {

/// \brief Represents one load set supported by the AGV.
struct LoadSet
{
  /// \brief Unique name of the load set (e.g., DEFAULT, SET1, etc.)
  std::string set_name;

  /// \brief Type of load (e.g., EPAL, XLT1200, etc.)
  std::string load_type;

  /// \brief Array of load positions between load handling devices that this
  /// load set is valid for. If this parameter does not exist or is empty, this
  /// load set is valid for all load handling devices on this AGV.
  std::optional<std::vector<std::string>> load_positions;

  /// \brief Bounding box reference as defined in parameter loads[] in the state
  /// message.
  std::optional<BoundingBoxReference> bounding_box_reference;

  /// \brief Load dimensions as defined in parameter loads[] in the state
  /// message.
  std::optional<LoadDimensions> load_dimensions;

  /// \brief Maximum weight of this load type [kg].
  std::optional<double> max_weight;

  /// \brief Minimum allowed height for handling this load type [m].
  /// References bounding_box_reference.
  std::optional<double> min_load_handling_height;

  /// \brief Maximum allowed height for handling this load type [m].
  /// References bounding_box_reference.
  std::optional<double> max_load_handling_height;

  /// \brief Minimum allowed depth for handling this load type [m].
  /// References bounding_box_reference.
  std::optional<double> min_load_handling_depth;

  /// \brief Maximum allowed depth for handling this load type [m].
  /// References bounding_box_reference.
  std::optional<double> max_load_handling_depth;

  /// \brief Minimum allowed tilt for handling this load type [rad].
  std::optional<double> min_load_handling_tilt;

  /// \brief Maximum allowed tilt for handling this load type [rad].
  std::optional<double> max_load_handling_tilt;

  /// \brief Maximum allowed speed for handling this load type [m/s].
  std::optional<double> agv_speed_limit;

  /// \brief Maximum allowed acceleration for handling this load type [m/s²].
  std::optional<double> agv_acceleration_limit;

  /// \brief Maximum allowed deceleration for handling this load type [m/s²].
  std::optional<double> agv_deceleration_limit;

  /// \brief Approximate time for picking up the load [s].
  std::optional<double> pick_time;

  /// \brief Approximate time for dropping the load [s].
  std::optional<double> drop_time;

  /// \brief Free-form description of this load handling set.
  std::optional<std::string> description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const LoadSet& other) const
  {
    if (this->set_name != other.set_name) return false;
    if (this->load_type != other.load_type) return false;
    if (this->load_positions != other.load_positions) return false;
    if (this->bounding_box_reference != other.bounding_box_reference)
      return false;
    if (this->load_dimensions != other.load_dimensions) return false;
    if (this->max_weight != other.max_weight) return false;
    if (this->min_load_handling_height != other.min_load_handling_height)
      return false;
    if (this->max_load_handling_height != other.max_load_handling_height)
      return false;
    if (this->min_load_handling_depth != other.min_load_handling_depth)
      return false;
    if (this->max_load_handling_depth != other.max_load_handling_depth)
      return false;
    if (this->min_load_handling_tilt != other.min_load_handling_tilt)
      return false;
    if (this->max_load_handling_tilt != other.max_load_handling_tilt)
      return false;
    if (this->agv_speed_limit != other.agv_speed_limit) return false;
    if (this->agv_acceleration_limit != other.agv_acceleration_limit)
      return false;
    if (this->agv_deceleration_limit != other.agv_deceleration_limit)
      return false;
    if (this->pick_time != other.pick_time) return false;
    if (this->drop_time != other.drop_time) return false;
    if (this->description != other.description) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const LoadSet& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__LOAD_SET_HPP_
