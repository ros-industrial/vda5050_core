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

#ifndef VDA5050_CORE__TYPES__LOAD_DIMENSIONS_HPP_
#define VDA5050_CORE__TYPES__LOAD_DIMENSIONS_HPP_

#include <nlohmann/json.hpp>
#include <optional>

namespace vda5050_core {

namespace types {

/// \struct LoadDimensions
/// \brief  Dimensions of the loads bounding box in meters.
struct LoadDimensions
{
  /// \brief Absolute length of the loads bounding box in meter.
  double length = 0.0;

  /// \brief Absolute width of the loads bounding box in meter.
  double width = 0.0;

  /// \brief Absolute height of the loads bounding box in meter.
  ///        Optional: Set value only if known.
  std::optional<double> height;
};

using json = nlohmann::json;

inline void to_json(json& j, const LoadDimensions& dim)
{
  j = json{{"length", dim.length}, {"width", dim.width}};

  if (dim.height.has_value())
  {
    j["height"] = dim.height.value();
  }
}

inline void from_json(const json& j, LoadDimensions& dim)
{
  j.at("length").get_to(dim.length);
  j.at("width").get_to(dim.width);

  if (j.contains("height") && !j.at("height").is_null())
  {
    dim.height = j.at("height").get<double>();
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__LOAD_DIMENSIONS_HPP_
