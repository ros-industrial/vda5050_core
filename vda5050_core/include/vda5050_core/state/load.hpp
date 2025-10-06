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

#ifndef VDA5050_CORE__STATE__LOAD_HPP_
#define VDA5050_CORE__STATE__LOAD_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "vda5050_core/state/bounding_box_reference.hpp"
#include "vda5050_core/state/load_dimensions.hpp"

namespace vda5050_core {

namespace msg {

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

using json = nlohmann::json;

inline void to_json(json& j, const Load& l)
{
  j = json{};

  if (l.load_id.has_value())
  {
    j["load_id"] = l.load_id.value();
  }

  if (l.load_type.has_value())
  {
    j["load_type"] = l.load_type.value();
  }

  if (l.load_position.has_value())
  {
    j["load_position"] = l.load_position.value();
  }

  if (l.bounding_box_reference.has_value())
  {
    j["bounding_box_reference"] = l.bounding_box_reference.value();
  }

  if (l.load_dimensions.has_value())
  {
    j["load_dimensions"] = l.load_dimensions.value();
  }

  if (l.weight.has_value())
  {
    j["weight"] = l.weight.value();
  }
}

inline void from_json(const json& j, Load& l)
{
  if (j.contains("load_id") && !j.at("load_id").is_null())
  {
    l.load_id = j.at("load_id").get<std::string>();
  }

  if (j.contains("load_type") && !j.at("load_type").is_null())
  {
    l.load_type = j.at("load_type").get<std::string>();
  }

  if (j.contains("load_position") && !j.at("load_position").is_null())
  {
    l.load_position = j.at("load_position").get<std::string>();
  }

  if (
    j.contains("bounding_box_reference") &&
    !j.at("bounding_box_reference").is_null())
  {
    l.bounding_box_reference =
      j.at("bounding_box_reference").get<BoundingBoxReference>();
  }

  if (j.contains("load_dimensions") && !j.at("load_dimensions").is_null())
  {
    l.load_dimensions = j.at("load_dimensions").get<LoadDimensions>();
  }

  if (j.contains("weight") && !j.at("weight").is_null())
  {
    l.weight = j.at("weight").get<uint32_t>();
  }
}

}  // namespace msg
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__LOAD_HPP_