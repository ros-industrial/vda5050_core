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

#ifndef VDA5050_CORE__TYPES__VALUE_DATA_TYPE_HPP_
#define VDA5050_CORE__TYPES__VALUE_DATA_TYPE_HPP_

namespace vda5050_core {

namespace types {

/// \brief Enum values for ActionParameterFactsheet
enum class ValueDataType
{
  /// \brief Boolean data type
  BOOL,

  /// \brief Number data type
  NUMBER,

  /// \brief Integer data type
  INTEGER,

  /// \brief Float data type
  FLOAT,

  /// \brief String data type
  STRING,

  /// \brief Object data type
  OBJECT,

  /// \brief Array data type
  ARRAY
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__VALUE_DATA_TYPE_HPP_
