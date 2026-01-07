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

#include <memory>
#include <typeindex>

#include <vda5050_types/agv_position.hpp>
#include <vda5050_types/battery_state.hpp>
#include <vda5050_types/operating_mode.hpp>

#ifndef VDA5050_EXECUTION__PROVIDER_HPP_
#define VDA5050_EXECUTION__PROVIDER_HPP_

namespace vda5050_execution {

struct ProviderBase
{
  virtual ~ProviderBase() = default;
  virtual std::type_index get_type() const = 0;
};

struct PositionData : public ProviderBase
{
  std::shared_ptr<vda5050_types::AGVPosition> agv_position;

  std::type_index get_type() const override
  {
    return std::type_index(typeid(PositionData));
  }
};

struct BatteryData : public ProviderBase
{
  std::shared_ptr<vda5050_types::BatteryState> battery_state;

  std::type_index get_type() const override
  {
    return std::type_index(typeid(BatteryData));
  }
};

struct OperatingModeData : public ProviderBase
{
  std::shared_ptr<vda5050_types::OperatingMode> operating_mode;

  std::type_index get_type() const override
  {
    return std::type_index(typeid(OperatingModeData));
  }
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__STATE_PROVIDER_HPP_
