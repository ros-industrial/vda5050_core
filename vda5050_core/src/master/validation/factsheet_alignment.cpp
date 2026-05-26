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

#include "vda5050_core/master/validation/factsheet_alignment.hpp"

#include <algorithm>
#include <sstream>
#include <string>

namespace vda5050_core::master {

bool FactsheetAlignmentResult::has_error() const
{
  return std::any_of(findings.begin(), findings.end(), [](const auto& f) {
    return f.severity == AlignmentSeverity::ERROR;
  });
}

FactsheetAlignmentResult check_factsheet_alignment(
  const Map& map, const vda5050_core::types::Factsheet& factsheet)
{
  FactsheetAlignmentResult result;

  // SpeedExceedsCapability: flag edges whose max_speed exceeds the AGV's.
  const double agv_speed_max = factsheet.physical_parameters.speed_max;
  for (const auto& edge : map.edges)
  {
    if (!edge.max_speed.has_value()) continue;
    const double edge_max = edge.max_speed.value();
    if (edge_max > agv_speed_max)
    {
      std::ostringstream oss;
      oss << "Edge '" << edge.edge_id << "' max_speed=" << edge_max
          << " m/s exceeds AGV factsheet physical_parameters.speed_max="
          << agv_speed_max << " m/s.";
      result.findings.push_back(
        {AlignmentSeverity::ERROR, "SpeedExceedsCapability", oss.str()});
    }
  }

  return result;
}

}  // namespace vda5050_core::master
