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

#include "vda5050_core/execution/engine.hpp"

namespace vda5050_core {

namespace execution {

//=============================================================================
void Engine::emit_shared(std::shared_ptr<EventBase> event, Priority priority)
{
  event_queue_.push(std::move(event), priority);
}

//=============================================================================
void Engine::notify(std::shared_ptr<UpdateBase> update)
{
  std::lock_guard<std::mutex> lock(wait_mutex_);
  if (waiting_ && wait_predicate_)
  {
    if (wait_predicate_(update)) reset_internal_wait();
  }
}

//=============================================================================
void Engine::step()
{
  std::shared_ptr<EventBase> event;
  {
    std::lock_guard<std::mutex> lock(wait_mutex_);
    check_timeout();
    event = waiting_ ? event_queue_.pop_critical_only() : event_queue_.pop();
  }

  if (!event) return;

  std::vector<ErasedCallback> targets;
  {
    std::lock_guard<std::mutex> lock(registry_mutex_);
    auto it = callbacks_.find(event->get_type());
    if (it != callbacks_.end()) targets = it->second;
  }

  for (auto cb : targets)
  {
    cb(event);
  }
}

//=============================================================================
bool Engine::waiting() const
{
  std::lock_guard<std::mutex> lock(wait_mutex_);
  check_timeout();
  return waiting_;
}

//=============================================================================
void Engine::reset_internal_wait() const
{
  waiting_ = false;
  wait_predicate_ = nullptr;
}

//=============================================================================
void Engine::check_timeout() const
{
  if (waiting_ && std::chrono::steady_clock::now() > wait_timeout_)
    reset_internal_wait();
}

}  // namespace execution
}  // namespace vda5050_core
