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

#ifndef VDA5050_EXECUTION__EXECUTION_ENGINE_HPP_
#define VDA5050_EXECUTION__EXECUTION_ENGINE_HPP_

#include <functional>
#include <memory>
#include <utility>

#include "vda5050_execution/callback_registry.hpp"
#include "vda5050_execution/event_queue.hpp"

namespace vda5050_execution {

class ExecutionEngine : public std::enable_shared_from_this<ExecutionEngine>
{
public:
  template <typename EventT, typename... Args>
  void emit(Args&&... args)
  {
    event_queue_.push<EventT>(std::forward<Args>(args)...);
  }

  template <typename EventT>
  void on(std::function<void(std::shared_ptr<EventT>)> callback)
  {
    callback_registry_.register_callback<EventT>(std::move(callback));
  }

  void step();

private:
  EventQueue event_queue_;
  CallbackRegistry callback_registry_;
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__EXECUTION_ENGINE_HPP_
