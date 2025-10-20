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

#ifndef VDA5050_CORE__TYPES__ERROR_HPP_
#define VDA5050_CORE__TYPES__ERROR_HPP_

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "vda5050_core/types/error_level.hpp"
#include "vda5050_core/types/error_reference.hpp"

namespace vda5050_core {

namespace types {

/// \struct Error
/// \brief  Array of error objects.
///         All active errors of the AGV should be
///         in the array.
///         An empty array indicates that the
///         AGV has no active errors.
struct Error
{
  /// \brief Type/name of error
  std::string error_type;

  /// \brief Array of references (e.g., nodeId,
  /// edgeId, orderId, actionId, etc.) to
  /// provide more information related to
  /// the error.
  std::optional<std::vector<ErrorReference>> error_references;

  /// \brief Verbose description providing details
  ///        and possible causes of the error.
  std::optional<std::string> error_description;

  /// TODO: (johnaa) should this be optional?
  /// \brief Hint on how to approach or solve the
  ///        reported error.
  std::optional<std::string> error_hint;

  /// \brief Enum {warning, fatal}
  /// warning: AGV is ready to start (e.g. maintenance
  ///          cycle expiration warning)
  /// fatal: AGV is not in running condition, user
  ///        intervention required (e.g. laser scanner
  ///        is contaminated)
  ErrorLevel error_level = ErrorLevel::WARNING;

  /// \brief Compares two Error objects for equality.
  /// \param other The Error instance to compare with.
  /// \return True if errorType, errorReferences, errorDescription, and errorLevel are equal, otherwise false.
  bool operator==(const Error& other) const
  {
    if (error_type != other.error_type) return false;
    if (error_references != other.error_references) return false;
    if (error_description != other.error_description) return false;
    if (error_level != other.error_level) return false;
    return true;
  }

  /// \brief Compares two Error objects for inequality.
  /// \param other The Error instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const Error& other) const
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Error& e)
{
  j = json{{"error_type", e.error_type}, {"error_level", e.error_level}};

  if (e.error_references.has_value())
  {
    j["error_references"] = e.error_references.value();
  }

  if (e.error_description.has_value())
  {
    j["error_description"] = e.error_description.value();
  }

  if (e.error_hint.has_value())
  {
    j["error_hint"] = e.error_hint.value();
  }
}

inline void from_json(const json& j, Error& e)
{
  j.at("error_type").get_to(e.error_type);
  j.at("error_level").get_to(e.error_level);

  if (j.contains("error_references") && !j.at("error_references").is_null())
  {
    e.error_references =
      j.at("error_references").get<std::vector<ErrorReference>>();
  }
  else
  {
    e.error_references = std::nullopt;
  }

  if (j.contains("error_description") && !j.at("error_description").is_null())
  {
    e.error_description = j.at("error_description").get<std::string>();
  }
  else
  {
    e.error_description = std::nullopt;
  }

  if (j.contains("error_hint") && !j.at("error_hint").is_null())
  {
    e.error_hint = j.at("error_hint").get<std::string>();
  }
  else
  {
    e.error_hint = std::nullopt;
  }
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ERROR_HPP_
