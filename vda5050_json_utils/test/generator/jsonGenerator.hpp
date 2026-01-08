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

#ifndef GENERATOR__JSONGENERATOR_HPP_
#define GENERATOR__JSONGENERATOR_HPP_

#include <nlohmann/json.hpp>

#include "generator.hpp"
#include "vda5050_types/connection_state.hpp"

/// \brief Utility class to generate random VDA5050 JSON objects
class RandomJSONgenerator
{
public:
  /// \brief Enum values for each VDA5050 JSON object
  enum class JsonTypes
  {
    Connection,
    Order,
    InstantActions,
    State,
    Visualization,
    Factsheet
  };

  /// \brief Generate a fully populated JSON object of a supported type
  nlohmann::json generate(const JsonTypes type)
  {
    nlohmann::json j;
    RandomDataGenerator generator;

    j["headerId"] = generator.generate_random_uint();
    j["timestamp"] = generator.generate_random_ISO8601_timestamp();
    j["version"] = generator.generate_random_string();
    j["manufacturer"] = generator.generate_random_string();
    j["serialNumber"] = generator.generate_random_string();

    switch (type)
    {
      case JsonTypes::Connection:
        /// create Connection JSON object

        ConnectionState random_connection_state{
          generator.generate_random_connection_state()};

        if (random_connection_state == ConnectionState::CONNECTIONBROKEN)
        {
          j["connectionState"] = "CONNECTIONBROKEN";
        }
        else if (random_connection_state == ConnectionState::OFFLINE)
        {
          j["connectionState"] = "OFFLINE";
        }
        else
        {
          j["connectionState"] = "ONLINE";
        }

        break;

      case JsonTypes::Order:
        /// TODO: (@shawnkchan) complete this once random generator for Order
        /// message is completed create Order JSON Object
        break;

      case JsonTypes::InstantActions:
        /// TODO: (@shawnkchan) complete this once random generator for
        /// InstantActions message is completed create InstantActions JSON
        /// Object
        break;

      case JsonTypes::State:
        /// TODO: (@shawnkchan) complete this once random generator for State
        /// message is completed create State object
        break;

      case JsonTypes::Visualization:
        /// TODO: (@shawnkchan) complete this once random generator for
        /// Visualization message is completed create Visualization object
        break;

      case JsonTypes::Factsheet:
        /// TODO: (@shawnkchan) complete this once random generator for
        /// Factsheet message is completed Factsheet
        break;
    }
    return j;
  }
};

#endif  // GENERATOR__JSONGENERATOR_HPP_
