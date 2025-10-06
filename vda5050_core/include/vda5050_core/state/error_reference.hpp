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

#ifndef VDA5050_CORE__STATE__ERROR_REFERENCE_HPP_
#define VDA5050_CORE__STATE__ERROR_REFERENCE_HPP_

#include <nlohmann/json.hpp>
#include <string>

namespace vda5050_core {

namespace msg {

/// @struct ErrorReference
/// @brief  Provide more information related to
///         the error.
struct ErrorReference
{
  /// @brief Specifies the type of reference used
  ///        (e.g., "nodeId", "edgeId", "orderId",
  ///        "actionId", etc.).
  std::string reference_key;

  /// @brief The value that belongs to the
  ///        reference key. For example, the ID of
  ///        the node where the error occurred.
  std::string reference_value;
};

using json = nlohmann::json;

inline void to_json(json& j, const ErrorReference& ref)
{
  j = json{
    {"reference_key", ref.reference_key},
    {"reference_value", ref.reference_value}};
}

inline void from_json(const json& j, ErrorReference& ref)
{
  j.at("reference_key").get_to(ref.reference_key);
  j.at("reference_value").get_to(ref.reference_value);
}

}  // namespace msg
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__ERROR_REFERENCE_HPP_