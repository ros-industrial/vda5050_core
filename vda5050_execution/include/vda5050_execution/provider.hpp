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

#include <functional>
#include <memory>
#include <utility>

#include "vda5050_execution/callback_registry.hpp"
#include "vda5050_execution/update.hpp"

#ifndef VDA5050_EXECUTION__PROVIDER_HPP_
#define VDA5050_EXECUTION__PROVIDER_HPP_

namespace vda5050_execution {

class Provider : public std::enable_shared_from_this<Provider>
{
public:
  template <typename UpdateT, typename... Args>
  void push(Args&&... args)
  {
    static_assert(
      std::is_base_of_v<UpdateBase, UpdateT>,
      "Update must be derived from UpdateBase");
    callback_registry_.query(
      std::make_shared<UpdateT>(std::forward<Args>(args)...));
  }

  template <typename UpdateT>
  void on(std::function<void(std::shared_ptr<UpdateT>)> callback)
  {
    callback_registry_.register_provider<UpdateT>(std::move(callback));
  }

private:
  CallbackRegistry callback_registry_;
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__PROVIDER_HPP_
