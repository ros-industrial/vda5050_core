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

#ifndef VDA5050_CORE__EXECUTION__PROVIDER_HPP_
#define VDA5050_CORE__EXECUTION__PROVIDER_HPP_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "vda5050_core/execution/base.hpp"

namespace vda5050_core {

namespace execution {

class Provider : public std::enable_shared_from_this<Provider>
{
public:
  template <typename UpdateT, typename... Args>
  void push(Args&&... args)
  {
    static_assert(
      std::is_base_of_v<UpdateBase, UpdateT>,
      "Update must be derived from UpdateBase");

    auto update = std::make_shared<UpdateT>(std::forward<Args>(args)...);
    push_shared(std::move(update));
  }

  void push_shared(std::shared_ptr<UpdateBase> update);

  template <typename UpdateT>
  void on(std::function<void(std::shared_ptr<UpdateT>)> provider)
  {
    static_assert(
      std::is_base_of_v<UpdateBase, UpdateT>,
      "Update must be derived from UpdateBase");

    auto wrapper = [cb =
                      std::move(provider)](std::shared_ptr<UpdateBase> update) {
      cb(std::static_pointer_cast<UpdateT>(update));
    };

    std::lock_guard<std::mutex> lock(registry_mutex_);
    providers_[std::type_index(typeid(UpdateT))].push_back(std::move(wrapper));
  }

private:
  using ErasedProvider = std::function<void(std::shared_ptr<UpdateBase>)>;
  std::unordered_map<std::type_index, std::vector<ErasedProvider>> providers_;
  std::mutex registry_mutex_;
};

}  // namespace execution
}  // namespace vda5050_core

#endif  // VDA5050_CORE__EXECUTION__PROVIDER_HPP_
