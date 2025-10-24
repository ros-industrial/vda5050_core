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

#ifndef VDA5050_TYPES__SAFETY_STATE_HPP_
#define VDA5050_TYPES__SAFETY_STATE_HPP_

#include "vda5050_types/e_stop.hpp"

namespace vda5050_types {

/// @struct SafetyState
/// @brief  Contains all safety-related information
struct SafetyState
{
  /// @brief Acknowledge-Type of eStop:
  ///        AUTOACK: auto-acknowledgeable e-stop is activated, e.g., by bumper or protective field.
  ///        MANUAL: e-stop hast to be acknowledged manually at the vehicle.
  ///        REMOTE: facility e-stop has to be acknowledged remotely.
  ///        NONE: no e-stop activated.
  EStop e_stop = EStop::NONE;

  /// @brief Protective field violation. True: field is violated. False: field is not violated.
  bool field_violation = false;
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__SAFETY_STATE_HPP_
