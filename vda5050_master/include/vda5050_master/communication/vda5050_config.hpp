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

#ifndef VDA5050_MASTER__COMMUNICATION__VDA5050_CONFIG_HPP_
#define VDA5050_MASTER__COMMUNICATION__VDA5050_CONFIG_HPP_

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "vda5050_master/vda5050_interfaces.hpp"

/**
 * @brief Configuration struct for VDA5050 communication channels
 *
 * This struct holds all 6 standard VDA5050 message types as defined in the spec.
 * Use std::optional for channels that may not be needed (e.g., visualization).
 *
 * Connection and State are mandatory per VDA5050 spec.
 */
struct VDA5050Config
{
  // ========== REQUIRED CHANNELS ==========

  // Connection (from AGV to Master)
  std::string connection_topic;
  std::shared_ptr<std::vector<vda5050_msgs::msg::Connection>>
    connection_container;

  // State (from AGV to Master)
  std::string state_topic;
  std::shared_ptr<std::vector<vda5050_msgs::msg::State>> state_container;

  // ========== OPTIONAL CHANNELS ==========

  // Factsheet (from AGV to Master)
  std::optional<std::string> factsheet_topic;
  std::shared_ptr<std::vector<vda5050_msgs::msg::Factsheet>>
    factsheet_container;

  // Order (from Master to AGV)
  std::optional<std::string> order_topic;
  std::shared_ptr<std::vector<vda5050_msgs::msg::Order>> order_container;

  // Instant Actions (from Master to AGV)
  std::optional<std::string> instant_actions_topic;
  std::shared_ptr<std::vector<vda5050_msgs::msg::InstantActions>>
    instant_actions_container;

  // Visualization (from AGV to Master)
  std::optional<std::string> visualization_topic;
  std::shared_ptr<std::vector<vda5050_msgs::msg::Visualization>>
    visualization_container;

  /**
   * @brief Validate that required fields are properly configured
   * @throws std::runtime_error if validation fails
   */
  void validate() const
  {
    // Check required topics are not empty
    if (connection_topic.empty())
    {
      throw std::runtime_error("Connection topic is required but empty");
    }
    if (state_topic.empty())
    {
      throw std::runtime_error("State topic is required but empty");
    }

    // Check required containers are not null
    if (!connection_container)
    {
      throw std::runtime_error("Connection container is required but null");
    }
    if (!state_container)
    {
      throw std::runtime_error("State container is required but null");
    }

    // If optional topics are provided, check their containers are valid
    if (factsheet_topic.has_value() && factsheet_topic->empty())
    {
      throw std::runtime_error("Factsheet topic provided but empty");
    }
    if (factsheet_topic.has_value() && !factsheet_container)
    {
      throw std::runtime_error(
        "Factsheet topic provided but container is null");
    }

    if (order_topic.has_value() && order_topic->empty())
    {
      throw std::runtime_error("Order topic provided but empty");
    }
    if (order_topic.has_value() && !order_container)
    {
      throw std::runtime_error("Order topic provided but container is null");
    }

    if (instant_actions_topic.has_value() && instant_actions_topic->empty())
    {
      throw std::runtime_error("Instant actions topic provided but empty");
    }
    if (instant_actions_topic.has_value() && !instant_actions_container)
    {
      throw std::runtime_error(
        "Instant actions topic provided but container is null");
    }

    if (visualization_topic.has_value() && visualization_topic->empty())
    {
      throw std::runtime_error("Visualization topic provided but empty");
    }
    if (visualization_topic.has_value() && !visualization_container)
    {
      throw std::runtime_error(
        "Visualization topic provided but container is null");
    }
  }

  /**
   * @brief Helper to check if a channel is configured
   */
  bool has_factsheet() const
  {
    return factsheet_topic.has_value() && factsheet_container != nullptr;
  }

  bool has_order() const
  {
    return order_topic.has_value() && order_container != nullptr;
  }

  bool has_instant_actions() const
  {
    return instant_actions_topic.has_value() &&
           instant_actions_container != nullptr;
  }

  bool has_visualization() const
  {
    return visualization_topic.has_value() &&
           visualization_container != nullptr;
  }
};

#endif  // VDA5050_MASTER__COMMUNICATION__VDA5050_CONFIG_HPP_
