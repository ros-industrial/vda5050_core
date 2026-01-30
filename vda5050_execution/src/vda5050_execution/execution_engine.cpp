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

#include "vda5050_execution/execution_engine.hpp"
#include "vda5050_execution/event.hpp"

namespace vda5050_execution {

//=============================================================================
void ExecutionEngine::emit_shared(std::shared_ptr<EventBase> event)
{
  event_queue_.push(event);
}

//=============================================================================
void ExecutionEngine::step()
{
  std::shared_ptr<EventBase> event = event_queue_.pop();
  if (!event) return;

  auto it = callbacks_.find(event->get_type());
  if (it != callbacks_.end())
  {
    for (const auto& cb : it->second)
    {
      cb(event);
    }
  }
}

}  // namespace vda5050_execution
