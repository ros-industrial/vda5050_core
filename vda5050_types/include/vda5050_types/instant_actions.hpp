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

#ifndef VDA5050_TYPES__INSTANT_ACTIONS_HPP_
#define VDA5050_TYPES__INSTANT_ACTIONS_HPP_

#include <vector>
#include "vda5050_types/action.hpp"
#include "vda5050_types/header.hpp"

namespace vda5050_types {

/// \brief A message containing an instantActions information
/// Published by Master Control on the topic /instantActions to inform an AGV
/// of a set of actions to be executed as soon as they arrive
struct InstantActions
{
  /// \brief Message header
  Header header;

  /// \brief Array of actions that need to be performed immediately and
  /// are not part of the regular order
  std::vector<Action> actions;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const InstantActions& other) const
  {
    if (this->header != other.header) return false;
    if (this->actions != other.actions) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const InstantActions& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__INSTANT_ACTIONS_HPP_
