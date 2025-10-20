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

#ifndef VDA5050_CORE__TYPES__INFO_REFERENCE_HPP_
#define VDA5050_CORE__TYPES__INFO_REFERENCE_HPP_

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {

/// \struct InfoReference
/// \brief  Determines the type and value of reference
struct InfoReference
{
  /// \brief References the type of reference (e.g. orderId, headerId, actionId, etc.).
  std::string reference_key;

  /// \brief References the value, which belongs to the key.
  std::string reference_value;

  /// \brief Compares two InfoReference objects for equality.
  /// \param other The InfoReference instance to compare with.
  /// \return True if reference_key and reference_value are equal, otherwise false.
  bool operator==(const InfoReference& other) const
  {
    if (reference_key != other.reference_key) return false;
    if (reference_value != other.reference_value) return false;
    return true;
  }

  /// \brief Compares two InfoReference objects for inequality.
  /// \param other The InfoReference instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const InfoReference& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const InfoReference& ref)
{
  j = json{
    {"reference_key", ref.reference_key},
    {"reference_value", ref.reference_value}};
}

inline void from_json(const json& j, InfoReference& ref)
{
  j.at("reference_key").get_to(ref.reference_key);
  j.at("reference_value").get_to(ref.reference_value);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__INFO_REFERENCE_HPP_
