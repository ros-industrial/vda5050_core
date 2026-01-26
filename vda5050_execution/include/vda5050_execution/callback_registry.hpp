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

#ifndef VDA5050_EXECUTION__CALLBACK_REGISTRY_HPP_
#define VDA5050_EXECUTION__CALLBACK_REGISTRY_HPP_

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "vda5050_execution/event.hpp"

namespace vda5050_execution {

class CallbackRegistry
{
public:
  template <typename EventT>
  void register_callback(std::function<void(std::shared_ptr<EventT>)> callback)
  {
    static_assert(
      std::is_base_of_v<EventBase, EventT>,
      "Event must be derived from EventBase");

    auto wrapper = [cb =
                      std::move(callback)](std::shared_ptr<EventBase> event) {
      cb(std::static_pointer_cast<EventT>(event));
    };

    callbacks_[std::type_index(typeid(EventT))].push_back(std::move(wrapper));
  }

  void dispatch(std::shared_ptr<EventBase> event) const;

private:
  using ErasedCallback = std::function<void(std::shared_ptr<EventBase>)>;
  std::unordered_map<std::type_index, std::vector<ErasedCallback>> callbacks_;
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__CALLBACK_REGISTRY_HPP_
