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

#ifndef VDA5050_CORE__TYPES__ACTION_HPP_
#define VDA5050_CORE__TYPES__ACTION_HPP_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/action_parameter.hpp"
#include "vda5050_core/types/blocking_type.hpp"

namespace vda5050_core {

namespace types {

/// @struct Node
/// @brief  An AGV action
struct Action
{
  /// \brief Name of action as described in the first column
  ///        of “Actions and Parameters”. Identifies the
  ///        function of the action.
  std::string action_type;

  /// \brief Unique ID to identify the action and map them to
  ///        the actionState in the state.Suggestion : Use UUIDs.
  std::string action_id;

  /// \brief Additional information on the action
  std::optional<std::string> action_description;

  /// \brief NONE, SOFT, HARD
  BlockingType blocking_type = BlockingType::NONE;

  /// \brief Array of actionParameter objects for the indicated
  ///        action e.g.deviceId, loadId, external Triggers.
  ///        See “Actions and Parameters”.
  std::optional<std::vector<ActionParameter>> action_parameters;

  /// \brief Compares two Action objects for equality.
  /// \param other The Action instance to compare with.
  /// \return True if all fields are equal; otherwise false.
  inline bool operator==(const Action& other) const noexcept(true)
  {
    if (action_type != other.action_type) return false;
    if (action_id != other.action_id) return false;
    if (blocking_type != other.blocking_type) return false;
    if (action_parameters != other.action_parameters) return false;
    return true;
  }

  /// \brief Compares two Action objects for inequality.
  /// \param other The Action instance to compare with.
  /// \return True if any field differs, otherwise false.
  inline bool operator!=(const Action& other) const noexcept(true)
  {
    return !this->operator==(other);
  }
};

using json = nlohmann::json;

inline void to_json(json& j, const Action& d)
{
  if (d.action_description.has_value())
  {
    j["actionDescription"] = *d.action_description;
  }
  j["actionId"] = d.action_id;
  if (d.action_parameters.has_value())
  {
    j["actionParameters"] = *d.action_parameters;
  }
  j["actionType"] = d.action_type;
  j["blockingType"] = d.blocking_type;
}

inline void from_json(const json& j, Action& d)
{
  if (j.contains("actionDescription"))
  {
    d.action_description = j.at("actionDescription");
  }
  d.action_id = j.at("actionId");
  if (j.contains("actionParameters"))
  {
    d.action_parameters =
      j.at("actionParameters").get<std::vector<ActionParameter>>();
  }
  d.action_type = j.at("actionType");
  d.blocking_type = j.at("blockingType");
}

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__ACTION_HPP_
