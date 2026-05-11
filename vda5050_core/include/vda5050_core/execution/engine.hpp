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

#ifndef VDA5050_CORE__EXECUTION__ENGINE_HPP_
#define VDA5050_CORE__EXECUTION__ENGINE_HPP_

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/execution/event_queue.hpp"

namespace vda5050_core {

namespace execution {

class Engine : public std::enable_shared_from_this<Engine>
{
public:
  template <typename EventT, typename... Args>
  void emit(Priority priority, Args&&... args)
  {
    static_assert(
      std::is_base_of_v<EventBase, EventT>,
      "Event must be derived from EventBase");

    emit_shared(
      std::make_shared<EventT>(std::forward<Args>(args)...), priority);
  }

  void emit_shared(
    std::shared_ptr<EventBase> event, Priority priority = Priority::NORMAL);

  template <typename EventT>
  void on(std::function<void(std::shared_ptr<EventT>)> callback)
  {
    static_assert(
      std::is_base_of_v<EventBase, EventT>,
      "Event must be derived from EventBase");

    auto wrapper = [cb =
                      std::move(callback)](std::shared_ptr<EventBase> event) {
      cb(std::static_pointer_cast<EventT>(event));
    };

    std::lock_guard<std::mutex> lock(registry_mutex_);
    callbacks_[std::type_index(typeid(EventT))].push_back(std::move(wrapper));
  }

  template <typename UpdateT>
  void suspend_for(
    std::chrono::milliseconds timeout,
    std::function<bool(std::shared_ptr<UpdateT>)> predicate = nullptr)
  {
    static_assert(
      std::is_base_of_v<UpdateBase, UpdateT>,
      "Update must be derived from UpdateBase");

    std::lock_guard<std::mutex> lock(wait_mutex_);
    wait_timeout_ = std::chrono::steady_clock::now() + timeout;

    wait_predicate_ = [predicate](std::shared_ptr<UpdateBase> update) -> bool {
      if (update->get_type() == std::type_index(typeid(UpdateT)))
      {
        auto update_t = std::static_pointer_cast<UpdateT>(update);
        if (predicate)
        {
          return predicate(update_t);
        }
      }
      return true;
    };

    waiting_ = true;
  }

  void notify(std::shared_ptr<UpdateBase> update);

  void step();

  bool waiting() const;

private:
  void reset_internal_wait() const;

  void check_timeout() const;

  EventQueue event_queue_;

  using ErasedCallback = std::function<void(std::shared_ptr<EventBase>)>;
  std::unordered_map<std::type_index, std::vector<ErasedCallback>> callbacks_;
  std::mutex registry_mutex_;

  mutable bool waiting_ = false;
  mutable std::chrono::steady_clock::time_point wait_timeout_ =
    std::chrono::steady_clock::time_point::min();
  mutable std::function<bool(std::shared_ptr<UpdateBase>)> wait_predicate_ =
    nullptr;
  mutable std::mutex wait_mutex_;
};

}  // namespace execution
}  // namespace vda5050_core

#endif  // VDA5050_CORE__EXECUTION__ENGINE_HPP_
