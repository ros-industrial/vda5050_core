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

#ifndef VDA5050_JSON_UTILS__TEST__GENERATOR__GENERATOR_HPP_
#define VDA5050_JSON_UTILS__TEST__GENERATOR__GENERATOR_HPP_

#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <vda5050_types/action_state.hpp>
#include <vda5050_types/action_status.hpp>
#include <vda5050_types/battery_state.hpp>
#include <vda5050_types/connection.hpp>
#include <vda5050_types/connection_state.hpp>
#include <vda5050_types/control_point.hpp>
#include <vda5050_types/e_stop.hpp>
#include <vda5050_types/edge_state.hpp>
#include <vda5050_types/error.hpp>
#include <vda5050_types/error_level.hpp>
#include <vda5050_types/error_reference.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/node_position.hpp>
#include <vda5050_types/node_state.hpp>
#include <vda5050_types/operating_mode.hpp>
#include <vda5050_types/safety_state.hpp>
#include <vda5050_types/state.hpp>
#include <vda5050_types/trajectory.hpp>

using vda5050_types::ActionState;
using vda5050_types::ActionStatus;
using vda5050_types::BatteryState;
using vda5050_types::Connection;
using vda5050_types::ConnectionState;
using vda5050_types::ControlPoint;
using vda5050_types::EdgeState;
using vda5050_types::Error;
using vda5050_types::ErrorLevel;
using vda5050_types::ErrorReference;
using vda5050_types::EStop;
using vda5050_types::Header;
using vda5050_types::NodePosition;
using vda5050_types::NodeState;
using vda5050_types::OperatingMode;
using vda5050_types::SafetyState;
using vda5050_types::State;
using vda5050_types::Trajectory;

/// \brief Utility class to generate random instances of VDA 5050 message types
class RandomDataGenerator
{
public:
  /// \brief Default constructor using a non-deterministic seed
  RandomDataGenerator()
  : rng_(std::random_device()()),
    uint_dist_(0, std::numeric_limits<uint32_t>::max()),
    int_dist_(0, 100),
    float_dist_(
      std::numeric_limits<double>::min(), std::numeric_limits<double>::max()),
    bool_dist_(0, 1),
    string_length_dist_(0, 50),
    size_dist_(0, 10),
    percentage_dist_(0, 100)
  {
    // Nothing to do
  }

  /// \brief Constructor with a fixed seed for deterministic results
  explicit RandomDataGenerator(uint32_t seed)
  : rng_(seed),
    uint_dist_(0, std::numeric_limits<uint32_t>::max()),
    int_dist_(0, 100),
    float_dist_(
      std::numeric_limits<double>::min(), std::numeric_limits<double>::max()),
    bool_dist_(0, 1),
    string_length_dist_(0, 50),
    size_dist_(0, 10),
    percentage_dist_(0, 100)
  {
    // Nothing to do
  }

  /// \brief Generate a random unsigned 32-bit integer
  uint32_t generate_random_uint()
  {
    return uint_dist_(rng_);
  }

  /// \brief Generate a random signed 8-bit integer
  int8_t generate_random_int()
  {
    return int_dist_(rng_);
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

  /// \brief Generate a random value for vector length
  uint8_t generate_random_size()
  {
    return size_dist_(rng_);
  }

  /// \brief Generate a random 64-bit floating-point number
  double generate_random_float()
  {
    return float_dist_(rng_);
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

  /// \brief Generate current timestamp
  std::chrono::time_point<std::chrono::system_clock> generate_timestamp()
  {
    return std::chrono::system_clock::now();
  }

  /// \brief Generate a random percentage value
  double generate_random_percentage()
  {
    return percentage_dist_(rng_);
  }

  /// \brief Generate a random connectionState value
  ConnectionState generate_random_connection_state()
  {
    std::vector<ConnectionState> states = {
      ConnectionState::ONLINE, ConnectionState::OFFLINE,
      ConnectionState::CONNECTIONBROKEN};
    auto state_idx = generate_random_index(states.size());
    return states[state_idx];
  }

  /// \brief Generate a random operatingMode value
  OperatingMode generate_random_operating_mode()
  {
    std::vector<OperatingMode> mode = {
      OperatingMode::AUTOMATIC, OperatingMode::SEMIAUTOMATIC,
      OperatingMode::MANUAL, OperatingMode::SERVICE, OperatingMode::TEACHIN};
    auto mode_idx = generate_random_index(mode.size());
    return mode[mode_idx];
  }

  /// \brief Generate a random actionStatus value
  ActionStatus generate_action_status()
  {
    std::vector<ActionStatus> statuses = {
      ActionStatus::WAITING, ActionStatus::INITIALIZING, ActionStatus::RUNNING,
      ActionStatus::PAUSED, ActionStatus::FINISHED};
    auto status_idx = generate_random_index(statuses.size());
    return statuses[status_idx];
  }

  /// \brief Generate a random errorLevel value
  ErrorLevel generate_random_error_level()
  {
    std::vector<ErrorLevel> levels = {ErrorLevel::WARNING, ErrorLevel::FATAL};
    auto level_idx = generate_random_index(levels.size());
    return levels[level_idx];
  }

  /// \brief Generate a random eStop value
  vda5050_types::EStop generate_random_e_stop()
  {
    std::vector<vda5050_types::EStop> types = {
      EStop::AUTOACK, EStop::MANUAL, EStop::REMOTE, EStop::NONE};
    auto type_idx = generate_random_index(types.size());
    return types[type_idx];
  }

  /// \brief Generate a random vector of type float64
  std::vector<double> generate_random_float_vector(const uint8_t size)
  {
    std::vector<double> vec(size);
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
      *it = generate_random_float();
    }
    return vec;
  }

  /// \brief Generate a random vector of type T
  template <typename T>
  std::vector<T> generate_random_vector(const uint8_t size)
  {
    std::vector<T> vec(size);
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
      *it = generate<T>();
    }
    return vec;
  }

