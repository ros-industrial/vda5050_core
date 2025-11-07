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

#ifndef VDA5050_TYPES__ENVELOPE3D_HPP_
#define VDA5050_TYPES__ENVELOPE3D_HPP_

#include <optional>
#include <string>

namespace vda5050_types {

/// \brief Struct for an envelope curve in 3D.
struct Envelope3d
{
  /// \brief Name of the envelope curve set.
  std::string set;

  /// \brief Format of the data, eg: DXF.
  std::string format;

  /// \brief 3D-envelope curve data as a string. Format of this data is specified in the 'format' field, and should be handled accordingly.
  std::optional<std::string> data;

  /// \brief Protocol and URL definition for downloading the 3D-envelope curve data. eg: ftp://xxx.yyy.com/ac4dgvhoif5tghji
  std::optional<std::string> url;

  /// \brief Free-form text: description of envelope curve set.
  std::optional<std::string> description;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Envelope3d& other) const
  {
    if (this->set != other.set) return false;
    if (this->format != other.format) return false;
    if (this->data != other.data) return false;
    if (this->url != other.url) return false;
    if (this->description != other.description) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Envelope3d& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__ENVELOPE3D_HPP_
