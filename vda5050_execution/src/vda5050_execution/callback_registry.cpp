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

#include "vda5050_execution/callback_registry.hpp"

namespace vda5050_execution {

//=============================================================================
void CallbackRegistry::dispatch(std::shared_ptr<EventBase> event) const
{
  auto it = callbacks_.find(event->get_type());
  if (it != callbacks_.end())
  {
    for (const auto& cb : it->second)
    {
      cb(event);
    }
  }
}

//=============================================================================
void CallbackRegistry::query(std::shared_ptr<UpdateBase> update) const
{
  auto it = providers_.find(update->get_type());
  if (it != providers_.end())
  {
    auto& cb = it->second;
    cb(update);
  }
}

}  // namespace vda5050_execution
