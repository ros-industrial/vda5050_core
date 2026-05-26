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

#ifndef VDA5050_CORE__MASTER__VALIDATION__FACTSHEET_ALIGNMENT_HPP_
#define VDA5050_CORE__MASTER__VALIDATION__FACTSHEET_ALIGNMENT_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "vda5050_core/master/map/map.hpp"
#include "vda5050_core/types/factsheet.hpp"

namespace vda5050_core::master {

// Cross-checks a loaded Map against an AGV factsheet (speed capability).

enum class AlignmentSeverity : std::uint8_t
{
  WARNING = 0,
  ERROR = 1,
};

struct AlignmentFinding
{
  AlignmentSeverity severity;
  std::string code;
  std::string description;
};

struct FactsheetAlignmentResult
{
  std::vector<AlignmentFinding> findings;
  bool has_error() const;
};

FactsheetAlignmentResult check_factsheet_alignment(
  const Map& map, const vda5050_core::types::Factsheet& factsheet);

}  // namespace vda5050_core::master

#endif  // VDA5050_CORE__MASTER__VALIDATION__FACTSHEET_ALIGNMENT_HPP_
