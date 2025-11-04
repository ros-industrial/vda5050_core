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

#ifndef VDA5050_JSON_UTILS__TEST__GENERATOR__GENERATOR_ROS2_HPP_
#define VDA5050_JSON_UTILS__TEST__GENERATOR__GENERATOR_ROS2_HPP_

#include <limits>
#include <random>
#include <string>
#include <vector>

#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/header.hpp>
#include <vda5050_msgs/msg/state.hpp>

using vda5050_msgs::msg::Connection;
using vda5050_msgs::msg::Header;
using vda5050_msgs::msg::State;

/// \brief Utility class to generate random instances of VDA 5050 message types
class RandomDataGeneratorROS2
{
public:
  /// \brief Default constructor using a non-deterministic seed
  RandomDataGeneratorROS2()
  : rng_(std::random_device()()),
    uint_dist_(0, std::numeric_limits<uint32_t>::max()),
    bool_dist_(0, 1),
    string_length_dist_(0, 50),
    milliseconds_dist_(0, 4000000000000L)
  {
    // Nothing to do
  }

  /// \brief Constructor with a fixed seed for deterministic results
  explicit RandomDataGeneratorROS2(uint32_t seed)
  : rng_(seed),
    uint_dist_(0, std::numeric_limits<uint32_t>::max()),
    bool_dist_(0, 1),
    string_length_dist_(0, 50),
    milliseconds_dist_(0, 4000000000000L)
  {
    // Nothing to do
  }

  /// \brief Generate a random unsigned 32-bit integer
  uint32_t generate_random_uint()
  {
    return uint_dist_(rng_);
  }

  /// \brief Generate a random boolean value
  bool generate_random_bool()
  {
    return bool_dist_(rng_);
  }

  /// \brief Generate a random index for enum selection
  uint8_t generate_random_index(size_t size)
  {
    std::uniform_int_distribution<uint8_t> index_dist(0, size - 1);
    return index_dist(rng_);
  }

  /// \brief Generate a random alphanumerical string with length upto 50
  std::string generate_random_string()
  {
    int length = string_length_dist_(rng_);
    std::string s;
    s.reserve(length);

    const std::string charset =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::uniform_int_distribution<size_t> char_index_dist(
      0, charset.length() - 1);

    for (int i = 0; i < length; i++)
    {
      s += charset[char_index_dist(rng_)];
    }
    return s;
  }

  /// \brief Generate a random millisecond timestamp
  int64_t generate_milliseconds()
  {
    return milliseconds_dist_(rng_);
  }

  /// \brief Generate a random connectionState value
  std::string generate_random_connection_state()
  {
    std::vector<std::string> states = {
      Connection::ONLINE, Connection::OFFLINE, Connection::CONNECTIONBROKEN};
    auto state_idx = generate_random_index(states.size());
    return states[state_idx];
  }

  /// \brief Generate a random operatingMode value
  std::string generate_random_operating_mode()
  {
    std::vector<std::string> states = {
      State::OPERATING_MODE_AUTOMATIC, State::OPERATING_MODE_SEMIAUTOMATIC,
      State::OPERATING_MODE_MANUAL, State::OPERATING_MODE_SERVICE,
      State::OPERATING_MODE_TEACHIN};
    auto state_idx = generate_random_index(states.size());
    return states[state_idx];
  }

  /// \brief Generate a fully populated message of a supported type
  template <typename T>
  T generate()
  {
    T msg;
    if constexpr (std::is_same_v<T, Connection>)
    {
      msg.header = generate<Header>();
      msg.connection_state = generate_random_connection_state();
    }
    else if constexpr (std::is_same_v<T, Header>)
    {
      msg.header_id = generate_random_uint();
      msg.timestamp = generate_milliseconds();
      msg.version = "2.0.0";  // Fix the VDA 5050 version to 2.0.0
      msg.manufacturer = generate_random_string();
      msg.serial_number = generate_random_string();
    }
    else if constexpr (std::is_same_v<T, State>)
    {
      msg.header = generate<Header>();
      msg.order_id = generate_random_string();
      msg.order_update_id = generate_random_uint();
      // msg.zone_set_id.push_back(generate_random_string());
      msg.last_node_id = generate_random_string();
      msg.last_node_sequence_id = generate_random_uint();
      msg.driving = generate_random_bool();
      // msg.paused.push_back(generate_random_bool());
      // msg.new_base_request.push_back(generate_random_bool());
      // msg.distance_since_last_node.push_back(generate_random_float());
      msg.operating_mode = generate_random_operating_mode();
      // msg.node_states =
      //   generate_random_vector<NodeState>(generate_random_size());
      // msg.edge_states =
      //   generate_random_vector<EdgeState>(generate_random_size());
      // msg.agv_position.push_back(generate<AGVPosition>());
      // msg.velocity.push_back(generate<Velocity>());
      // msg.loads = generate_random_vector<Load>(generate_random_size());
      // msg.action_states =
      //   generate_random_vector<ActionState>(generate_random_size());
      // msg.battery_state = generate<BatteryState>();
      // msg.errors = generate_random_vector<Error>(generate_random_size());
      // msg.information = generate_random_vector<Info>(generate_random_size());
      // msg.safety_state = generate<SafetyState>();
    }
    else
    {
      throw std::runtime_error(
        "No random data generator defined for this custom type: " +
        std::string(typeid(T).name()));
    }
    return msg;
  }

private:
  /// \brief Mersenne Twister random number engine
  std::mt19937 rng_;

  /// \brief Distribution for unsigned 32-bit integers
  std::uniform_int_distribution<uint32_t> uint_dist_;

  /// \brief Distribution for a boolean value
  std::uniform_int_distribution<int> bool_dist_;

  /// \brief Distribution for random string lengths
  std::uniform_int_distribution<int> string_length_dist_;

  /// \brief Distribution for milliseconds from epoch
  std::uniform_int_distribution<int64_t> milliseconds_dist_;
};

#endif  // VDA5050_JSON_UTILS__TEST__GENERATOR__GENERATOR_ROS2_HPP_
