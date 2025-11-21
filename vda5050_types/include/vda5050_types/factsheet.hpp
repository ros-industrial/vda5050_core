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

#ifndef VDA5050_TYPES__FACTSHEET_HPP_
#define VDA5050_TYPES__FACTSHEET_HPP_

#include "vda5050_types/agv_geometry.hpp"
#include "vda5050_types/header.hpp"
#include "vda5050_types/load_specification.hpp"
#include "vda5050_types/physical_parameters.hpp"
#include "vda5050_types/protocol_features.hpp"
#include "vda5050_types/protocol_limits.hpp"
#include "vda5050_types/type_specification.hpp"
#include "vda5050_types/vehicle_config.hpp"

namespace vda5050_types {

/// \brief Contains the AGV's capabilities, limits, features, and configuration
/// as required by the VDA5050 specification.
struct Factsheet
{
  /// \brief Message header that contains the common fields required across all
  /// messages
  Header header;

  /// \brief Parameters that generally specify the class and the capabilities
  /// of the AGV
  TypeSpecification type_specification;

  /// \brief Parameters that specify the basic physical properties of the AGV
  PhysicalParameters physical_parameters;

  /// \brief Limits for length of identifiers, arrays, strings, and similar in
  /// MQTT communication
  ProtocolLimits protocol_limits;

  /// \brief Supported features of VDA5050 protocol
  ProtocolFeatures protocol_features;

  /// \brief Detailed definition of AGV geometry
  AGVGeometry agv_geometry;

  /// \brief Abstract specification of load capabilities
  LoadSpecification load_specification;

  /// \brief Summary of current software and hardware versions on the vehicle
  /// and optional network information
  VehicleConfig vehicle_config;

  /// \brief Equality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is equal?
  inline bool operator==(const Factsheet& other) const
  {
    if (this->header != other.header) return false;
    if (this->type_specification != other.type_specification) return false;
    if (this->physical_parameters != other.physical_parameters) return false;
    if (this->protocol_limits != other.protocol_limits) return false;
    if (this->protocol_features != other.protocol_features) return false;
    if (this->agv_geometry != other.agv_geometry) return false;
    if (this->load_specification != other.load_specification) return false;
    if (this->vehicle_config != other.vehicle_config) return false;
    return true;
  }

  /// \brief Inequality operator
  ///
  /// \param other The other object to compare to
  ///
  /// \return is not equal?
  inline bool operator!=(const Factsheet& other) const
  {
    return !(this->operator==(other));
  }
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__FACTSHEET_HPP_
