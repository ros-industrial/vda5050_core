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

#ifndef VDA5050_MASTER__COMMUNICATION__ROS2_HPP_
#define VDA5050_MASTER__COMMUNICATION__ROS2_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "vda5050_master/communication/communication.hpp"

class Ros2Communication : public ICommunicationStrategy
{
public:
  Ros2Communication(
    std::shared_ptr<rclcpp::Node> node, const std::string& node_name);

  void subscribe(
    const std::string& topic, MessageCallback callback,
    const int qos = 0) override;

  void connect() override;

  void disconnect() override;

  void send_message(
    const std::string& topic, const std::string& message,
    const int qos = 0) override;

  ConnectionState get_state() const override;

private:
  std::shared_ptr<rclcpp::Publisher<std_msgs::msg::String>>
  get_or_create_publisher(const std::string& topic, int qos);

  rclcpp::QoS convert_qos(int vda5050_qos);

  std::shared_ptr<rclcpp::Node> node_;
  std::string node_name_;
  ConnectionState state_{ConnectionState::DISCONNECTED};
  mutable std::mutex state_mutex_;

  // Store subscriptions and publishers to keep them alive
  std::unordered_map<
    std::string, rclcpp::Subscription<std_msgs::msg::String>::SharedPtr>
    subscriptions_;
  std::unordered_map<
    std::string, rclcpp::Publisher<std_msgs::msg::String>::SharedPtr>
    publishers_;
  std::mutex publisher_mutex_;
  std::mutex subscriber_mutex_;
};

#endif  // VDA5050_MASTER__COMMUNICATION__ROS2_HPP_
