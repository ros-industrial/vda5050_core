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

#ifndef VDA5050_TYPES__VERSION_INFO_HPP_
#define VDA5050_TYPES__VERSION_INFO_HPP_

#include <string>

namespace vda5050_types {

/// \brief Key-value pair containing software/hardware version information.
struct VersionInfo
{
  /// \brief Key of the software/hardware version used (e.g., softwareVersion).
  std::string key;

  /// \brief The version corresponding to the key (e.g., v1.12.4-beta).
  std::string value;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const VersionInfo& other) const
  {
    if (this->key != other.key) return false;
    if (this->value != other.value) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const VersionInfo& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__VERSION_INFO_HPP_
