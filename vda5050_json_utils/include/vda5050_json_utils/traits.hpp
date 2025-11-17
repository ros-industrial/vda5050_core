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

#include <vda5050_types/action_status.hpp>
#include <vda5050_types/blocking_type.hpp>
#include <vda5050_types/connection.hpp>
#include <vda5050_types/connection_state.hpp>
#include <vda5050_types/e_stop.hpp>
#include <vda5050_types/error_level.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/info_level.hpp>
#include <vda5050_types/operating_mode.hpp>
#include <vda5050_types/orientation_type.hpp>

#ifdef ENABLE_ROS2
#include <rosidl_runtime_cpp/bounded_vector.hpp>
#include <vda5050_msgs/msg/action.hpp>
#include <vda5050_msgs/msg/action_state.hpp>
#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/edge.hpp>
#include <vda5050_msgs/msg/error.hpp>
#include <vda5050_msgs/msg/info.hpp>
#include <vda5050_msgs/msg/safety_state.hpp>
#include <vda5050_msgs/msg/state.hpp>
#endif  // ENABLE_ROS2

namespace vda5050_json_utils {

//=============================================================================
template <typename T>
struct optional_field_traits;

//=============================================================================
template <typename T>
struct optional_field_traits<std::optional<T>>
{
  using value_type = T;

  static bool has_value(const std::optional<T>& opt)
  {
    return opt.has_value();
  }

  static const T& get(const std::optional<T>& opt)
  {
    return opt.value();
  }

