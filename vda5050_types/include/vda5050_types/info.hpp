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

#ifndef VDA5050_TYPES__INFO_HPP_
#define VDA5050_TYPES__INFO_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_types/info_level.hpp"
#include "vda5050_types/info_reference.hpp"

namespace vda5050_types {

/// \brief Messages carrying information for visualization or debugging
/// Part of the state message
struct Info
{
  /// \brief Type/name of information
  std::string info_type;

  /// \brief Array of references to identify the source of the information
  std::optional<std::vector<InfoReference>> info_references;

  /// \brief Verbose description of the information
  std::optional<std::string> info_description;

  /// \brief Type of the information
  InfoLevel info_level = InfoLevel::DEBUG;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  bool operator==(const Info& other) const
  {
    if (info_type != other.info_type) return false;
    if (info_references != other.info_references) return false;
    if (info_description != other.info_description) return false;
    if (info_level != other.info_level) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Info& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__INFO_HPP_
