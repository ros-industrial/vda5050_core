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

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "vda5050_core/logger/logger.hpp"

#ifndef VDA5050_MASTER__COMMUNICATION__HEARTBEAT_HPP_
#define VDA5050_MASTER__COMMUNICATION__HEARTBEAT_HPP_

namespace vda5050_master {
namespace communication {

/**
 * @brief Heartbeat listener lifecycle states
 *
 * State transitions:
 *   STOPPED -> RUNNING (via start_connection_heartbeat())
 *   RUNNING -> STOPPING (via stop_connection_heartbeat())
 *   STOPPING -> STOPPED (when cleanup completes)
 */
enum class HeartbeatState
{
  STOPPED,  // Not running, safe to destroy or restart
  RUNNING,  // Actively monitoring heartbeats
  STOPPING  // Stop in progress, cleanup ongoing
};

class HeartbeatListener
{
public:
  HeartbeatListener(
    const std::string& id, const int heartbeat_interval,
    std::function<void()> disconnection_callback)
  : id_(id),
    heartbeat_interval_(heartbeat_interval),
    state_(HeartbeatState::STOPPED),
    last_connection_report_(std::chrono::system_clock::now()),
    disconnection_callback_(disconnection_callback)
  {
    // Nothing to do here ...
  }

  void start_connection_heartbeat()
  {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ != HeartbeatState::STOPPED)
    {
      VDA5050_DEBUG(
        "Connection heartbeat already running or stopping, ignored");
      return;
    }

    VDA5050_INFO("Starting Connection heartbeat listener");
    state_ = HeartbeatState::RUNNING;
    connection_thread_ = std::thread(&HeartbeatListener::listen, this);
  }

  void received_connection()
  {
    // Check state with proper synchronization
    if (get_state() != HeartbeatState::RUNNING)
    {
      VDA5050_DEBUG("Connection heartbeat not running, ignored...");
      return;
    }
    std::lock_guard<std::mutex> lock(last_connection_report_mutex_);
    last_connection_report_ = get_current_time();
    VDA5050_INFO("[" + id_ + "] Received connection heartbeat");
    message_received_.notify_all();
  }

  std::chrono::system_clock::time_point get_last_connection_report()
  {
    std::lock_guard<std::mutex> lock(last_connection_report_mutex_);
    return last_connection_report_;
  }

  ~HeartbeatListener()
  {
    stop_connection_heartbeat();
    VDA5050_INFO("[" + id_ + "] Deconstructing HeartbeatListener");
  }

  /**
   * @brief Get the current heartbeat listener state
   * @return HeartbeatState enum value
   */
  HeartbeatState get_state()
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_;
  }

  // Stop the listener
  void stop_connection_heartbeat()
  {
    std::thread conn_thread_to_join;

    {
      std::lock_guard<std::mutex> lock(state_mutex_);

      if (state_ == HeartbeatState::STOPPED)
      {
        VDA5050_DEBUG("Connection heartbeat already stopped");
        return;
      }

      VDA5050_INFO("Stopping Connection heartbeat listener");
      state_ = HeartbeatState::STOPPING;

      conn_thread_to_join = std::move(connection_thread_);
    }

    message_received_.notify_all();

    if (conn_thread_to_join.joinable())
    {
      conn_thread_to_join.join();
    }

    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      state_ = HeartbeatState::STOPPED;
    }

    VDA5050_INFO("Stopped Connection heartbeat listener");
  }

  virtual std::chrono::system_clock::time_point get_current_time()
  {
    return std::chrono::system_clock::now();
  }

  // Virtual method to allow overriding during tests.
  virtual int get_check_interval()
  {
    return heartbeat_interval_;
  }

protected:
  std::condition_variable message_received_;

private:
  bool is_stop_requested()
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_ == HeartbeatState::STOPPING;
  }

  bool is_timeout()
  {
    std::chrono::system_clock::time_point current_time = get_current_time();
    int time_since_last_connection_report;
    {
      std::lock_guard<std::mutex> lock(last_connection_report_mutex_);
      time_since_last_connection_report =
        std::chrono::duration_cast<std::chrono::seconds>(
          current_time - last_connection_report_)
          .count();
    }

    if (std::abs(time_since_last_connection_report) > heartbeat_interval_)
    {
      VDA5050_WARN(
        "[" + id_ + "] Connection heartbeat timeout after " +
        std::to_string(time_since_last_connection_report) + " seconds " +
        "(max: " + std::to_string(heartbeat_interval_) + "s)");
      return true;
    }
    return false;
  }

  // Listener loop

  void listen()
  {
    while (!is_stop_requested())
    {
      std::unique_lock<std::mutex> lock(check_lock_);
      message_received_.wait_for(
        lock, std::chrono::seconds(get_check_interval()));

      // Check if shutdown was requested while waiting
      if (is_stop_requested())
      {
        VDA5050_DEBUG("[" + id_ + "] Shutdown requested, exiting listen loop");
        return;
      }

      if (is_timeout())
      {
        disconnection_callback_();
        VDA5050_INFO(
          "[" + id_ + "] Heartbeat monitoring stopped after timeout");
        return;
      }
    }
  }

  std::string id_;

  std::thread connection_thread_;

  int heartbeat_interval_;

  // Lifecycle state protected by state_mutex_
  HeartbeatState state_;

  std::chrono::system_clock::time_point last_connection_report_;

  // Mutex for state variable
  mutable std::mutex state_mutex_;

  // Mutex to protect access to last_connection_report_
  mutable std::mutex last_connection_report_mutex_;

  // Mutex for condition variable wait
  std::mutex check_lock_;

  std::function<void()> disconnection_callback_;
};

}  // namespace communication
}  // namespace vda5050_master

#endif  // VDA5050_MASTER__COMMUNICATION__HEARTBEAT_HPP_
