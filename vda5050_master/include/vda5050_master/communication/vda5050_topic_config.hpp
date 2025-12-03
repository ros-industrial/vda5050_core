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

#ifndef VDA5050_MASTER__COMMUNICATION__VDA5050_TOPIC_CONFIG_HPP_
#define VDA5050_MASTER__COMMUNICATION__VDA5050_TOPIC_CONFIG_HPP_

#include <optional>
#include <stdexcept>
#include <string>

/**
 * @brief Pure configuration struct for VDA5050 topic names
 *
 * This struct holds only topic configuration for the 6 standard VDA5050
 * message types. Data handling is managed separately via VDA5050Handlers.
 *
 * Connection and State topics are mandatory per VDA5050 spec.
 */
struct VDA5050TopicConfig
{
  // ========== REQUIRED TOPICS (AGV → Master) ==========
  std::string connection_topic;
  std::string state_topic;

  // ========== OPTIONAL TOPICS ==========
  // AGV → Master
  std::optional<std::string> factsheet_topic;
  std::optional<std::string> visualization_topic;

  // Master → AGV
  std::optional<std::string> order_topic;
  std::optional<std::string> instant_actions_topic;

  /**
   * @brief Validate that required fields are properly configured
   * @throws std::runtime_error if validation fails
   */
  void validate() const
  {
    if (connection_topic.empty())
    {
      throw std::runtime_error("Connection topic is required but empty");
    }
    if (state_topic.empty())
    {
      throw std::runtime_error("State topic is required but empty");
    }

    // Validate optional topics are not empty strings if provided
    if (factsheet_topic.has_value() && factsheet_topic->empty())
    {
      throw std::runtime_error("Factsheet topic provided but empty");
    }
    if (visualization_topic.has_value() && visualization_topic->empty())
    {
      throw std::runtime_error("Visualization topic provided but empty");
    }
    if (order_topic.has_value() && order_topic->empty())
    {
      throw std::runtime_error("Order topic provided but empty");
    }
    if (instant_actions_topic.has_value() && instant_actions_topic->empty())
    {
      throw std::runtime_error("Instant actions topic provided but empty");
    }
  }

  /**
   * @brief Helper methods to check if optional topics are configured
   */
  bool has_factsheet() const
  {
    return factsheet_topic.has_value();
  }
  bool has_visualization() const
  {
    return visualization_topic.has_value();
  }
  bool has_order() const
  {
    return order_topic.has_value();
  }
  bool has_instant_actions() const
  {
    return instant_actions_topic.has_value();
  }
};

#endif  // VDA5050_MASTER__COMMUNICATION__VDA5050_TOPIC_CONFIG_HPP_
