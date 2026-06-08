/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
 * Advanced Remanufacturing and Technology Centre
 * A*STAR Research Entities (Co. Registration No. 199702110H)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vda5050_core/client/contexts/agv_context.hpp"

#include <memory>
#include <typeindex>

#include "vda5050_core/client/updates/order.hpp"

namespace vda5050_core {

namespace client {

void AGVContext::init()
{
  if (initialized_) return;
  initialized_ = true;

  cache_resource(config_);
  cache_resource(execution_);

  // Register provider callbacks so incoming updates are cached automatically
  provider()->on<OrderUpdate>([this](std::shared_ptr<OrderUpdate> update) {
    this->cache_update(update);
  });
}

// Thread-safe update retrieval, lock covers the map read
std::shared_ptr<execution::UpdateBase> AGVContext::get_update_raw(
  std::type_index type) const
{
  std::lock_guard<std::mutex> lock(storage_mutex_);
  auto it = updates_.find(type);
  if (it != updates_.end()) return it->second;
  return nullptr;
}

// Thread-safe resource retrieval, lock covers the map read
std::shared_ptr<execution::ResourceBase> AGVContext::get_resource_raw(
  std::type_index type) const
{
  std::lock_guard<std::mutex> lock(storage_mutex_);
  auto it = resources_.find(type);
  if (it != resources_.end()) return it->second;
  return nullptr;
}

// Stores an update and wakes the Handler
void AGVContext::cache_update(std::shared_ptr<execution::UpdateBase> update)
{
  if (!update) return;
  {
    std::lock_guard<std::mutex> lock(storage_mutex_);
    updates_[update->get_type()] = update;
  }
  notify_on_change();
}

// Stores a resource (no wake needed as resources are not event-driven)
void AGVContext::cache_resource(
  std::shared_ptr<execution::ResourceBase> resource)
{
  if (!resource) return;
  std::lock_guard<std::mutex> lock(storage_mutex_);
  resources_[resource->get_type()] = resource;
}

}  // namespace client
}  // namespace vda5050_core