  template <typename U>
  static void set(std::optional<T>& opt, U&& val)
  {
    opt = std::forward<U>(val);
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <typename T, typename Alloc>
struct optional_field_traits<rosidl_runtime_cpp::BoundedVector<T, 1, Alloc>>
{
  using value_type = T;

  static bool has_value(
    const rosidl_runtime_cpp::BoundedVector<T, 1, Alloc>& opt)
  {
    return !opt.empty();
  }

  static const T get(const rosidl_runtime_cpp::BoundedVector<T, 1, Alloc>& opt)
  {
    return opt.front();
  }

  template <typename U>
  static void set(rosidl_runtime_cpp::BoundedVector<T, 1, Alloc>& opt, U&& val)
  {
    opt.clear();
    opt.push_back(std::forward<U>(val));
  }
};

template <typename T, std::size_t Max, typename Alloc>
struct optional_field_traits<rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>>
{
  using value_type = T;

  static bool has_value(
    const rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>& opt)
  {
    return !opt.empty();
  }

  static const rosidl_runtime_cpp::BoundedVector<T, Max, Alloc> get(
    const rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>& opt)
  {
    return opt;
  }

  template <typename U>
  static void set(
    rosidl_runtime_cpp::BoundedVector<T, Max, Alloc>& opt, U&& val)
  {
    opt = std::forward<U>(val);
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
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
struct action_status_traits;

//=============================================================================
template <>
struct action_status_traits<vda5050_types::ActionStatus>
{
  static std::string to_string(const vda5050_types::ActionStatus& status)
  {
    using vda5050_types::ActionStatus;

    switch (status)
    {
      case ActionStatus::WAITING:
        return "WAITING";
      case ActionStatus::INITIALIZING:
        return "INITIALIZING";
      case ActionStatus::RUNNING:
        return "RUNNING";
      case ActionStatus::PAUSED:
        return "PAUSED";
      case ActionStatus::FINISHED:
        return "FINISHED";
      case ActionStatus::FAILED:
        return "FAILED";
      default:
        throw std::runtime_error("Invalid ActionStatus enum value");
    }
  }

  static vda5050_types::ActionStatus from_string(const std::string& status)
  {
    using vda5050_types::ActionStatus;

    if (status == "WAITING") return ActionStatus::WAITING;
    if (status == "INITIALIZING") return ActionStatus::INITIALIZING;
    if (status == "RUNNING") return ActionStatus::RUNNING;
    if (status == "PAUSED") return ActionStatus::PAUSED;
    if (status == "FINISHED") return ActionStatus::FINISHED;
    if (status == "FAILED") return ActionStatus::FAILED;
    throw std::runtime_error("Invalid actionStatus string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct action_status_traits<std::string>
{
  static std::string to_string(const std::string& status)
  {
    using vda5050_msgs::msg::ActionState;

    if (
      status == ActionState::ACTION_STATUS_WAITING ||
      status == ActionState::ACTION_STATUS_INITIALIZING ||
      status == ActionState::ACTION_STATUS_RUNNING ||
      status == ActionState::ACTION_STATUS_PAUSED ||
      status == ActionState::ACTION_STATUS_FINISHED ||
      status == ActionState::ACTION_STATUS_FAILED)
    {
      return status;
    }
    throw std::runtime_error("Invalid action_status value");
  }

  static std::string from_string(const std::string& status)
  {
    using vda5050_msgs::msg::ActionState;

    if (
      status == ActionState::ACTION_STATUS_WAITING ||
      status == ActionState::ACTION_STATUS_INITIALIZING ||
      status == ActionState::ACTION_STATUS_RUNNING ||
      status == ActionState::ACTION_STATUS_PAUSED ||
      status == ActionState::ACTION_STATUS_FINISHED ||
      status == ActionState::ACTION_STATUS_FAILED)
    {
      return status;
    }
    throw std::runtime_error("Invalid actionStatus string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct error_level_traits;

//=============================================================================
template <>
struct error_level_traits<vda5050_types::ErrorLevel>
{
  static std::string to_string(const vda5050_types::ErrorLevel& level)
  {
    using vda5050_types::ErrorLevel;

    switch (level)
    {
      case ErrorLevel::WARNING:
        return "WARNING";
      case ErrorLevel::FATAL:
        return "FATAL";
      default:
        throw std::runtime_error("Invalid ErrorLevel enum value");
    }
  }

  static vda5050_types::ErrorLevel from_string(const std::string& level)
  {
    using vda5050_types::ErrorLevel;

    if (level == "WARNING") return ErrorLevel::WARNING;
    if (level == "FATAL") return ErrorLevel::FATAL;
    throw std::runtime_error("Invalid errorLevel string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct error_level_traits<std::string>
{
  static std::string to_string(const std::string& level)
  {
    using vda5050_msgs::msg::Error;

    if (
      level == Error::ERROR_LEVEL_WARNING || level == Error::ERROR_LEVEL_FATAL)
    {
      return level;
    }
    throw std::runtime_error("Invalid error_level value");
  }

  static std::string from_string(const std::string& level)
  {
    using vda5050_msgs::msg::Error;

    if (
      level == Error::ERROR_LEVEL_WARNING || level == Error::ERROR_LEVEL_FATAL)
    {
      return level;
    }
    throw std::runtime_error("Invalid errorLevel string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct e_stop_traits;

//=============================================================================
template <>
struct e_stop_traits<vda5050_types::EStop>
{
  static std::string to_string(const vda5050_types::EStop& type)
  {
    using vda5050_types::EStop;

    switch (type)
    {
      case EStop::AUTOACK:
        return "AUTOACK";
      case EStop::MANUAL:
        return "MANUAL";
      case EStop::REMOTE:
        return "REMOTE";
      case EStop::NONE:
        return "NONE";
      default:
        throw std::runtime_error("Invalid EStop enum value");
    }
  }

  static vda5050_types::EStop from_string(const std::string& type)
  {
    using vda5050_types::EStop;

    if (type == "AUTOACK") return EStop::AUTOACK;
    if (type == "MANUAL") return EStop::MANUAL;
    if (type == "REMOTE") return EStop::REMOTE;
    if (type == "NONE") return EStop::NONE;
    throw std::runtime_error("Invalid eStop string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct e_stop_traits<std::string>
{
  static std::string to_string(const std::string& type)
  {
    using vda5050_msgs::msg::SafetyState;

    if (
      type == SafetyState::E_STOP_AUTOACK ||
      type == SafetyState::E_STOP_MANUAL ||
      type == SafetyState::E_STOP_REMOTE || type == SafetyState::E_STOP_NONE)
    {
      return type;
    }
    throw std::runtime_error("Invalid e_stop value");
  }

  static std::string from_string(const std::string& type)
  {
    using vda5050_msgs::msg::SafetyState;

    if (
      type == SafetyState::E_STOP_AUTOACK ||
      type == SafetyState::E_STOP_MANUAL ||
      type == SafetyState::E_STOP_REMOTE || type == SafetyState::E_STOP_NONE)
    {
      return type;
    }
    throw std::runtime_error("Invalid eStop string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct info_level_traits;

//=============================================================================
template <>
struct info_level_traits<vda5050_types::InfoLevel>
{
  static std::string to_string(const vda5050_types::InfoLevel& level)
  {
    using vda5050_types::InfoLevel;

    switch (level)
    {
      case InfoLevel::INFO:
        return "INFO";
      case InfoLevel::DEBUG:
        return "DEBUG";
      default:
        throw std::runtime_error("Invalid InfoLevel enum value");
    }
  }

  static vda5050_types::InfoLevel from_string(const std::string& level)
  {
    using vda5050_types::InfoLevel;

    if (level == "INFO") return InfoLevel::INFO;
    if (level == "DEBUG") return InfoLevel::DEBUG;
    throw std::runtime_error("Invalid infoLevel string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct info_level_traits<std::string>
{
  static std::string to_string(const std::string& level)
  {
    using vda5050_msgs::msg::Info;

    if (level == Info::INFO_LEVEL_INFO || level == Info::INFO_LEVEL_DEBUG)
    {
      return level;
    }
    throw std::runtime_error("Invalid info_level value");
  }

  static std::string from_string(const std::string& level)
  {
    using vda5050_msgs::msg::Info;

    if (level == Info::INFO_LEVEL_INFO || level == Info::INFO_LEVEL_DEBUG)
    {
      return level;
    }
    throw std::runtime_error("Invalid infoLevel string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct blocking_type_traits;

//=============================================================================
template <>
struct blocking_type_traits<vda5050_types::BlockingType>
{
  static std::string to_string(const vda5050_types::BlockingType& type)
  {
    using vda5050_types::BlockingType;

    switch (type)
    {
      case BlockingType::NONE:
        return "NONE";
      case BlockingType::SOFT:
        return "SOFT";
      case BlockingType::HARD:
        return "HARD";
      default:
        throw std::runtime_error("Invalid BlockingType enum value");
    }
  }

  static vda5050_types::BlockingType from_string(const std::string& type)
  {
    using vda5050_types::BlockingType;

    if (type == "NONE") return BlockingType::NONE;
    if (type == "SOFT") return BlockingType::SOFT;
    if (type == "HARD") return BlockingType::HARD;
    throw std::runtime_error("Invalid blockingType string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct blocking_type_traits<std::string>
{
  static std::string to_string(const std::string& type)
  {
    using vda5050_msgs::msg::Action;

    if (
      type == Action::BLOCKING_TYPE_NONE ||
      type == Action::BLOCKING_TYPE_SOFT || type == Action::BLOCKING_TYPE_HARD)
    {
      return type;
    }
    throw std::runtime_error("Invalid blocking_type value");
  }

  static std::string from_string(const std::string& type)
  {
    using vda5050_msgs::msg::Action;

    if (
      type == Action::BLOCKING_TYPE_NONE ||
      type == Action::BLOCKING_TYPE_SOFT || type == Action::BLOCKING_TYPE_HARD)
    {
      return type;
    }
    throw std::runtime_error("Invalid blockingType string");
  }
};
#endif  // ENABLE_ROS2

//=============================================================================
template <typename T>
struct orientation_type_traits;

//=============================================================================
template <>
struct orientation_type_traits<vda5050_types::OrientationType>
{
  static std::string to_string(const vda5050_types::OrientationType& type)
  {
    using vda5050_types::OrientationType;

    switch (type)
    {
      case OrientationType::GLOBAL:
        return "GLOBAL";
      case OrientationType::TANGENTIAL:
        return "TANGENTIAL";
      default:
        throw std::runtime_error("Invalid OrientationType enum value");
    }
  }

  static vda5050_types::OrientationType from_string(const std::string& type)
  {
    using vda5050_types::OrientationType;

    if (type == "GLOBAL") return OrientationType::GLOBAL;
    if (type == "TANGENTIAL") return OrientationType::TANGENTIAL;
    throw std::runtime_error("Invalid orientationType string");
  }
};

//=============================================================================
#ifdef ENABLE_ROS2
template <>
struct orientation_type_traits<std::string>
{
  static std::string to_string(const std::string& type)
  {
    using vda5050_msgs::msg::Edge;

    if (
      type == Edge::ORIENTATION_TYPE_TANGENTIAL ||
      type == Edge::ORIENTATION_TYPE_GLOBAL)
    {
      return type;
    }
    throw std::runtime_error("Invalid orientation_type value");
  }

  static std::string from_string(const std::string& type)
  {
    using vda5050_msgs::msg::Edge;

    if (
      type == Edge::ORIENTATION_TYPE_TANGENTIAL ||
      type == Edge::ORIENTATION_TYPE_GLOBAL)
    {
      return type;
    }
    throw std::runtime_error("Invalid orientationType string");
  }
};
#endif  // ENABLE_ROS2

}  // namespace vda5050_json_utils

#endif  // VDA5050_JSON_UTILS__TRAITS_HPP_
