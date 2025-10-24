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

#ifndef VDA5050_TYPES__ERROR_HPP_
#define VDA5050_TYPES__ERROR_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_types/error_level.hpp"
#include "vda5050_types/error_reference.hpp"

namespace vda5050_types {

/// @struct Error
/// @brief  Array of error objects.
///         All active errors of the AGV should be
///         in the array.
///         An empty array indicates that the
///         AGV has no active errors.
struct Error
{
  /// @brief Type/name of error
  std::string error_type;

  /// @brief Array of references (e.g., nodeId,
  /// edgeId, orderId, actionId, etc.) to
  /// provide more information related to
  /// the error.
  std::optional<std::vector<ErrorReference>> error_references;

  /// @brief Verbose description providing details
  ///        and possible causes of the error.
  std::optional<std::string> error_description;

  /// TODO: (johnaa) should this be optional?
  /// @brief Hint on how to approach or solve the
  ///        reported error.

  std::optional<std::string> error_hint;

  /// @brief Enum {warning, fatal}
  /// warning: AGV is ready to start (e.g. maintenance
  ///          cycle expiration warning)
  /// fatal: AGV is not in running condition, user
  ///        intervention required (e.g. laser scanner
  ///        is contaminated)
  ErrorLevel error_level = ErrorLevel::WARNING;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__ERROR_HPP_