  /// \brief Generate a fully populated message of a supported type
  template <typename T>
  T generate()
  {
    T msg;
    if constexpr (std::is_same_v<T, ActionState>)
    {
      msg.action_id = generate_random_string();
      msg.action_type = generate_random_string();
      msg.action_description = generate_random_string();
      msg.action_status = generate_action_status();
      msg.result_description = generate_random_string();
    }
    else if constexpr (std::is_same_v<T, BatteryState>)
    {
      msg.battery_charge = generate_random_percentage();
      msg.battery_voltage = generate_random_float();
      msg.battery_health = generate_random_int();
      msg.charging = generate_random_bool();
      msg.reach = generate_random_uint();
    }
    else if constexpr (std::is_same_v<T, Connection>)
    {
      msg.header = generate<Header>();
      msg.connection_state = generate_random_connection_state();
    }
    else if constexpr (std::is_same_v<T, ControlPoint>)
    {
      msg.x = generate_random_float();
      msg.y = generate_random_float();
      msg.weight = 1.0;
    }
    else if constexpr (std::is_same_v<T, EdgeState>)
    {
      msg.edge_id = generate_random_string();
      msg.sequence_id = generate_random_uint();
      msg.edge_description = generate_random_string();
      msg.released = generate_random_bool();
      msg.trajectory = generate<Trajectory>();
    }
    else if constexpr (std::is_same_v<T, Error>)
    {
      msg.error_type = generate_random_string();
      msg.error_references = generate_random_vector<ErrorReference>(10);
      msg.error_description = generate_random_string();
      msg.error_level = generate_random_error_level();
    }
    else if constexpr (std::is_same_v<T, ErrorReference>)
    {
      msg.reference_key = generate_random_string();
      msg.reference_value = generate_random_string();
    }
    else if constexpr (std::is_same_v<T, Header>)
    {
      msg.header_id = generate_random_uint();
      msg.timestamp = generate_timestamp();
      msg.version = "2.0.0";  // Fix the VDA 5050 version to 2.0.0
      msg.manufacturer = generate_random_string();
      msg.serial_number = generate_random_string();
    }
    else if constexpr (std::is_same_v<T, NodePosition>)
    {
      msg.x = generate_random_float();
      msg.y = generate_random_float();
      msg.theta = generate_random_float();
      msg.allowed_deviation_x_y = generate_random_float();
      msg.allowed_deviation_theta = generate_random_float();
      msg.map_id = generate_random_string();
      msg.map_description = generate_random_string();
    }
    else if constexpr (std::is_same_v<T, NodeState>)
    {
      msg.node_id = generate_random_string();
      msg.sequence_id = generate_random_uint();
      msg.node_description = generate_random_string();
      msg.node_position = generate<NodePosition>();
      msg.released = generate_random_bool();
    }
    else if constexpr (std::is_same_v<T, SafetyState>)
    {
      msg.e_stop = generate_random_e_stop();
      msg.field_violation = generate_random_bool();
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
      msg.node_states =
        generate_random_vector<NodeState>(generate_random_size());
      msg.edge_states =
        generate_random_vector<EdgeState>(generate_random_size());
      // msg.agv_position.push_back(generate<AGVPosition>());
      // msg.velocity.push_back(generate<Velocity>());
      // msg.loads = generate_random_vector<Load>(generate_random_size());
      msg.action_states =
        generate_random_vector<ActionState>(generate_random_size());
      msg.battery_state = generate<BatteryState>();
      msg.errors = generate_random_vector<Error>(generate_random_size());
      // msg.information = generate_random_vector<Info>(generate_random_size());
      msg.safety_state = generate<SafetyState>();
    }
    else if constexpr (std::is_same_v<T, Trajectory>)
    {
      msg.knot_vector = generate_random_float_vector(generate_random_size());
      msg.control_points =
        generate_random_vector<ControlPoint>(generate_random_size());
      msg.degree = 1.0;
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

  /// \brief Distribution for signed 8-bit integers
  std::uniform_int_distribution<int8_t> int_dist_;

  /// \brief Distribution for 64-bit floating-point numbers
  std::uniform_real_distribution<double> float_dist_;

  /// \brief Distribution for a boolean value
  std::uniform_int_distribution<int> bool_dist_;

  /// \brief Distribution for random string lengths
  std::uniform_int_distribution<int> string_length_dist_;

  /// \brief Distribution for random vector size
  std::uniform_int_distribution<uint8_t> size_dist_;

  /// \brief Distribution for random percentage values
  std::uniform_real_distribution<double> percentage_dist_;
};

#endif  // VDA5050_JSON_UTILS__TEST__GENERATOR__GENERATOR_HPP_
