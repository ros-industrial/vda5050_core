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

#ifndef VDA5050_CORE__EXECUTION__STRATEGY_INTERFACE_HPP_
#define VDA5050_CORE__EXECUTION__STRATEGY_INTERFACE_HPP_

#include <memory>

#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/engine.hpp"

namespace vda5050_core {

namespace execution {

class StrategyInterface
{
public:
  StrategyInterface() : engine_(std::make_shared<Engine>())
  {
    // Nothing to do here ...
  }

  virtual ~StrategyInterface() = default;

  virtual void init(std::shared_ptr<ContextInterface> context) = 0;

  virtual void step(std::shared_ptr<ContextInterface> context) = 0;

  std::shared_ptr<Engine> engine()
  {
    return engine_;
  }

private:
  std::shared_ptr<Engine> engine_;
};

}  // namespace execution
}  // namespace vda5050_core

#endif  // VDA5050_CORE__EXECUTION__STRATEGY_INTERFACE_HPP_
