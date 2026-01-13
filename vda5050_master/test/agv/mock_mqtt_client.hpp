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

#ifndef AGV__MOCK_MQTT_CLIENT_HPP_
#define AGV__MOCK_MQTT_CLIENT_HPP_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"

namespace vda5050_master::test {

/**
 * @brief Mock MQTT Client for unit testing
 *
 * This mock provides:
 * - Basic connect/disconnect/subscribe/unsubscribe/publish operations
 * - Message simulation via simulate_receive()
 * - Topic lookup via find_topic_containing()
 * - Message tracking and waiting (for verifying published messages)
 * - Blocking support for queue policy testing
 */
class MockMqttClient : public vda5050_core::mqtt_client::MqttClientInterface
{
public:
  MockMqttClient() : connected_(false) {}

  void connect() override
  {
    connected_ = true;
  }

  void disconnect() override
  {
    connected_ = false;
  }

  bool connected() override
  {
    return connected_;
  }

  void subscribe(
    const std::string& topic, MessageHandler handler, int /*qos*/) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_[topic] = handler;
  }

  void unsubscribe(const std::string& topic) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.erase(topic);
  }

  void publish(
    const std::string& topic, const std::string& message, int /*qos*/,
    bool /*retain*/ = false) override
  {
    // Wait until unblocked (for queue policy testing)
    {
      std::unique_lock<std::mutex> lock(block_mutex_);
      blocked_waiters_++;
      blocked_waiter_cv_.notify_all();  // Notify that we're waiting
      block_cv_.wait(lock, [this] { return !blocked_; });
      blocked_waiters_--;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    sent_messages_.push_back({topic, message});
    send_count_++;
    cv_.notify_all();
  }

  void set_will(
    const std::string& /*topic*/, const std::string& /*message*/,
    int /*qos*/) override
  {
    // No-op for testing
  }

  // =========================================================================
  // Blocking control for queue policy testing
  // =========================================================================

  void block()
  {
    std::lock_guard<std::mutex> lock(block_mutex_);
    blocked_ = true;
  }

  void unblock()
  {
    std::lock_guard<std::mutex> lock(block_mutex_);
    blocked_ = false;
    block_cv_.notify_all();
  }

  // Wait until at least one message is blocked in publish
  bool wait_for_blocked(
    std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
  {
    std::unique_lock<std::mutex> lock(block_mutex_);
    return blocked_waiter_cv_.wait_for(
      lock, timeout, [this] { return blocked_waiters_ > 0; });
  }

  // =========================================================================
  // Test helper methods for message tracking
  // =========================================================================

  size_t get_send_count() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return send_count_;
  }

  std::vector<std::pair<std::string, std::string>> get_sent_messages() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return sent_messages_;
  }

  void clear_sent_messages()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    sent_messages_.clear();
    send_count_ = 0;
  }

  bool wait_for_messages(
    size_t count,
    std::chrono::milliseconds timeout = std::chrono::milliseconds(5000))
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(
      lock, timeout, [this, count] { return send_count_ >= count; });
  }

  // =========================================================================
  // Test helper methods for subscription simulation
  // =========================================================================

  // Simulate receiving a message on a subscribed topic
  void simulate_receive(const std::string& topic, const std::string& payload)
  {
    MessageHandler handler;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = subscriptions_.find(topic);
      if (it != subscriptions_.end())
      {
        handler = it->second;
      }
    }
    // Call handler outside lock to avoid deadlock
    if (handler)
    {
      handler(topic, payload);
    }
  }

  // Find a topic containing the given substring
  std::string find_topic_containing(const std::string& substr) const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [topic, _] : subscriptions_)
    {
      if (topic.find(substr) != std::string::npos)
      {
        return topic;
      }
    }
    return "";
  }

private:
  std::atomic<bool> connected_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::map<std::string, MessageHandler> subscriptions_;
  std::vector<std::pair<std::string, std::string>> sent_messages_;
  size_t send_count_ = 0;

  // Blocking support for queue policy testing
  bool blocked_{false};
  size_t blocked_waiters_{0};
  std::mutex block_mutex_;
  std::condition_variable block_cv_;
  std::condition_variable blocked_waiter_cv_;
};

}  // namespace vda5050_master::test

#endif  // AGV__MOCK_MQTT_CLIENT_HPP_
