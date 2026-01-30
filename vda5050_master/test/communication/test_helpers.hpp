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

#ifndef COMMUNICATION__TEST_HELPERS_HPP_
#define COMMUNICATION__TEST_HELPERS_HPP_

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "nlohmann/json.hpp"

namespace vda5050_master::test {

// Test constants
namespace constants {
constexpr auto DEFAULT_TIMEOUT = std::chrono::seconds(3);
constexpr auto DISCOVERY_TIMEOUT = std::chrono::seconds(2);
constexpr auto DISCOVERY_POLL_INTERVAL = std::chrono::milliseconds(10);
const char TEST_TOPIC_PREFIX[] = "/test/integration";

inline nlohmann::json default_payload_json()
{
  return R"(
    {
      "happy": true,
      "pi": 3.141
    }
  )"_json;
}
}  // namespace constants
/**
 * @brief Generic polling helper with timeout
 * @param condition Lambda that returns true when condition is met
 * @param timeout Maximum time to wait
 * @param poll_interval How often to check the condition
 * @return true if condition was met, false if timeout occurred
 */
template <typename Condition>
bool wait_for_condition(
  Condition condition, std::chrono::milliseconds poll_interval,
  std::chrono::milliseconds timeout = constants::DEFAULT_TIMEOUT)
{
  auto start = std::chrono::steady_clock::now();
  while (!condition() && (std::chrono::steady_clock::now() - start) < timeout)
  {
    std::this_thread::sleep_for(poll_interval);
  }
  return condition();
}

/**
 * @brief Helper to verify multiple messages in order
 * @param get_message_func Function to retrieve next message
 * @param expected_payloads List of expected payloads in order
 * @param check_size_func Function to check current message count
 */
template <typename GetMessageFunc, typename CheckSizeFunc>
void verify_messages_in_order(
  GetMessageFunc get_message_func,
  const std::vector<std::string>& expected_payloads,
  CheckSizeFunc check_size_func)
{
  size_t expected_count = expected_payloads.size();
  for (size_t i = 0; i < expected_payloads.size(); ++i)
  {
    check_size_func(expected_count);
    auto received = get_message_func();
    if (received != expected_payloads[i])
    {
      throw std::runtime_error(
        "Message " + std::to_string(i) +
        " mismatch. Expected: " + expected_payloads[i] + ", Got: " + received);
    }
    expected_count--;
  }
  check_size_func(0);  // Should be empty after all reads
}

/**
 * @brief Generate test topic name with suffix
 */
inline std::string make_test_topic(const std::string& suffix)
{
  return std::string(constants::TEST_TOPIC_PREFIX) + "/" + suffix;
}

/**
 * @brief Generate multiple JSON payloads with IDs
 */
inline std::vector<std::string> make_numbered_payloads(size_t count)
{
  std::vector<std::string> payloads;
  payloads.reserve(count);
  for (size_t i = 1; i <= count; ++i)
  {
    nlohmann::json j;
    j["id"] = i;
    payloads.push_back(j.dump());
  }
  return payloads;
}

/**
 * @brief Create a VDA5050 connection JSON message
 * @param manufacturer AGV manufacturer
 * @param serial_number AGV serial number
 * @param state Connection state: "ONLINE", "OFFLINE", or "CONNECTIONBROKEN"
 */
inline std::string make_connection_json(
  const std::string& manufacturer, const std::string& serial_number,
  const std::string& state = "ONLINE")
{
  return R"({"headerId": 1, "timestamp": "2025-01-01T00:00:00.000Z", "version": "2.0.0", "manufacturer": ")" +
         manufacturer + R"(", "serialNumber": ")" + serial_number +
         R"(", "connectionState": ")" + state + R"("})";
}

/**
 * @brief Build the VDA5050 connection topic path for an AGV
 * @param manufacturer AGV manufacturer
 * @param serial_number AGV serial number
 */
inline std::string make_connection_topic(
  const std::string& manufacturer, const std::string& serial_number)
{
  return "rmf2/v2/" + manufacturer + "/" + serial_number + "/connection";
}

/**
 * @brief Create a VDA5050 state JSON message
 * @param manufacturer AGV manufacturer
 * @param serial_number AGV serial number
 */
inline std::string make_state_json(
  const std::string& manufacturer, const std::string& serial_number)
{
  return R"({"headerId": 1, "timestamp": "2025-01-01T00:00:00.000Z", "version": "2.0.0", "manufacturer": ")" +
         manufacturer + R"(", "serialNumber": ")" + serial_number +
         R"(", "orderId": "", "orderUpdateId": 0, "driving": false, "paused": false, "newBaseRequest": false, "distanceSinceLastNode": 0.0, "nodeStates": [], "edgeStates": [], "actionStates": [], "batteryState": {"batteryCharge": 100.0, "charging": false}, "errors": [], "safetyState": {"eStop": "NONE", "fieldViolation": false}, "operatingMode": "AUTOMATIC"})";
}

/**
 * @brief Build the VDA5050 state topic path for an AGV
 * @param manufacturer AGV manufacturer
 * @param serial_number AGV serial number
 */
inline std::string make_state_topic(
  const std::string& manufacturer, const std::string& serial_number)
{
  return "rmf2/v2/" + manufacturer + "/" + serial_number + "/state";
}

}  // namespace vda5050_master::test

#endif  // COMMUNICATION__TEST_HELPERS_HPP_
