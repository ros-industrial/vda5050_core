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

#ifndef VDA5050_MASTER__COMMUNICATION__VDA5050_HANDLERS_HPP_
#define VDA5050_MASTER__COMMUNICATION__VDA5050_HANDLERS_HPP_

#include <functional>
#include <stdexcept>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_master/vda5050_interfaces.hpp"

/**
 * @brief Callback handlers for VDA5050 messages
 *
 * This struct holds protocol-agnostic callbacks that are invoked when
 * VDA5050 messages are received. The callbacks receive typed messages,
 * completely abstracted from the underlying transport (MQTT, ROS2, etc.).
 *
 * Required handlers (on_connection, on_state) must be set by the user.
 * Optional handlers (on_factsheet, on_visualization) have default implementations
 * that print received messages, and can be overridden.
 *
 * Thread safety note: Callbacks are invoked on the communication thread.
 * If thread safety is required, the callback implementation should handle
 * synchronization (e.g., using a mutex).
 */
struct VDA5050Handlers
{
  // ========== REQUIRED HANDLERS (must be set by user) ==========
  std::function<void(const vda5050_msgs::msg::Connection&)> on_connection;
  std::function<void(const vda5050_msgs::msg::State&)> on_state;

  // ========== OPTIONAL HANDLERS (have defaults, can be overridden) ==========
  std::function<void(const vda5050_msgs::msg::Factsheet&)> on_factsheet =
    default_factsheet_handler;
  std::function<void(const vda5050_msgs::msg::Visualization&)>
    on_visualization = default_visualization_handler;

  /**
   * @brief Validate that all handlers are properly configured
   * @throws std::runtime_error if validation fails
   */
  void validate() const
  {
    if (!on_connection)
    {
      throw std::runtime_error("on_connection handler is required but not set");
    }
    if (!on_state)
    {
      throw std::runtime_error("on_state handler is required but not set");
    }
    if (!on_factsheet)
    {
      throw std::runtime_error("on_factsheet handler is not set");
    }
    if (!on_visualization)
    {
      throw std::runtime_error("on_visualization handler is not set");
    }
  }

  // ========== DEFAULT HANDLER IMPLEMENTATIONS ==========
  static void default_factsheet_handler(const vda5050_msgs::msg::Factsheet& msg)
  {
    VDA5050_WARN(
      "on_factsheet handler not overridden. Received factsheet from: {}/{}",
      msg.header.manufacturer, msg.header.serial_number);
  }

  static void default_visualization_handler(
    const vda5050_msgs::msg::Visualization& msg)
  {
    VDA5050_WARN(
      "on_visualization handler not overridden. Received visualization from: "
      "{}/{}",
      msg.header.manufacturer, msg.header.serial_number);
  }
};

#endif  // VDA5050_MASTER__COMMUNICATION__VDA5050_HANDLERS_HPP_
