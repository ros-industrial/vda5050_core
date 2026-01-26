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

#ifndef VDA5050_EXECUTION__EVENT_QUEUE_HPP_
#define VDA5050_EXECUTION__EVENT_QUEUE_HPP_

#include <memory>
#include <mutex>
#include <queue>
#include <utility>

#include "vda5050_execution/event.hpp"

namespace vda5050_execution {

class EventQueue
{
public:
  template <typename EventT, typename... Args>
  void push(Args&&... args)
  {
    static_assert(
      std::is_base_of_v<EventBase, EventT>,
      "Event must be derived from EventBase");

    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::make_unique<EventT>(std::forward<Args>(args)...));
  }

  bool empty() const;

  std::shared_ptr<EventBase> pop();

private:
  std::queue<std::unique_ptr<EventBase>> queue_;
  mutable std::mutex mutex_;
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__EVENT_QUEUE_HPP_
