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

#include "vda5050_master/communication/ros2.hpp"

#include "vda5050_core/logger/logger.hpp"

Ros2Communication::Ros2Communication(
  std::shared_ptr<rclcpp::Node> node, const std::string& node_name)
: ICommunicationStrategy(), node_(node), node_name_(node_name)
{
  VDA5050_INFO(
    "[ROS2] Initializing ROS2 communication for node: {}", node_name_);
}

void Ros2Communication::subscribe(
  const std::string& topic, MessageCallback callback, const int qos)
{
  // Convert VDA5050 QoS to ROS2 QoS
  auto ros2_qos = convert_qos(qos);

  // Create a subscriber for this topic
  auto subscription = node_->create_subscription<std_msgs::msg::String>(
    topic, ros2_qos,
    [this, topic, callback](const std_msgs::msg::String::SharedPtr msg) {
      callback(topic, msg->data);
    });

  std::lock_guard<std::mutex> lock(subscriber_mutex_);
  // Store the subscription to keep it alive
  subscriptions_[topic] = subscription;
  VDA5050_INFO("[ROS2] Subscribed to topic: {}", topic);
}

void Ros2Communication::unsubscribe(const std::string& topic)
{
  std::lock_guard<std::mutex> lock(subscriber_mutex_);
  auto it = subscriptions_.find(topic);
  if (it != subscriptions_.end())
  {
    subscriptions_.erase(it);
    VDA5050_INFO("[ROS2] Unsubscribed from topic: {}", topic);
  }
  else
  {
    VDA5050_WARN(
      "[ROS2] Cannot unsubscribe: no subscription for topic: {}", topic);
  }
}

void Ros2Communication::connect()
{
  // ROS2 nodes are connected when created, but we can initialize publishers here
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    state_ = ConnectionState::CONNECTED;
  }
  VDA5050_INFO("[ROS2] Node connected and ready");
}

void Ros2Communication::disconnect()
{
  VDA5050_INFO("[ROS2] Disconnecting from ROS2 communication");

  // Signal disconnecting state
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    state_ = ConnectionState::DISCONNECTING;
  }

  // Clear all subscriptions and publishers
  {
    std::lock_guard<std::mutex> lock(subscriber_mutex_);
    subscriptions_.clear();
  }
  {
    std::lock_guard<std::mutex> lock(publisher_mutex_);
    publishers_.clear();
  }

  // Mark as fully disconnected
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    state_ = ConnectionState::DISCONNECTED;
  }

  VDA5050_INFO("[ROS2] Disconnected from ROS2 communication");
}

void Ros2Communication::send_message(
  const std::string& topic, const std::string& message, const int qos)
{
  if (get_state() != ConnectionState::CONNECTED)
  {
    VDA5050_WARN("[ROS2] Cannot send message - not connected");
    return;
  }

  // Get or create publisher for this topic
  auto publisher = get_or_create_publisher(topic, qos);

  // Create and publish the message
  auto ros_msg = std_msgs::msg::String();
  ros_msg.data = message;
  publisher->publish(ros_msg);

  VDA5050_INFO("[ROS2] Published message to topic: {}", topic);
}

ConnectionState Ros2Communication::get_state() const
{
  std::lock_guard<std::mutex> lock(state_mutex_);
  return state_;
}

std::shared_ptr<rclcpp::Publisher<std_msgs::msg::String>>
Ros2Communication::get_or_create_publisher(const std::string& topic, int qos)
{
  // Check if publisher already exists
  std::lock_guard<std::mutex> lock(publisher_mutex_);
  auto it = publishers_.find(topic);
  if (it != publishers_.end())
  {
    return it->second;
  }

  // Create new publisher
  auto ros2_qos = convert_qos(qos);
  auto publisher =
    node_->create_publisher<std_msgs::msg::String>(topic, ros2_qos);
  publishers_[topic] = publisher;

  VDA5050_INFO("[ROS2] Created publisher for topic: {}", topic);
  return publisher;
}

rclcpp::QoS Ros2Communication::convert_qos(int vda5050_qos)
{
  // 0 = At most once (Best effort)
  // 1 = At least once (Reliable)
  // 2 = Exactly once (Reliable + Transient local)
  switch (vda5050_qos)
  {
    case 0:
      return rclcpp::QoS(10).best_effort().durability_volatile();
    case 1:
      return rclcpp::QoS(10).reliable().durability_volatile();
    case 2:
      return rclcpp::QoS(10).reliable().transient_local();
    default:
      VDA5050_WARN("[ROS2] Unknown QoS level {}, using default", vda5050_qos);
      return rclcpp::QoS(10);
  }
}
