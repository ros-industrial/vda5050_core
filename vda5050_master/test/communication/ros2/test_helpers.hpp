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

#ifndef COMMUNICATION__ROS2__TEST_HELPERS_HPP_
#define COMMUNICATION__ROS2__TEST_HELPERS_HPP_

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "test_helpers.hpp"

namespace vda5050_master::test::ros2 {
namespace constants {
constexpr auto ROS2_POLL_INTERVAL = std::chrono::milliseconds(100);
}  // namespace constants
/**
 * @brief Helper to publish multiple messages to a ROS2 topic
 */
inline void publish_messages(
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher,
  const std::vector<std::string>& payloads)
{
  for (const auto& payload : payloads)
  {
    auto msg = std_msgs::msg::String();
    msg.data = payload;
    publisher->publish(msg);
  }
}

/**
 * @brief Helper to wait for a condition while spinning ROS2 executor
 */
template <typename Condition>
bool wait_for_condition_with_spin(
  rclcpp::executors::SingleThreadedExecutor& executor, Condition condition,
  std::chrono::milliseconds timeout =
    vda5050_master::test::constants::DEFAULT_TIMEOUT,
  std::chrono::milliseconds spin_interval = constants::ROS2_POLL_INTERVAL)
{
  auto start = std::chrono::steady_clock::now();
  while (!condition() && (std::chrono::steady_clock::now() - start) < timeout)
  {
    executor.spin_some(spin_interval);
  }
  return condition();
}

/**
 * @brief Helper to wait for publisher-subscriber discovery
 */
template <typename PublisherOrSubscriber>
bool wait_for_discovery(
  rclcpp::executors::SingleThreadedExecutor& executor,
  PublisherOrSubscriber pub_or_sub, size_t expected_count = 1)
{
  return wait_for_condition_with_spin(
    executor,
    [&]() { return pub_or_sub->get_subscription_count() >= expected_count; },
    vda5050_master::test::constants::DISCOVERY_TIMEOUT,
    vda5050_master::test::constants::DISCOVERY_POLL_INTERVAL);
}

/**
 * @brief Specialization for subscriber checking publisher count
 */
inline bool wait_for_publisher_discovery(
  rclcpp::executors::SingleThreadedExecutor& executor,
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription,
  size_t expected_count = 1)
{
  return wait_for_condition_with_spin(
    executor,
    [&]() { return subscription->get_publisher_count() >= expected_count; },
    vda5050_master::test::constants::DISCOVERY_TIMEOUT,
    vda5050_master::test::constants::DISCOVERY_POLL_INTERVAL);
}

/**
 * @brief Helper to create and configure a test executor with multiple nodes
 */
class TestExecutor
{
public:
  TestExecutor()
  : executor_(std::make_unique<rclcpp::executors::SingleThreadedExecutor>())
  {
  }

  void add_node(std::shared_ptr<rclcpp::Node> node)
  {
    executor_->add_node(node);
  }

  rclcpp::executors::SingleThreadedExecutor& get()
  {
    return *executor_;
  }

  template <typename Condition>
  bool wait_for(
    Condition condition, std::chrono::milliseconds timeout =
                           vda5050_master::test::constants::DEFAULT_TIMEOUT)
  {
    return wait_for_condition_with_spin(*executor_, condition, timeout);
  }

private:
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor_;
};

}  // namespace vda5050_master::test::ros2

#endif  // COMMUNICATION__ROS2__TEST_HELPERS_HPP_
