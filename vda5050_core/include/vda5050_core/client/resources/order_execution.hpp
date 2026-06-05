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

#ifndef VDA5050_CORE__CLIENT__RESOURCES__ORDER_EXECUTION_HPP_
#define VDA5050_CORE__CLIENT__RESOURCES__ORDER_EXECUTION_HPP_

#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "vda5050_core/execution/base.hpp"
#include "vda5050_core/types/action_state.hpp"
#include "vda5050_core/types/edge_state.hpp"
#include "vda5050_core/types/node_state.hpp"
#include "vda5050_core/types/state.hpp"

namespace vda5050_core {

namespace client {

/// \brief Persistent, mutable AGV execution state shared across strategies.
///
/// Owns the working VDA5050 `State` snapshot together with runtime flags for
/// the current order. Access is synchronised internally, so callers do not
/// need to manage locking themselves.
///
/// Read APIs return copies, while `update_state()` allows safe in-place
/// modification under the internal mutex.
///
/// Multiple strategies may access this resource concurrently via
/// get_resource<OrderExecutionResource>().
class OrderExecutionResource
: public execution::Initialize<OrderExecutionResource, execution::ResourceBase>
{
public:
  /// \brief Whether the AGV is actively executing an order.
  bool is_executing_order() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return executing_order_;
  }

  /// \brief Set whether the AGV is actively executing an order.
  void set_executing_order(bool executing)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    executing_order_ = executing;
  }

  /// \brief Whether the AGV is waiting for a base extension from the master.
  bool is_awaiting_order_update() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return awaiting_order_update_;
  }

  /// \brief Set whether the AGV is waiting for a base extension.
  void set_awaiting_order_update(bool awaiting)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    awaiting_order_update_ = awaiting;
  }

  /// \brief Order ID of the current or last finished order ("" if none).
  std::string get_active_order_id() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.order_id;
  }

  /// \brief Update ID of the accepted order (0 if none).
  uint32_t get_active_order_update_id() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.order_update_id;
  }

  /// \brief Node ID of the last reached/current node ("" if none).
  std::string get_last_node_id() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.last_node_id;
  }

  /// \brief Sequence ID of the last reached/current node (0 if none).
  uint32_t get_last_node_sequence_id() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.last_node_sequence_id;
  }

  /// \brief Copy of the node states still to be traversed.
  std::vector<types::NodeState> get_node_states() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.node_states;
  }

  /// \brief Copy of the edge states still to be traversed.
  std::vector<types::EdgeState> get_edge_states() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.edge_states;
  }

  /// \brief Copy of the current and pending action states.
  std::vector<types::ActionState> get_action_states() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.action_states;
  }

  /// \brief Return a copy of the full working state snapshot.
  types::State get_state() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
  }

  /// \brief Replace the full working state snapshot.
  void set_state(types::State state)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = std::move(state);
  }

  /// \brief Mutate the working state under the resource lock.
  template <typename Fn>
  void update_state(Fn&& fn)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    fn(state_);
  }

private:
  /// \brief Serialises concurrent reads and writes to the members below.
  mutable std::mutex mutex_;

  /// \brief True while the AGV is actively executing an order.
  bool executing_order_ = false;

  /// \brief True when AGV is waiting for base extension from master.
  bool awaiting_order_update_ = false;

  /// \brief Working VDA5050 state snapshot.
  ///
  /// Includes:
  /// - order_id, order_update_id
  /// - last_node_id, last_node_sequence_id
  /// - node_states, edge_states, action_states
  types::State state_;
};

}  // namespace client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__CLIENT__RESOURCES__ORDER_EXECUTION_HPP_
