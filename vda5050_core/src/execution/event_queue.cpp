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

#include <algorithm>

#include "vda5050_core/execution/event_queue.hpp"

namespace vda5050_core {

namespace execution {

//=============================================================================
void EventQueue::push(std::shared_ptr<EventBase> event, Priority priority)
{
  if (!event) return;

  std::lock_guard<std::mutex> lock(mutex_);

  if (priority == Priority::CRITICAL)
  {
    critical_queue_.push(std::move(event));
  }
  else
  {
    normal_queue_.push(std::move(event));
  }
}

//=============================================================================
std::shared_ptr<EventBase> EventQueue::pop()
{
  std::lock_guard<std::mutex> lock(mutex_);

  if (!critical_queue_.empty()) return pop_internal(critical_queue_);
  if (!normal_queue_.empty()) return pop_internal(normal_queue_);

  return nullptr;
}

//=============================================================================
std::shared_ptr<EventBase> EventQueue::pop_critical_only()
{
  std::lock_guard<std::mutex> lock(mutex_);

  if (!critical_queue_.empty()) return pop_internal(critical_queue_);

  return nullptr;
}

//=============================================================================
bool EventQueue::empty() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return critical_queue_.empty() && normal_queue_.empty();
}

//=============================================================================
void EventQueue::clear_normal()
{
  std::lock_guard<std::mutex> lock(mutex_);
  std::queue<std::shared_ptr<EventBase>> empty_queue;
  std::swap(normal_queue_, empty_queue);
}

//=============================================================================
std::shared_ptr<EventBase> EventQueue::pop_internal(
  std::queue<std::shared_ptr<EventBase>>& queue)
{
  auto event = std::move(queue.front());
  queue.pop();
  return event;
}

}  // namespace execution
}  // namespace vda5050_core
