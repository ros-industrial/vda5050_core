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

#ifndef VDA5050_CORE__TYPES__ERROR_REFERENCE_HPP_
#define VDA5050_CORE__TYPES__ERROR_REFERENCE_HPP_

#include <nlohmann/json.hpp>
#include <string>

namespace vda5050_core {

namespace types {

/// \struct ErrorReference
/// \brief  Provide more information related to
///         the error.
struct ErrorReference
{
  /// \brief Specifies the type of reference used
  ///        (e.g., "nodeId", "edgeId", "orderId",
  ///        "actionId", etc.).
  std::string reference_key;

  /// \brief The value that belongs to the
  ///        reference key. For example, the ID of
  ///        the node where the error occurred.
  std::string reference_value;

  /// \brief Compares two ErrorReference objects for equality.
  /// \param other The ErrorReference instance to compare with.
  /// \return True if reference_key and reference_value are equal, otherwise false.
  bool operator==(const ErrorReference& other) const
  {
    if (reference_key != other.reference_key) return false;
    if (reference_value != other.reference_value) return false;
    return true;
  }

  /// \brief Compares two ErrorReference objects for inequality.
  /// \param other The ErrorReference instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const ErrorReference& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const ErrorReference& ref)
{
  j = json{
    {"referenceKey", ref.reference_key},
    {"referenceValue", ref.reference_value}};
}

inline void from_json(const json& j, ErrorReference& ref)
{
  j.at("referenceKey").get_to(ref.reference_key);
  j.at("referenceValue").get_to(ref.reference_value);
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ERROR_REFERENCE_HPP_
