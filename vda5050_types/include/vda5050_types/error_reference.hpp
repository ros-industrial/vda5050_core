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

#ifndef VDA5050_TYPES__ERROR_REFERENCE_HPP_
#define VDA5050_TYPES__ERROR_REFERENCE_HPP_

#include <string>

namespace vda5050_types {

/// \brief Describes a variable and its value related to the error
/// Part of the state message
struct ErrorReference
{
  /// \brief Specifies the type of reference used
  /// (e.g., nodeId, edgeId, orderId, actionId, etc.)
  std::string reference_key;

  /// \brief The value that belongs to the reference key. For
  /// example, the ID of the node where the error occurred
  std::string reference_value;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  bool operator==(const ErrorReference& other) const
  {
    if (reference_key != other.reference_key) return false;
    if (reference_value != other.reference_value) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other the other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const ErrorReference& other) const
  {
    return !this->operator==(other);
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__ERROR_REFERENCE_HPP_
