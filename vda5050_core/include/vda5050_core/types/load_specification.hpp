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

#ifndef VDA5050_CORE__TYPES__LOAD_SPECIFICATION_HPP_
#define VDA5050_CORE__TYPES__LOAD_SPECIFICATION_HPP_

#include <optional>
#include <string>
#include <vector>

#include "vda5050_core/types/load_set.hpp"

namespace vda5050_core {

namespace types {

/// \brief Message specifying the load handling and supported load types
/// of the AGV
struct LoadSpecification
{
  /// \brief Array of load positions / load handling devices.
  /// This array contains the valid values for "state.loads[].loadPosition"
  /// and for the action parameter "lhd" of pick-and-drop actions.
  /// If this array doesn't exist or is empty, the AGV has no load handling
  /// device.
  std::optional<std::vector<std::string>> load_positions;

  /// \brief Array of load sets that can be handled by the AGV.
  std::optional<std::vector<LoadSet>> load_sets;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const LoadSpecification& other) const
  {
    if (this->load_positions != other.load_positions) return false;
    if (this->load_sets != other.load_sets) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const LoadSpecification& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__LOAD_SPECIFICATION_HPP_
