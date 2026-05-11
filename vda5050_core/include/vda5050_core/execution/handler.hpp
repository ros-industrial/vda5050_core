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

#ifndef VDA5050_CORE__EXECUTION__HANDLER_HPP_
#define VDA5050_CORE__EXECUTION__HANDLER_HPP_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <vector>

#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/execution/strategy_interface.hpp"

namespace vda5050_core {

namespace execution {

class Handler : public std::enable_shared_from_this<Handler>
{
public:
  static std::shared_ptr<Handler> make(
    std::shared_ptr<ContextInterface> context,
    const std::vector<std::shared_ptr<StrategyInterface>>& strategies = {});

  void add_strategy(std::shared_ptr<StrategyInterface> strategy);

  void remove_strategy(std::shared_ptr<StrategyInterface> strategy);

  template <typename StrategyT>
  void remove_strategy_by_type()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    strategies_.erase(
      std::remove_if(
        strategies_.begin(), strategies_.end(),
        [](const auto& s) {
          return std::dynamic_pointer_cast<StrategyT>(s) != nullptr;
        }),
      strategies_.end());
  }

  void wake();

  void spin(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

  void spin_once();

  std::vector<std::shared_ptr<StrategyInterface>> get_active_strategies();

  bool running();

  void stop();

private:
  Handler(
    std::shared_ptr<ContextInterface> context,
    const std::vector<std::shared_ptr<StrategyInterface>>& strategies);

  void step_active_strategies();

  std::shared_ptr<ContextInterface> context_;

  std::vector<std::shared_ptr<StrategyInterface>> strategies_;

  std::atomic_bool running_;
  std::atomic_bool spinning_;

  std::condition_variable cv_;
  std::mutex mutex_;
};

}  // namespace execution
}  // namespace vda5050_core

#endif  // VDA5050_CORE__EXECUTION__HANDLER_HPP_
