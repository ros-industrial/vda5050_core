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

#ifndef VDA5050_CORE__EXECUTION__EVENT_QUEUE_HPP_
#define VDA5050_CORE__EXECUTION__EVENT_QUEUE_HPP_

#include <memory>
#include <mutex>
#include <queue>

#include "vda5050_core/execution/base.hpp"

namespace vda5050_core {

namespace execution {

enum class Priority
{
  NORMAL,
  CRITICAL
};

class EventQueue
{
public:
  void push(
    std::shared_ptr<EventBase> event, Priority priority = Priority::NORMAL);

  std::shared_ptr<EventBase> pop();

  std::shared_ptr<EventBase> pop_critical_only();

  bool empty() const;

  void clear_normal();

private:
  std::shared_ptr<EventBase> pop_internal(
    std::queue<std::shared_ptr<EventBase>>& queue);

  std::queue<std::shared_ptr<EventBase>> normal_queue_;
  std::queue<std::shared_ptr<EventBase>> critical_queue_;

  mutable std::mutex mutex_;
};

}  // namespace execution
}  // namespace vda5050_core

#endif  // VDA5050_CORE__EXECUTION__EVENT_QUEUE_HPP_
