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

#include <rosidl_runtime_cpp/traits.hpp>

#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/header.hpp>
#include <vda5050_msgs/msg/node_position.hpp>
#include <vda5050_msgs/msg/node_state.hpp>
#include <vda5050_msgs/msg/state.hpp>

#include "vda5050_json_utils/serialization.hpp"

#include "generator/generator_ros2.hpp"

using vda5050_msgs::msg::Connection;
using vda5050_msgs::msg::Header;
using vda5050_msgs::msg::NodePosition;
using vda5050_msgs::msg::NodeState;
using vda5050_msgs::msg::State;

// List of types to be tested for serialization round-trip
using SerializableTypesROS2 =
  ::testing::Types<Connection, Header, NodePosition, NodeState, State>;

template <typename T>
class SerializationTestROS2 : public ::testing::Test
{
protected:
  // Random data generator instance
  RandomDataGeneratorROS2 generator;

  /// \brief Performs a serialization round-trip for a given message object
  ///
  /// \param original Object of type T to be tested
  void round_trip_test(const T& original)
  {
    // Serialize the original object into JSON
    nlohmann::json serialized_data = original;

    // Deserialize the JSON object back into an object of type T
    T deserialized_object = serialized_data;

    EXPECT_EQ(original, deserialized_object)
      << "Serialization round-trip failed for type: " << typeid(T).name();
  }
};

TYPED_TEST_SUITE(SerializationTestROS2, SerializableTypesROS2);

TYPED_TEST(SerializationTestROS2, RoundTrip)
{
  // Number of iterations for round-trip of each object
  const int num_random_tests = 100;

  for (int i = 0; i < num_random_tests; i++)
  {
    SCOPED_TRACE("Test iteration " + std::to_string(i));
    TypeParam random_object = this->generator.template generate<TypeParam>();
    this->round_trip_test(random_object);
  }
}
