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

#ifndef VDA5050_CORE__TYPES__INFO_HPP_
#define VDA5050_CORE__TYPES__INFO_HPP_

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "vda5050_core/types/info_level.hpp"
#include "vda5050_core/types/info_reference.hpp"

namespace vda5050_core {

namespace types {

/// \struct Info
/// \brief Information for visualization or debugging
struct Info
{
  /// \brief Type/name of information
  std::string info_type;

  /// \brief Array of references.
  std::optional<std::vector<InfoReference>> info_references;

  /// \brief Description of the information.
  std::optional<std::string> info_description;

  /// \brief  Enum {'DEBUG', 'INFO'}
  ///        'DEBUG': used for debugging.
  ///        'INFO': used for visualization.
  InfoLevel info_level = InfoLevel::DEBUG;

  /// \brief Compares two Info objects for equality.
  /// \param other The Info instance to compare with.
  /// \return True if infoType, infoReferences, infoDescription, and infoLevel are equal, otherwise false.
  bool operator==(const Info& other) const
  {
    if (info_type != other.info_type) return false;
    if (info_references != other.info_references) return false;
    if (info_description != other.info_description) return false;
    if (info_level != other.info_level) return false;
    return true;
  }

  /// \brief Compares two Info objects for inequality.
  /// \param other The Info instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const Info& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Info& i)
{
  j = json{{"infoType", i.info_type}, {"infoLevel", i.info_level}};

  if (i.info_references.has_value())
  {
    j["infoReferences"] = i.info_references.value();
  }

  if (i.info_description.has_value())
  {
    j["infoDescription"] = i.info_description.value();
  }
}

inline void from_json(const json& j, Info& i)
{
  j.at("infoType").get_to(i.info_type);
  j.at("infoLevel").get_to(i.info_level);

  if (j.contains("infoReferences") && !j.at("infoReferences").is_null())
  {
    i.info_references =
      j.at("infoReferences").get<std::vector<InfoReference>>();
  }
  else
  {
    i.info_references = std::nullopt;
  }

  if (j.contains("infoDescription") && !j.at("infoDescription").is_null())
  {
    i.info_description = j.at("infoDescription").get<std::string>();
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__INFO_HPP_
