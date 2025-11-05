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

#ifndef VDA5050_JSON_UTILS__TRAITS_HPP_
#define VDA5050_JSON_UTILS__TRAITS_HPP_

#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <vda5050_types/connection.hpp>
#include <vda5050_types/connection_state.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/operating_mode.hpp>

#ifdef ENABLE_ROS2
#include <rosidl_runtime_cpp/bounded_vector.hpp>
#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/state.hpp>
#endif  // ENABLE_ROS2

namespace vda5050_json_utils {

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

//=============================================================================
inline std::string to_iso8601(TimePoint time_point)
{
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;
  using std::chrono::system_clock;

  std::time_t time_sec = system_clock::to_time_t(time_point);
  auto duration = time_point.time_since_epoch();
  auto millisec = duration_cast<milliseconds>(duration).count() % 1000;

  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&time_sec), vda5050_types::ISO8601_FORMAT);
  oss << "." << std::setw(3) << std::setfill('0') << millisec << "Z";
  if (oss.fail())
  {
    throw std::runtime_error("Failed to format timestamp for serialization.");
  }
  return oss.str();
}

//=============================================================================
inline TimePoint from_iso8601(const std::string& time_string)
{
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;
  using std::chrono::system_clock;

  std::tm t = {};
  char sep;
  int millisec = 0;

  std::istringstream ss(time_string);
  ss >> std::get_time(&t, vda5050_types::ISO8601_FORMAT);

  ss >> sep;
  if (ss.fail() || sep != '.')
  {
    throw std::runtime_error(
      "JSON parsing error for Header: Unexpected character after seconds in "
      "timestamp.");
  }
  else
  {
    ss >> millisec;
    if (ss.fail())
    {
      throw std::runtime_error(
        "JSON parsing error for Header: Failed to parse milliseconds from "
        "timestamp.");
    }

    if (!ss.eof())
    {
      ss.ignore(std::numeric_limits<std::streamsize>::max(), 'Z');
    }
    else
    {
      throw std::runtime_error(
        "JSON parsing error for Header: Expected 'Z' at the end of timestamp "
        "to indicate UTC.");
    }
  }

  // TODO(sauk): Add a check to see if the platform supports timegm
  auto tp = system_clock::from_time_t(timegm(&t));

  return tp + milliseconds(millisec);
}

//=============================================================================
template <typename T>
struct timestamp_traits;

//=============================================================================
template <>
struct timestamp_traits<TimePoint>
{
  static std::string to_string(const TimePoint& time_point)
  {
    return to_iso8601(time_point);
  }

  static TimePoint from_string(const std::string& time_string)
  {
    return from_iso8601(time_string);
  }
};

//=============================================================================
template <>
struct timestamp_traits<int64_t>
{
  static std::string to_string(const int64_t& millisec)
  {
    using std::chrono::milliseconds;

    TimePoint time_point{milliseconds(millisec)};
    return to_iso8601(time_point);
  }

  static int64_t from_string(const std::string& time_string)
  {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    auto time_point = from_iso8601(time_string);
    return duration_cast<milliseconds>(time_point.time_since_epoch()).count();
  }
};

//=============================================================================
template <typename T>
struct connection_state_traits;

//=============================================================================
template <>
struct connection_state_traits<vda5050_types::ConnectionState>
{
  static std::string to_string(const vda5050_types::ConnectionState& state)
  {
    using vda5050_types::ConnectionState;

    switch (state)
    {
      case ConnectionState::ONLINE:
        return "ONLINE";
      case ConnectionState::OFFLINE:
        return "OFFLINE";
      case ConnectionState::CONNECTIONBROKEN:
        return "CONNECTIONBROKEN";
      default:
        throw std::runtime_error("Invalid ConnectionState enum value");
    }
  }

