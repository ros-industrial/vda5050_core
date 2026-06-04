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

#ifndef VDA5050_CORE__CLIENT__UPDATES__ORDER_CONTEXT_HPP_
#define VDA5050_CORE__CLIENT__UPDATES__ORDER_CONTEXT_HPP_

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "vda5050_core/client/resources/config.hpp"
#include "vda5050_core/client/resources/order_execution.hpp"
#include "vda5050_core/client/updates/order.hpp"
#include "vda5050_core/execution/context_interface.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/edge_state.hpp"
#include "vda5050_core/types/node_state.hpp"

namespace vda5050_core {
namespace client {

/// \brief AGV client context: in-memory storage for VDA5050 execution data.
///
/// Seeds `HeaderConfigResource` and `OrderExecutionResource` as persistent
/// resources. Registers a provider callback for `OrderUpdate` so strategies
/// can read the latest inbound order via get_update<OrderUpdate>().
///
/// All map reads and writes are protected by a single `storage_mutex_` via
/// std::lock_guard to prevent race conditions when strategies run concurrently.
class OrderContext : public execution::ContextInterface
{
public:
  /// \brief Construct with AGV identity resource.
  ///
  /// `HeaderConfigResource` and a blank `OrderExecutionResource` are available
  /// via get_resource<T>() after init() is called.
  explicit OrderContext(std::shared_ptr<HeaderConfigResource> config)
  : config_(std::move(config)),
    execution_(std::make_shared<OrderExecutionResource>())
  {
    if (!config_)
    {
      throw std::invalid_argument("OrderContext: config cannot be nullptr");
    }
  }

  ~OrderContext() override = default;

  /// \brief Seed resources and register the provider callback for updates.
  ///
  /// Seeds HeaderConfigResource and OrderExecutionResource into the resource
  /// map. Registers provider()->on<OrderUpdate>() to cache incoming orders.
  /// Each update triggers notify_on_change() so the Handler wakes up.
  void init() override;

  std::string get_active_order_id() const;
  uint32_t get_active_order_update_id() const;
  std::string get_last_node_id() const;
  uint32_t get_last_node_sequence_id() const;
  std::vector<types::NodeState> get_node_states() const;
  std::vector<types::EdgeState> get_edge_states() const;
  std::vector<types::ActionState> get_action_states() const;

protected:
  /// \brief Thread-safe lookup in the updates map. Returns nullptr if not found.
  std::shared_ptr<execution::UpdateBase> get_update_raw(
    std::type_index type) const override;

  /// \brief Thread-safe lookup in the resources map. Returns nullptr if not found.
  std::shared_ptr<execution::ResourceBase> get_resource_raw(
    std::type_index type) const override;

private:
  // Thread-safe internal layout caches
  void cache_update(std::shared_ptr<execution::UpdateBase> update);
  void cache_resource(std::shared_ptr<execution::ResourceBase> resource);

  // config_ and execution_ are also seeded into resources_ during init() so
  // strategies can reach them via get_resource<T>(). They are kept as direct
  // members for the bridge getters below, which read the same shared object.
  std::shared_ptr<HeaderConfigResource> config_;
  std::shared_ptr<OrderExecutionResource> execution_;

  std::unordered_map<std::type_index, std::shared_ptr<execution::UpdateBase>>
    updates_;
  std::unordered_map<std::type_index, std::shared_ptr<execution::ResourceBase>>
    resources_;
  mutable std::mutex storage_mutex_;
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__UPDATES__ORDER_CONTEXT_HPP_
