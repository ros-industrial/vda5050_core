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

#ifndef VDA5050_CORE__STATE__ACTION_STATUS_HPP_
#define VDA5050_CORE__STATE__ACTION_STATUS_HPP_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace vda5050_core {

namespace msg {

/// @enum  ActionStatus
/// @brief Represents the execution state of an action on the AGV.
enum class ActionStatus
{
  /// @brief Action was received by AGV but the node where it triggers was not yet
  ///        reached or the edge where it is active was not yet entered.
  WAITING,

  /// @brief Action was triggered, preparatory measures are initiated.
  INITIALIZING,

  /// @brief The action is running.
  RUNNING,

  /// @brief The action is paused because of a pause instantAction or external trigger
  ///        (pause button on AGV)
  PAUSED,

  /// @brief The action is finished. A result is reported via the resultDescription
  FINISHED,

  /// @brief Action could not be finished for whatever reason.
  FAILED
};

using json = nlohmann::json;

inline void to_json(json& j, const ActionStatus& status)
{
  switch (status)
  {
    case ActionStatus::WAITING:
      j = "WAITING";
      return;
    case ActionStatus::INITIALIZING:
      j = "INITIALIZING";
      return;
    case ActionStatus::RUNNING:
      j = "RUNNING";
      return;
    case ActionStatus::PAUSED:
      j = "PAUSED";
      return;
    case ActionStatus::FINISHED:
      j = "FINISHED";
      return;
    case ActionStatus::FAILED:
      j = "FAILED";
      return;
  }
  throw std::invalid_argument("Unknown ActionStatus enum value");
}

inline void from_json(const json& j, ActionStatus& status)
{
  const std::string& s = j.get<std::string>();

  if (s == "WAITING")
  {
    status = ActionStatus::WAITING;
    return;
  }
  if (s == "INITIALIZING")
  {
    status = ActionStatus::INITIALIZING;
    return;
  }
  if (s == "RUNNING")
  {
    status = ActionStatus::RUNNING;
    return;
  }
  if (s == "PAUSED")
  {
    status = ActionStatus::PAUSED;
    return;
  }
  if (s == "FINISHED")
  {
    status = ActionStatus::FINISHED;
    return;
  }
  if (s == "FAILED")
  {
    status = ActionStatus::FAILED;
    return;
  }
  throw std::invalid_argument("Invalid ActionStatus string: " + s);
}

}  // namespace msg
}  // namespace vda5050_core

#endif  // VDA5050_CORE__STATE__ACTION_STATUS_HPP_