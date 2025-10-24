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

#include <iostream>

#include <nlohmann/json.hpp>

#include <vda5050_types/header.hpp>

#ifdef ENABLE_ROS2
#include <vda5050_msgs/msg/header.hpp>
#endif  // ENABLE_ROS2

#include "traits.hpp"

namespace vda5050_types {

namespace header_detail {

//=============================================================================
template <typename HeaderT>
void to_json(nlohmann::json& j, const HeaderT& msg)
{
  using namespace vda5050_json_utils;

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
  using namespace vda5050_json_utils;

  msg.header_id = j.at("headerId").get<uint32_t>();
  msg.timestamp = timestamp_traits<decltype(msg.timestamp)>::from_string(
    j.at("timestamp").get<std::string>());
  msg.version = j.at("version").get<std::string>();
  msg.manufacturer = j.at("manufacturer").get<std::string>();
  msg.serial_number = j.at("serialNumber").get<std::string>();
}

}  // namespace header_detail

inline void to_json(nlohmann::json& j, const Header& msg)
{
  vda5050_types::header_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Header& msg)
{
  vda5050_types::header_detail::from_json(j, msg);
}

namespace connection_detail {

//=============================================================================
template <typename ConnectionT>
void to_json(nlohmann::json& j, const ConnectionT& msg)
{
  using namespace vda5050_json_utils;

  header_detail::to_json(j, msg.header);

  j["connectionState"] =
    connection_state_traits<decltype(msg.connection_state)>::to_string(
      msg.connection_state);
}

//=============================================================================
template <typename ConnectionT>
void from_json(const nlohmann::json& j, ConnectionT& msg)
{
  using namespace vda5050_json_utils;

  header_detail::from_json(j, msg.header);

  msg.connection_state =
    connection_state_traits<decltype(msg.connection_state)>::from_string(
      j.at("connectionState").get<std::string>());
}

}  // namespace connection_detail

inline void to_json(nlohmann::json& j, const Connection& msg)
{
  vda5050_types::connection_detail::to_json(j, msg);
}

inline void from_json(const nlohmann::json& j, Connection& msg)
{
  vda5050_types::connection_detail::from_json(j, msg);
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

}  // namespace msg

}  // namespace vda5050_msgs
#endif  // ENABLE_ROS2

#endif  // VDA5050_JSON_UTILS__SERIALIZATION_HPP_
