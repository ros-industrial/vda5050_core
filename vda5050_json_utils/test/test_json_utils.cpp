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

#include <vda5050_types/action.hpp>
#include <vda5050_types/action_parameter.hpp>
#include <vda5050_types/action_state.hpp>
#include <vda5050_types/agv_position.hpp>
#include <vda5050_types/battery_state.hpp>
#include <vda5050_types/bounding_box_reference.hpp>
#include <vda5050_types/connection.hpp>
#include <vda5050_types/control_point.hpp>
#include <vda5050_types/edge.hpp>
#include <vda5050_types/edge_state.hpp>
#include <vda5050_types/error.hpp>
#include <vda5050_types/error_reference.hpp>
#include <vda5050_types/header.hpp>
#include <vda5050_types/info.hpp>
#include <vda5050_types/info_reference.hpp>
#include <vda5050_types/load.hpp>
#include <vda5050_types/load_dimensions.hpp>
#include <vda5050_types/node.hpp>
#include <vda5050_types/node_position.hpp>
#include <vda5050_types/node_state.hpp>
#include <vda5050_types/order.hpp>
#include <vda5050_types/safety_state.hpp>
#include <vda5050_types/state.hpp>
#include <vda5050_types/trajectory.hpp>
#include <vda5050_types/velocity.hpp>

#include "vda5050_json_utils/serialization.hpp"

#include "generator/generator.hpp"

using vda5050_types::Action;
using vda5050_types::ActionParameter;
using vda5050_types::ActionState;
using vda5050_types::AGVPosition;
using vda5050_types::BatteryState;
using vda5050_types::BoundingBoxReference;
using vda5050_types::Connection;
using vda5050_types::ControlPoint;
using vda5050_types::Edge;
using vda5050_types::EdgeState;
using vda5050_types::Error;
using vda5050_types::ErrorReference;
using vda5050_types::Header;
using vda5050_types::Info;
using vda5050_types::InfoReference;
using vda5050_types::Load;
using vda5050_types::LoadDimensions;
using vda5050_types::Node;
using vda5050_types::NodePosition;
using vda5050_types::NodeState;
using vda5050_types::Order;
using vda5050_types::SafetyState;
using vda5050_types::State;
using vda5050_types::Trajectory;
using vda5050_types::Velocity;

// List of types to be tested for serialization round-trip
using SerializableTypes = ::testing::Types<
  Action, ActionParameter, ActionState, AGVPosition, BatteryState,
  BoundingBoxReference, Connection, ControlPoint, Edge, EdgeState, Error,
  ErrorReference, Header, Info, InfoReference, Load, LoadDimensions, Node,
  NodePosition, NodeState, Order, SafetyState, State, Trajectory, Velocity>;

template <typename T>
class SerializationTest : public ::testing::Test
{
protected:
  // Random data generator instance
  RandomDataGenerator generator;

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

TYPED_TEST_SUITE(SerializationTest, SerializableTypes);

TYPED_TEST(SerializationTest, RoundTrip)
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
