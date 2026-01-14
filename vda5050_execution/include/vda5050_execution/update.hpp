/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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

#ifndef VDA5050_EXECUTION__UPDATE_HPP_
#define VDA5050_EXECUTION__UPDATE_HPP_

#include <memory>
#include <typeindex>
#include <utility>

#include <vda5050_types/agv_position.hpp>
#include <vda5050_types/battery_state.hpp>
#include <vda5050_types/operating_mode.hpp>

namespace vda5050_execution {

struct UpdateBase
{
  virtual ~UpdateBase() = default;

  virtual std::type_index get_type() const = 0;
};

struct SequenceAcknowledgement : public UpdateBase
{
  uint32_t sequence_id;

  explicit SequenceAcknowledgement(uint32_t sequence_id)
  : sequence_id(sequence_id)
  {
    // Nothing to do here ...
  }
};

struct PositionData : public UpdateBase
{
  std::shared_ptr<vda5050_types::AGVPosition> agv_position;

  explicit PositionData(std::shared_ptr<vda5050_types::AGVPosition> position)
  : agv_position(std::move(position))
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(PositionData));
  }
};

struct BatteryData : public UpdateBase
{
  std::shared_ptr<vda5050_types::BatteryState> battery_state;

  explicit BatteryData(std::shared_ptr<vda5050_types::BatteryState> battery)
  : battery_state(battery)
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(BatteryData));
  }
};

struct OperatingModeData : public UpdateBase
{
  vda5050_types::OperatingMode operating_mode;

  explicit OperatingModeData(vda5050_types::OperatingMode mode)
  : operating_mode(mode)
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(OperatingModeData));
  }
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__UPDATE_HPP_
