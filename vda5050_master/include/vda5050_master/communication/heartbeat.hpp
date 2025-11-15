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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "vda5050_core/logger/logger.hpp"

#ifndef VDA5050_MASTER__COMMUNICATION__HEARTBEAT_HPP_
#define VDA5050_MASTER__COMMUNICATION__HEARTBEAT_HPP_

namespace vda5050_master {
namespace communication {

class ConnectionHeartbeatListener
{
public:
  ConnectionHeartbeatListener(
    const std::string& id, const int heartbeat_interval,
    std::function<void()> disconnection_callback)
  : id_(id),
    heartbeat_interval_(heartbeat_interval),
    running_(false),
    last_connection_report_(std::chrono::steady_clock::now()),
    disconnection_callback_(disconnection_callback)
  {
    // Nothing to do here ...
  }

  void start_connection_heartbeat()
  {
    {
      std::lock_guard<std::mutex> lock(running_lock_);
      VDA5050_INFO("Starting Connection heartbeat listener");
      running_ = true;
    }
    connection_thread_ =
      std::thread(&ConnectionHeartbeatListener::listen, this);
  }

  void received_connection()
  {
    if (!connection_thread_.joinable()) {
      VDA5050_INFO("Connection heartbeat not started yet, ignored...");
      return;
    }
    std::lock_guard<std::mutex> lock(last_connection_report_mutex_);
    last_connection_report_ = get_current_time();
    VDA5050_INFO("[" + id_ + "] Received connection heartbeat");
    message_received_.notify_all();
  }

  std::chrono::steady_clock::time_point get_last_connection_report()
  {
    return last_connection_report_;
  }

  ~ConnectionHeartbeatListener()
  {
    stop_connection_heartbeat();
    VDA5050_INFO("[" + id_ + "] Deconstructing ConnectionHeartbeatListener");
  }

  bool is_running()
  {
    return running_;
  }

  // Stop the listener
  void stop_connection_heartbeat()
  {
    VDA5050_INFO("Stopping Connection heartbeat listener");
    if (connection_thread_.joinable()) {
      connection_thread_.join();
    } else {
      VDA5050_INFO("Connection thread not joinable");
    }
    VDA5050_INFO("Stopped Connection heartbeat listener");

    {
      std::lock_guard<std::mutex> lock(running_lock_);
      running_ = false;
    }
  }

  virtual std::chrono::steady_clock::time_point get_current_time()
  {
    VDA5050_INFO("Get Current Time no skip");
    return std::chrono::steady_clock::now();
  }

  // Virtual method to allow overriding during tests.
  virtual int get_check_interval()
  {
    return heartbeat_interval_;
  }

protected:
  std::condition_variable message_received_;

private:
  bool is_timeout()
  {
    VDA5050_INFO("Check Timeout");
    std::chrono::steady_clock::time_point current_time = get_current_time();
    int time_since_last_connection_report;
    {
      VDA5050_INFO("Bef lock Timeout");
      std::lock_guard<std::mutex> lock(last_connection_report_mutex_);
      time_since_last_connection_report =
        std::chrono::duration_cast<std::chrono::seconds>(
          current_time - get_last_connection_report())
          .count();
      VDA5050_INFO("aft lock TTimeout");
    }

    VDA5050_INFO(
      "time_since_last_connection_report" +
      std::to_string(time_since_last_connection_report));
    if (std::abs(time_since_last_connection_report) > heartbeat_interval_) {
      VDA5050_INFO(
        "Connection heartbeat lost after waiting for the maximum of " +
        std::to_string(heartbeat_interval_) + " seconds");
      VDA5050_ERROR(
        "Time since last connection report: " +
        std::to_string(time_since_last_connection_report) + " seconds");
      return true;
    }
    VDA5050_INFO("No");
    return false;
  }

  // Listener loop

  void listen()
  {
    while (is_running()) {
      VDA5050_INFO("Start Listen");
      std::unique_lock<std::mutex> lock(check_lock_);
      message_received_.wait_for(
        lock, std::chrono::seconds(get_check_interval()));
      if (is_timeout()) {
        VDA5050_INFO("Timeout reached");
        callback_thread_ = std::thread([&]() { disconnection_callback_(); });
        VDA5050_INFO("[" + id_ + "] Waiting for callback thread to finish");
        if (callback_thread_.joinable()) {
          callback_thread_.join();
        }
        VDA5050_INFO("Stopping hearbeat checks");
        return;
      }
    }
  }

  std::string id_;

  std::thread connection_thread_;

  std::thread callback_thread_;

  int heartbeat_interval_;

  std::atomic<bool> running_;

  std::chrono::steady_clock::time_point last_connection_report_;

  std::thread connection_listener_;

  std::mutex mutex_;  // Mutex to protect access to shared state

  std::mutex
    last_connection_report_mutex_;  // Mutex to protect access to last_connection_report_

  std::mutex running_lock_;

  std::mutex check_lock_;

  std::function<void()> disconnection_callback_;
};

}  // namespace communication
}  // namespace vda5050_master

#endif  // VDA5050_MASTER__COMMUNICATION__HEARTBEAT_HPP_
