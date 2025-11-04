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

#ifndef VDA5050_JSON_UTILS__SERIALIZATION_HPP_
#define VDA5050_JSON_UTILS__SERIALIZATION_HPP_

#include <string>

#include <nlohmann/json.hpp>

#include <vda5050_types/connection.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/state.hpp>

#ifdef ENABLE_ROS2
#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/header.hpp>
#include <vda5050_msgs/msg/state.hpp>
#endif  // ENABLE_ROS2

#include "traits.hpp"

namespace vda5050_types {

namespace header_detail {

//=============================================================================
template <typename HeaderT>
void to_json(nlohmann::json& j, const HeaderT& msg)
{
  using vda5050_json_utils::timestamp_traits;

  j = nlohmann::json{
    {"headerId", msg.header_id},
    {"timestamp",
     timestamp_traits<decltype(msg.timestamp)>::to_string(msg.timestamp)},
    {"version", msg.version},
    {"manufacturer", msg.manufacturer},
    {"serialNumber", msg.serial_number}};
}

//=============================================================================
template <typename HeaderT>
void from_json(const nlohmann::json& j, HeaderT& msg)
{
  using vda5050_json_utils::timestamp_traits;

  msg.header_id = j.at("headerId").get<uint32_t>();
  msg.timestamp = timestamp_traits<decltype(msg.timestamp)>::from_string(
    j.at("timestamp").get<std::string>());
  msg.version = j.at("version").get<std::string>();
  msg.manufacturer = j.at("manufacturer").get<std::string>();
  msg.serial_number = j.at("serialNumber").get<std::string>();
}

}  // namespace header_detail

//=============================================================================
inline void to_json(nlohmann::json& j, const Header& msg)
{
  vda5050_types::header_detail::to_json(j, msg);
}

//=============================================================================
inline void from_json(const nlohmann::json& j, Header& msg)
{
  vda5050_types::header_detail::from_json(j, msg);
}

namespace connection_detail {

//=============================================================================
template <typename ConnectionT>
void to_json(nlohmann::json& j, const ConnectionT& msg)
{
  using vda5050_json_utils::connection_state_traits;

  header_detail::to_json(j, msg.header);

  j["connectionState"] =
    connection_state_traits<decltype(msg.connection_state)>::to_string(
      msg.connection_state);
}

//=============================================================================
template <typename ConnectionT>
void from_json(const nlohmann::json& j, ConnectionT& msg)
{
  using vda5050_json_utils::connection_state_traits;

  header_detail::from_json(j, msg.header);

  msg.connection_state =
    connection_state_traits<decltype(msg.connection_state)>::from_string(
      j.at("connectionState").get<std::string>());
}

}  // namespace connection_detail

//=============================================================================
inline void to_json(nlohmann::json& j, const Connection& msg)
{
  vda5050_types::connection_detail::to_json(j, msg);
}

//=============================================================================
inline void from_json(const nlohmann::json& j, Connection& msg)
{
  vda5050_types::connection_detail::from_json(j, msg);
}

namespace state_detail {

//=============================================================================
template <typename StateT>
void to_json(nlohmann::json& j, const StateT& msg)
{
  using vda5050_json_utils::operating_mode_traits;

  header_detail::to_json(j, msg.header);

  j["orderId"] = msg.order_id;
  j["orderUpdateId"] = msg.order_update_id;
  j["lastNodeId"] = msg.last_node_id;
  j["lastNodeSequenceId"] = msg.last_node_sequence_id;
  j["driving"] = msg.driving;

  j["operatingMode"] =
    operating_mode_traits<decltype(msg.operating_mode)>::to_string(
      msg.operating_mode);
}

//=============================================================================
template <typename StateT>
void from_json(const nlohmann::json& j, StateT& msg)
{
  using vda5050_json_utils::operating_mode_traits;
  using vda5050_types::header_detail::from_json;

  header_detail::from_json(j, msg.header);

  msg.order_id = j.at("orderId").get<std::string>();
  msg.order_update_id = j.at("orderUpdateId").get<uint32_t>();
  msg.last_node_id = j.at("lastNodeId").get<std::string>();
  msg.last_node_sequence_id = j.at("lastNodeSequenceId").get<uint32_t>();
  msg.driving = j.at("driving").get<bool>();

  msg.operating_mode =
    operating_mode_traits<decltype(msg.operating_mode)>::from_string(
      j.at("operatingMode").get<std::string>());
}

}  // namespace state_detail

//=============================================================================
inline void to_json(nlohmann::json& j, const State& msg)
{
  vda5050_types::state_detail::to_json(j, msg);
}

//=============================================================================
inline void from_json(const nlohmann::json& j, State& msg)
{
  vda5050_types::state_detail::from_json(j, msg);
}

}  // namespace vda5050_types

//=============================================================================
#ifdef ENABLE_ROS2
namespace vda5050_msgs {

namespace msg {

inline void to_json(nlohmann::json& j, const Header& msg)
{
  vda5050_types::header_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Header& msg)
{
  vda5050_types::header_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const Connection& msg)
{
  vda5050_types::connection_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Connection& msg)
{
  vda5050_types::connection_detail::from_json(j, msg);
}

inline void to_json(nlohmann::json& j, const State& msg)
{
  vda5050_types::state_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, State& msg)
{
  vda5050_types::state_detail::from_json(j, msg);
}

}  // namespace msg

}  // namespace vda5050_msgs
#endif  // ENABLE_ROS2

#endif  // VDA5050_JSON_UTILS__SERIALIZATION_HPP_
