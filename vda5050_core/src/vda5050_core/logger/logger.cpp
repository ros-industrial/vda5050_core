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

#include "vda5050_core/logger/Logger.hpp"

#include <iostream>

namespace vda5050_core {

namespace logger {

//=============================================================================
static LogHandler log_handler = [](LogLevel level, const std::string& message) {
  switch (level)
  {
    case LogLevel::INFO:
      std::cout << "[INFO]: " << message << std::endl;
      break;
    case LogLevel::DEBUG:
      std::cout << "[DEBUG]: " << message << std::endl;
      break;
    case LogLevel::WARNING:
      std::cout << "[WARNING]: " << message << std::endl;
      break;
    case LogLevel::ERROR:
      std::cerr << "[ERROR]: " << message << std::endl;
      break;
  }
};

//=============================================================================
void set_handler(LogHandler handler)
{
  log_handler = handler;
}

//=============================================================================
void info(const std::string& message)
{
  log_handler(LogLevel::INFO, message);
}

//=============================================================================
void debug(const std::string& message)
{
  log_handler(LogLevel::DEBUG, message);
}

//=============================================================================
void warning(const std::string& message)
{
  log_handler(LogLevel::WARNING, message);
}

//=============================================================================
void error(const std::string& message)
{
  log_handler(LogLevel::ERROR, message);
}

}  // namespace logger
}  // namespace vda5050_core
