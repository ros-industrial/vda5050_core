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
#include <string>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "vda5050_core/logger/logger.hpp"
#include "vda5050_master/communication/communication.hpp"

class Ros2Communication : public ICommunicationStrategy
{
public:
  Ros2Communication(
    std::shared_ptr<rclcpp::Node> node, const std::string& node_name)
  : ICommunicationStrategy(), node_(node), node_name_(node_name)
  {
    VDA5050_INFO(
      "[ROS2] Initializing ROS2 communication for node: " + node_name_);
  }

  void subscribe(
    const std::string& topic, MessageCallback callback,
    const int qos = 0) override
  {
    // Convert VDA5050 QoS to ROS2 QoS
    auto ros2_qos = convert_qos(qos);

    // Create a subscriber for this topic
    auto subscription = node_->create_subscription<std_msgs::msg::String>(
      topic, ros2_qos,
      [this, topic, callback](const std_msgs::msg::String::SharedPtr msg) {
        callback(topic, msg->data);
      });

    // Store the subscription to keep it alive
    subscriptions_[topic] = subscription;
    VDA5050_INFO("[ROS2] Subscribed to topic: " + topic);
  }

  void connect() override
  {
    // ROS2 nodes are connected when created, but we can initialize publishers here
    connected_ = true;
    VDA5050_INFO("[ROS2] Node connected and ready");
  }

  void disconnect() override
  {
    // Clear all subscriptions and publishers
    subscriptions_.clear();
    publishers_.clear();
    connected_ = false;
    VDA5050_INFO("[ROS2] Disconnecting from ROS2 communication");
  }

  void send_message(
    const std::string& topic, const std::string& message,
    const int qos = 0) override
  {
    if (!connected_) {
      VDA5050_WARN("[ROS2] Cannot send message - not connected");
      return;
    }

    // Get or create publisher for this topic
    auto publisher = get_or_create_publisher(topic, qos);

    // Create and publish the message
    auto ros_msg = std_msgs::msg::String();
    ros_msg.data = message;
    publisher->publish(ros_msg);

    VDA5050_INFO("[ROS2] Published message to topic: " + topic);
  }

  std::shared_ptr<rclcpp::Publisher<std_msgs::msg::String>>
  get_or_create_publisher(const std::string& topic, int qos)
  {
    // Check if publisher already exists
    auto it = publishers_.find(topic);
    if (it != publishers_.end()) {
      return it->second;
    }

    // Create new publisher
    auto ros2_qos = convert_qos(qos);
    auto publisher =
      node_->create_publisher<std_msgs::msg::String>(topic, ros2_qos);
    publishers_[topic] = publisher;

    VDA5050_INFO("[ROS2] Created publisher for topic: " + topic);
    return publisher;
  }

private:
  rclcpp::QoS convert_qos(int vda5050_qos)
  {
    // 0 = At most once (Best effort)
    // 1 = At least once (Reliable)
    // 2 = Exactly once (Reliable + Transient local)
    switch (vda5050_qos) {
      case 0:
        return rclcpp::QoS(10).best_effort().durability_volatile();
      case 1:
        return rclcpp::QoS(10).reliable().durability_volatile();
      case 2:
        return rclcpp::QoS(10).reliable().transient_local();
      default:
        VDA5050_WARN(
          "[ROS2] Unknown QoS level " + std::to_string(vda5050_qos) +
          ", using default");
        return rclcpp::QoS(10);
    }
  }

  std::shared_ptr<rclcpp::Node> node_;
  std::string node_name_;
  bool connected_{false};

  // Store subscriptions and publishers to keep them alive
  std::unordered_map<
    std::string, rclcpp::Subscription<std_msgs::msg::String>::SharedPtr>
    subscriptions_;
  std::unordered_map<
    std::string, rclcpp::Publisher<std_msgs::msg::String>::SharedPtr>
    publishers_;
};

#endif  // VDA5050_MASTER__COMMUNICATION__ROS2_HPP_