  static vda5050_types::ConnectionState from_string(const std::string& state)
  {
    using vda5050_types::ConnectionState;

    if (state == "ONLINE") return ConnectionState::ONLINE;
    if (state == "OFFLINE") return ConnectionState::OFFLINE;
    if (state == "CONNECTIONBROKEN") return ConnectionState::CONNECTIONBROKEN;
    throw std::runtime_error("Invalid connectionState string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct connection_state_traits<std::string>
{
  static std::string to_string(const std::string& state)
  {
    using vda5050_msgs::msg::Connection;

    if (
      state == Connection::ONLINE || state == Connection::OFFLINE ||
      state == Connection::CONNECTIONBROKEN)
    {
      return state;
    }
    throw std::runtime_error("Invalid connection_state value");
  }

  static std::string from_string(const std::string& state)
  {
    using vda5050_msgs::msg::Connection;

    if (
      state == Connection::ONLINE || state == Connection::OFFLINE ||
      state == Connection::CONNECTIONBROKEN)
    {
      return state;
    }
    throw std::runtime_error("Invalid connectionState string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct operating_mode_traits;

//=============================================================================
template <>
struct operating_mode_traits<vda5050_types::OperatingMode>
{
  static std::string to_string(const vda5050_types::OperatingMode& mode)
  {
    using vda5050_types::OperatingMode;

    switch (mode)
    {
      case OperatingMode::AUTOMATIC:
        return "AUTOMATIC";
      case OperatingMode::SEMIAUTOMATIC:
        return "SEMIAUTOMATIC";
      case OperatingMode::MANUAL:
        return "MANUAL";
      case OperatingMode::SERVICE:
        return "SERVICE";
      case OperatingMode::TEACHIN:
        return "TEACHIN";
      default:
        throw std::runtime_error("Invalid OperatingMode enum value");
    }
  }

  static vda5050_types::OperatingMode from_string(const std::string& mode)
  {
    using vda5050_types::OperatingMode;

    if (mode == "AUTOMATIC") return OperatingMode::AUTOMATIC;
    if (mode == "SEMIAUTOMATIC") return OperatingMode::SEMIAUTOMATIC;
    if (mode == "MANUAL") return OperatingMode::MANUAL;
    if (mode == "SERVICE") return OperatingMode::SERVICE;
    if (mode == "TEACHIN") return OperatingMode::TEACHIN;
    throw std::runtime_error("Invalid operatingMode string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct operating_mode_traits<std::string>
{
  static std::string to_string(const std::string& mode)
  {
    using vda5050_msgs::msg::State;

    if (
      mode == State::OPERATING_MODE_AUTOMATIC ||
      mode == State::OPERATING_MODE_SEMIAUTOMATIC ||
      mode == State::OPERATING_MODE_MANUAL ||
      mode == State::OPERATING_MODE_SERVICE ||
      mode == State::OPERATING_MODE_TEACHIN)
    {
      return mode;
    }
    throw std::runtime_error("Invalid operating_mode value");
  }

  static std::string from_string(const std::string& mode)
  {
    using vda5050_msgs::msg::State;

    if (
      mode == State::OPERATING_MODE_AUTOMATIC ||
      mode == State::OPERATING_MODE_SEMIAUTOMATIC ||
      mode == State::OPERATING_MODE_MANUAL ||
      mode == State::OPERATING_MODE_SERVICE ||
      mode == State::OPERATING_MODE_TEACHIN)
    {
      return mode;
    }
    throw std::runtime_error("Invalid operatingMode string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct optional_field_traits;

//=============================================================================
template <typename T>
struct optional_field_traits<std::optional<T>>
{
  static bool has_value(const std::optional<T>& opt)
  {
    return opt.has_value();
  }

  static const T& get(const std::optional<T>& opt)
  {
    return opt.value();
  }

  static void set(std::optional<T>& opt, T&& val)
  {
    opt = std::move(val);
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <typename T, std::size_t Max, typename Alloc>
struct optional_field_traits<rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>>
{
  static bool has_value(
    const rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>& opt)
  {
    return !opt.empty();
  }

  static const T& get(
    const rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>& opt)
  {
    return opt.front();
  }

  static void set(
    rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>& opt, T&& val)
  {
    opt.clear();
    opt.push_back(std::move(val));
  }
};
#endif  // ENABLE_ROS2

}  // namespace vda5050_json_utils

#endif  // VDA5050_JSON_UTILS__TRAITS_HPP_
