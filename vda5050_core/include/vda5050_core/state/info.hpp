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

#ifndef VDA5050_CORE__STATE__INFO_HPP_
#define VDA5050_CORE__STATE__INFO_HPP_

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "vda5050_core/state/info_level.hpp"
#include "vda5050_core/state/info_reference.hpp"

namespace vda5050_core {

namespace msg {

/// @struct Info
/// @brief Information for visualization or debugging
struct Info
{
  /// @brief Type/name of information
  std::string info_type;

  /// @brief Array of references.
  std::optional<std::vector<InfoReference>> info_references;

  /// @brief Description of the information.
  std::optional<std::string> info_description;

  /// @brief  Enum {'DEBUG', 'INFO'}
  ///        'DEBUG': used for debugging.
  ///        'INFO': used for visualization.
  InfoLevel info_level = InfoLevel::DEBUG;
};

using json = nlohmann::json;

inline void to_json(json& j, const Info& i)
{
  j = json{{"info_type", i.info_type}, {"info_level", i.info_level}};

  if (i.info_references.has_value())
  {
    j["info_references"] = i.info_references.value();
  }

  if (i.info_description.has_value())
  {
    j["info_description"] = i.info_description.value();
  }
}

inline void from_json(const json& j, Info& i)
{
  j.at("info_type").get_to(i.info_type);
  j.at("info_level").get_to(i.info_level);

  if (j.contains("info_references") && !j.at("info_references").is_null())
  {
    i.info_references =
      j.at("info_references").get<std::vector<InfoReference>>();
  }
  else
  {
    i.info_references = std::nullopt;
  }

  if (j.contains("info_description") && !j.at("info_description").is_null())
  {
    i.info_description = j.at("info_description").get<std::string>();
  }
}

}  // namespace msg
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__INFO_HPP_