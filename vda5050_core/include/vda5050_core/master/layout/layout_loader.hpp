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

#ifndef VDA5050_CORE__MASTER__LAYOUT__LAYOUT_LOADER_HPP_
#define VDA5050_CORE__MASTER__LAYOUT__LAYOUT_LOADER_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "vda5050_core/master/layout/lif.hpp"

namespace vda5050_core::master::layout {

enum class LayoutLoadErrorType : std::uint8_t
{
  FILE_NOT_FOUND,
  FILE_READ_FAILED,
  JSON_PARSE_ERROR,
  MISSING_REQUIRED_FIELD,
  INVALID_FIELD_VALUE,
  DUPLICATE_ID,
  DANGLING_NODE_REF,
  EMPTY_REQUIRED_ARRAY,
  TRAJECTORY_SIZE_MISMATCH,
  OUT_OF_RANGE,
};

struct LayoutLoadError
{
  LayoutLoadErrorType type;
  std::string description;
};

struct LayoutLoadResult
{
  std::optional<LIF> lif;
  std::vector<LayoutLoadError> errors;

  explicit operator bool() const
  {
    return errors.empty() && lif.has_value();
  }
};

// Load a single LIF file. Node, edge, station, and layout IDs are required
// globally unique across the loaded LIF. Multi-vendor reconciliation (where
// different vendor LIFs may legitimately reuse IDs and must be remapped by
// the master) is out of scope; loading several vendor LIFs as one will yield
// DUPLICATE_ID errors by design.
LayoutLoadResult load_from_file(const std::string& path);
LayoutLoadResult load_from_json(const nlohmann::json& json);

std::vector<LayoutLoadError> validate_layout(const LIF& lif);

}  // namespace vda5050_core::master::layout

#endif  // VDA5050_CORE__MASTER__LAYOUT__LAYOUT_LOADER_HPP_
