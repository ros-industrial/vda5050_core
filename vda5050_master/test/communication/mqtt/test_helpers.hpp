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

#ifndef COMMUNICATION__MQTT__TEST_HELPERS_HPP_
#define COMMUNICATION__MQTT__TEST_HELPERS_HPP_

#include <chrono>
#include <cstdlib>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "nlohmann/json.hpp"

namespace vda5050_master::test::mqtt {

// Test constants
namespace constants {
constexpr auto MQTT_POLL_INTERVAL = std::chrono::milliseconds(50);

// Allow overriding MQTT broker via environment variable
// Default to localhost:1883 for local mosquitto broker
// Use localhost:1884 for automated Docker test broker by setting env var
// NOLINTNEXTLINE(runtime/string)
inline std::string get_mqtt_broker()
{
  const char* env_broker = std::getenv("VDA5050_TEST_MQTT_BROKER");
  if (env_broker != nullptr) {
    return std::string(env_broker);
  }
  return "tcp://localhost:1883";
}

// NOLINTNEXTLINE(runtime/string)
const std::string MQTT_BROKER = get_mqtt_broker();
}  // namespace constants
}  // namespace vda5050_master::test::mqtt

#endif  // COMMUNICATION__MQTT__TEST_HELPERS_HPP_
