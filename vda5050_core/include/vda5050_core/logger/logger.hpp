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

#ifndef VDA5050_CORE__LOGGER__LOGGER_HPP_
#define VDA5050_CORE__LOGGER__LOGGER_HPP_

#include <fmt/core.h>
#include <memory>
#include <sstream>
#include <string>

namespace vda5050_core {

namespace logger {

/// \brief Enum for logging level
enum class LogLevel
{
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL,
};

class LogHandler
{
public:
  virtual ~LogHandler() = default;

  virtual void log(LogLevel level, const std::string& message) = 0;
};

/// \brief Set a handler to receive logs
///
/// \param handler Callback to receive logs
void set_handler(std::unique_ptr<LogHandler> handler);

void release_handler();

void set_log_level(LogLevel level);

void log(LogLevel level, const std::string& message);

}  // namespace logger
}  // namespace vda5050_core

#define VDA5050_LOG(level, ...) \
  vda5050_core::logger::log(level, fmt::format(__VA_ARGS__))

#define VDA5050_LOG_STREAM(level, stream)       \
  do {                                          \
    std::stringstream ss;                       \
    ss << stream;                               \
    vda5050_core::logger::log(level, ss.str()); \
  } while (0)

#define VDA5050_DEBUG(...) \
  VDA5050_LOG(vda5050_core::logger::LogLevel::DEBUG, __VA_ARGS__)

#define VDA5050_INFO(...) \
  VDA5050_LOG(vda5050_core::logger::LogLevel::INFO, __VA_ARGS__)

#define VDA5050_WARN(...) \
  VDA5050_LOG(vda5050_core::logger::LogLevel::WARNING, __VA_ARGS__)

#define VDA5050_ERROR(...) \
  VDA5050_LOG(vda5050_core::logger::LogLevel::ERROR, __VA_ARGS__)

#define VDA5050_FATAL(...) \
  VDA5050_LOG(vda5050_core::logger::LogLevel::FATAL, __VA_ARGS__)

#define VDA5050_DEBUG_STREAM(arg) \
  VDA5050_LOG_STREAM(vda5050_core::logger::LogLevel::DEBUG, arg)

#define VDA5050_INFO_STREAM(arg) \
  VDA5050_LOG_STREAM(vda5050_core::logger::LogLevel::INFO, arg)

#define VDA5050_WARN_STREAM(arg) \
  VDA5050_LOG_STREAM(vda5050_core::logger::LogLevel::WARN, arg)

#define VDA5050_ERROR_STREAM(arg) \
  VDA5050_LOG_STREAM(vda5050_core::logger::LogLevel::ERROR, arg)

#define VDA5050_FATAL_STREAM(arg) \
  VDA5050_LOG_STREAM(vda5050_core::logger::LogLevel::FATAL, arg)

#endif  // VDA5050_CORE__LOGGER__LOGGER_HPP_
