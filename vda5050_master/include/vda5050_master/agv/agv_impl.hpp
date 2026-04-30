/*
 * Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
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

#ifndef VDA5050_MASTER__AGV__AGV_IMPL_HPP_
#define VDA5050_MASTER__AGV__AGV_IMPL_HPP_

#include <functional>
#include <optional>
#include <utility>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_execution/protocol_adapter.hpp"
#include "vda5050_master/agv/agv.hpp"
#include "vda5050_master/standard_names.hpp"
#include "vda5050_types/error.hpp"

namespace vda5050_master {
namespace detail {

/// Wire a typed subscription on `agv->protocol_adapter_`. Logs parse
/// errors at ERROR level and exceptions from `handler` at WARN level
/// without re-throwing — both are non-fatal for the AGV.
template <typename MsgType>
void create_subscription(
  AGV* agv, std::function<void(const MsgType&)> handler, QosLevel qos)
{
  agv->protocol_adapter_->template subscribe<MsgType>(
    [agv, handler = std::move(handler)](
      MsgType msg, std::optional<vda5050_types::Error> error) {
      if (error.has_value())
      {
        VDA5050_ERROR(
          "[AGV] Failed to parse message for {}: {}", agv->agv_id_,
          error->error_description.value_or("unknown error"));
        return;
      }

      try
      {
        handler(msg);
      }
      catch (const std::exception& e)
      {
        VDA5050_WARN(
          "[AGV] Failed to handle message for {}: {}", agv->agv_id_, e.what());
      }
    },
    static_cast<int>(qos));
}

}  // namespace detail
}  // namespace vda5050_master

#endif  // VDA5050_MASTER__AGV__AGV_IMPL_HPP_
