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

#ifndef VDA5050_CORE__TYPES__ACTION_PARAMETER_HPP_
#define VDA5050_CORE__TYPES__ACTION_PARAMETER_HPP_

#include <string>

#include <nlohmann/json.hpp>

namespace vda5050_core {

namespace types {


using json = nlohmann::json;

/// @struct ActionParameter
/// @brief  Defines the type and value field of the action parameter
struct ActionParameter
{
  /// \brief Action key
  std::string key;

  /// \brief Value of the action key
  json value;

  /// \brief Compares two ActionParameter objects for equality.
  /// \param other The ActionParameter instance to compare with.
  /// \return True if both key and value are equal; otherwise false.
  inline bool operator==(const ActionParameter& other) const noexcept(true)
  {
    if (key != other.key) return false;
    if (value != other.value) return false;
    return true;
  }

  /// \brief Compares two ActionParameter objects for inequality.
  /// \param other The ActionParameter instance to compare with.
  /// \return True if either key or value differ; otherwise false.
  inline bool operator!=(const ActionParameter& other) const noexcept(true)
  {
    return !this->operator==(other);
  }
};

inline void to_json(json& j, const ActionParameter& d)
{
  j["key"] = d.key;
  j["value"] = d.value;
}

inline void from_json(const json& j, ActionParameter& d)
{
  d.key = j.at("key");
  d.value = j.at("value");
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ACTION_PARAMETER_HPP_
