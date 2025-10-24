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

#ifndef VDA5050_TYPES__LOAD_HPP_
#define VDA5050_TYPES__LOAD_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include "vda5050_types/bounding_box_reference.hpp"
#include "vda5050_types/load_dimensions.hpp"

namespace vda5050_types {

/// @struct Load
/// @brief  Loads that are currently handled by the AGV.
///         Optional: If AGV cannot determine load state,
///         leave the array out of the state.
//          If the AGV can determine the load state, but the array is empty,
///         the AGV is considered unloaded.
struct Load
{
  /// @brief Unique identification number of the load (e.g., barcode or RFID).
  ///        Empty field, if the AGV can identify the load, but did not identify the load yet.
  ///        Optional, if the AGV cannot identify the load.
  std::optional<std::string> load_id;

  /// @brief Type of load.
  std::optional<std::string> load_type;

  /// @brief Indicates which load handling/carrying unit of the AGV is used,
  ///        e.g., in case the AGV has multiple spots/positions to carry loads.
  ///        Optional for vehicles with only one loadPosition.
  std::optional<std::string> load_position;

  /// @brief Point of reference for the location of the bounding box.
  ///        The point of reference is always the center of the bounding box bottom surface (at height = 0)
  ///        and is described in coordinates of the AGV coordinate system.
  std::optional<BoundingBoxReference> bounding_box_reference;

  /// @brief Dimensions of the loads bounding box in meters.
  std::optional<LoadDimensions> load_dimensions;

  /// @brief Absolute weight of the load measured in kg.
  std::optional<uint32_t> weight;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__LOAD_HPP_
