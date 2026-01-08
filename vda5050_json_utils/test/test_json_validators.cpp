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

#include <gtest/gtest.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "generator/jsonGenerator.hpp"
#include "vda5050_json_utils/schemas.hpp"
#include "vda5050_json_utils/validators.hpp"

/// \brief Fixture class to create VDA5050 JSON objects for tests
class JsonValidatorTest : public ::testing::Test
{
protected:
  JsonValidatorTest()
  {
    connection_object =
      json_generator.generate(RandomJSONgenerator::JsonTypes::Connection);
    connection_schema = nlohmann::json::parse(connection_schema);
    /// TODO: Instantiate other VDA5050 JSON objects
  }

  RandomJSONgenerator json_generator;
  nlohmann::json connection_object;
  nlohmann::json connection_schema;
  /// TODO: Declare other VDA5050 JSON objects
};

/// \brief Tests the is_valid_schema function to check that it passes when validating a correctly formatted JSON object
TEST_F(JsonValidatorTest, BasicValidationTest)
{
  /// TODO (@shawnkchan) Change this to a typed test so that we can iterate over the different VDA5050 object types
  std::cout << "running test" << "\n";
  int connection_result = is_valid_schema(connection_schema, connection_object);
  EXPECT_EQ(connection_result, true);
  std::cout << "test ended" << "\n";
}
