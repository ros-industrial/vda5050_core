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

#ifndef VDA5050_TYPES__OPERATING_MODE_HPP_
#define VDA5050_TYPES__OPERATING_MODE_HPP_

#include <cstdint>

namespace vda5050_types {

/// \brief Enum values for operatingMode
enum class OperatingMode
{
  /// \brief AGV is under full control of the master control. It drives
  /// and executes actions based on orders from the master control
  AUTOMATIC,

  /// \brief AGV is under control of the master control in areas like
  /// driving and executing actions. The driving speeds is however
  /// controlled by the HMI
  SEMIAUTOMATIC,

  /// \brief AGV is not under control of the master control. HMI can
  /// be used to control the AGV. Only location of the AGV is sent to
  /// the master control
  MANUAL,

  /// \brief AGV is not under the control of the master
  /// control and it doesn't send order or actions to it. Authorized
  /// personnel can reconfigure the AGV
  SERVICE,

  /// \brief AGV is not under the control of the master control. It is
  /// being taught
  TEACHIN
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__OPERATING_MODE_HPP_
