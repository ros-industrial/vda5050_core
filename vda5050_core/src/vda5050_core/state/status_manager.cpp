/*
 * Copyright (C) 2025 ROS-Industrial Consortium Asia Pacific
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

#include <algorithm>
#include <mutex>

#include "vda5050_core/state/status_manager.hpp"

namespace vda5050_core {

namespace msg {

void StatusManager::set_agv_position(
  const std::optional<AgvPosition>& agv_position)
{
  std::unique_lock lock(this->mutex_);
  this->agv_position_ = agv_position;
}

std::optional<AgvPosition> StatusManager::get_agv_position()
{
  std::shared_lock lock(this->mutex_);
  return this->agv_position_;
}

void StatusManager::set_velocity(const std::optional<Velocity>& velocity)
{
  std::unique_lock lock(this->mutex_);
  this->velocity_ = velocity;
}

std::optional<Velocity> StatusManager::get_velocity() const
{
  std::shared_lock lock(this->mutex_);
  return this->velocity_;
}

bool StatusManager::set_driving(bool driving)
{
  std::unique_lock lock(this->mutex_);
  bool changed = this->driving_ != driving;
  this->driving_ = driving;
  return changed;
}

void StatusManager::set_distance_since_last_node(
  double distance_since_last_node)
{
  std::unique_lock lock(this->mutex_);
  this->distance_since_last_node_ = distance_since_last_node;
}

void StatusManager::reset_distance_since_last_node()
{
  std::unique_lock lock(this->mutex_);
  this->distance_since_last_node_.reset();
}

bool StatusManager::add_load(const Load& load)
{
  std::unique_lock lock(this->mutex_);

  if (!this->loads_.has_value())
  {
    this->loads_ = {load};
  }
  else
  {
    this->loads_->push_back(load);
  }

  return true;
}

bool StatusManager::remove_load(std::string_view load_id)
{
  std::unique_lock lock(this->mutex_);

  if (!this->loads_.has_value())
  {
    return false;
  }

  auto match_id = [load_id](const Load& load) {
    return load.load_id == load_id;
  };

  auto before_size = this->loads_->size();

  this->loads_->erase(
    std::remove_if(this->loads_->begin(), this->loads_->end(), match_id),
    this->loads_->end());

  return this->loads_->size() != before_size;
}

const std::vector<Load>& StatusManager::get_loads()
{
  std::shared_lock lock(
    this->mutex_);  // Ensure that loads is not being altered at the moment

  const static std::vector<Load> empty;

  // value_or turns empty into a stack object, so this if block is required
  if (this->loads_.has_value())
  {
    return *this->loads_;
  }
  else
  {
    return empty;
  }
}

}  // namespace msg
}  // namespace vda5050_core