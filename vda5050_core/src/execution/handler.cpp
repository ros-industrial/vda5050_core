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

#include <thread>
#include <utility>

#include "vda5050_core/execution/handler.hpp"

namespace vda5050_core {

namespace execution {

//=============================================================================
std::shared_ptr<Handler> Handler::make(
  std::shared_ptr<ContextInterface> context,
  const std::vector<std::shared_ptr<StrategyInterface>>& strategies)
{
  auto handler = std::shared_ptr<Handler>(new Handler(context, strategies));
  return handler;
}

//=============================================================================
void Handler::add_strategy(std::shared_ptr<StrategyInterface> strategy)
{
  if (!strategy) return;

  std::lock_guard<std::mutex> lock(mutex_);
  if (
    std::find(strategies_.begin(), strategies_.end(), strategy) !=
    strategies_.end())
    return;
  strategies_.push_back(strategy);
  strategy->init(context_);
}

//=============================================================================
void Handler::remove_strategy(std::shared_ptr<StrategyInterface> strategy)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = std::find(strategies_.begin(), strategies_.end(), strategy);
  if (it != strategies_.end()) strategies_.erase(it);
}

//=============================================================================
void Handler::wake()
{
  if (!spinning_) return;

  std::lock_guard<std::mutex> lock(mutex_);
  cv_.notify_all();
}

//=============================================================================
void Handler::spin(std::chrono::milliseconds timeout)
{
  running_ = true;
  spinning_ = true;

  while (true)
  {
    std::unique_lock lock(mutex_);
    cv_.wait_for(lock, timeout);

    if (!running_) break;

    lock.unlock();

    step_active_strategies();
  }

  spinning_ = false;
}

//=============================================================================
void Handler::spin_once()
{
  step_active_strategies();
}

//=============================================================================
std::vector<std::shared_ptr<StrategyInterface>> Handler::get_active_strategies()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return strategies_;
}

//=============================================================================
bool Handler::running()
{
  return running_;
}

//=============================================================================
void Handler::stop()
{
  if (running_)
  {
    running_ = false;
    wake();
  }

  while (spinning_)
  {
    std::this_thread::yield();
  }
}

//=============================================================================
Handler::Handler(
  std::shared_ptr<ContextInterface> context,
  const std::vector<std::shared_ptr<StrategyInterface>>& strategies)
: context_(std::move(context)), running_(false), spinning_(false)
{
  context_->init();

  context_->on_change([&] { wake(); });

  for (auto strategy : strategies) add_strategy(strategy);

  spin_once();
}

//=============================================================================
void Handler::step_active_strategies()
{
  auto active_strategies = get_active_strategies();

  for (auto& strategy : active_strategies)
  {
    strategy->step(context_);
  }
}

}  // namespace execution
}  // namespace vda5050_core
