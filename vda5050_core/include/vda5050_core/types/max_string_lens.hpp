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

#ifndef VDA5050_CORE__TYPES__MAX_STRING_LENS_HPP_
#define VDA5050_CORE__TYPES__MAX_STRING_LENS_HPP_

#include <cstdint>
#include <optional>

namespace vda5050_core {

namespace types {

/// \brief This message defines the maximum lengths of strings used in
/// MQTT communication and other identifiers within the VDA5050 specification.
struct MaxStringLens
{
  /// \brief Maximum MQTT message length.
  std::optional<uint32_t> msg_len;

  /// \brief Maximum length of serial number part in MQTT topics.
  /// Affected parameters:
  /// - order.serialNumber
  /// - instantActions.serialNumber
  /// - state.serialNumber
  /// - visualization.serialNumber
  /// - connection.serialNumber
  std::optional<uint32_t> topic_serial_len;

  /// \brief Maximum length of all other parts in MQTT topics.
  /// Affected parameters:
  /// - order.timestamp
  /// - order.version
  /// - order.manufacturer
  /// - instantActions.timestamp
  /// - instantActions.version
  /// - instantActions.manufacturer
  /// - state.timestamp
  /// - state.version
  /// - state.manufacturer
  /// - visualization.timestamp
  /// - visualization.version
  /// - visualization.manufacturer
  /// - connection.timestamp
  /// - connection.version
  /// - connection.manufacturer
  std::optional<uint32_t> topic_elem_len;

  /// \brief Maximum length of ID strings.
  /// Affected parameters:
  /// - order.orderId
  /// - order.zoneSetId
  /// - node.nodeId
  /// - nodePosition.mapId
  /// - action.actionId
  /// - edge.edgeId
  /// - edge.startNodeId
  /// - edge.endNodeId
  std::optional<uint32_t> id_len;

  /// \brief Maximum length of enum and key strings.
  /// Affected parameters:
  /// - action.actionType
  /// - action.blockingType
  /// - edge.direction
  /// - actionParameter.key
  /// - state.operatingMode
  /// - load.loadPosition
  /// - load.loadType
  /// - actionState.actionStatus
  /// - error.errorType
  /// - error.errorLevel
  /// - errorReference.referenceKey
  /// - info.infoType
  /// - info.infoLevel
  /// - safetyState.eStop
  /// - connection.connectionState
  std::optional<uint32_t> enum_len;

  /// \brief Maximum length of loadId strings.
  std::optional<uint32_t> load_id_len;

  /// \brief If true, ID strings must contain numerical values only.
  std::optional<bool> id_numerical_only;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const MaxStringLens& other) const
  {
    if (this->msg_len != other.msg_len) return false;
    if (this->topic_serial_len != other.topic_serial_len) return false;
    if (this->topic_elem_len != other.topic_elem_len) return false;
    if (this->id_len != other.id_len) return false;
    if (this->enum_len != other.enum_len) return false;
    if (this->load_id_len != other.load_id_len) return false;
    if (this->id_numerical_only != other.id_numerical_only) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const MaxStringLens& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace types
}  // namespace vda5050_core

#endif  // VDA5050_CORE__TYPES__MAX_STRING_LENS_HPP_
