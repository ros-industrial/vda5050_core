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

#include <vda5050_msgs/msg/action.hpp>
#include <vda5050_msgs/msg/action_parameter.hpp>
#include <vda5050_msgs/msg/action_state.hpp>
#include <vda5050_msgs/msg/agv_position.hpp>
#include <vda5050_msgs/msg/battery_state.hpp>
#include <vda5050_msgs/msg/bounding_box_reference.hpp>
#include <vda5050_msgs/msg/connection.hpp>
#include <vda5050_msgs/msg/control_point.hpp>
#include <vda5050_msgs/msg/edge.hpp>
#include <vda5050_msgs/msg/edge_state.hpp>
#include <vda5050_msgs/msg/error.hpp>
#include <vda5050_msgs/msg/error_reference.hpp>
#include <vda5050_msgs/msg/header.hpp>
#include <vda5050_msgs/msg/info.hpp>
#include <vda5050_msgs/msg/info_reference.hpp>
#include <vda5050_msgs/msg/load.hpp>
#include <vda5050_msgs/msg/load_dimensions.hpp>
#include <vda5050_msgs/msg/node.hpp>
#include <vda5050_msgs/msg/node_position.hpp>
#include <vda5050_msgs/msg/node_state.hpp>
#include <vda5050_msgs/msg/order.hpp>
#include <vda5050_msgs/msg/safety_state.hpp>
#include <vda5050_msgs/msg/state.hpp>
#include <vda5050_msgs/msg/trajectory.hpp>
#include <vda5050_msgs/msg/velocity.hpp>

#include "vda5050_json_utils/serialization.hpp"

#include "generator/generator_ros2.hpp"

using vda5050_msgs::msg::Action;
using vda5050_msgs::msg::ActionParameter;
using vda5050_msgs::msg::ActionState;
using vda5050_msgs::msg::AGVPosition;
using vda5050_msgs::msg::BatteryState;
using vda5050_msgs::msg::BoundingBoxReference;
using vda5050_msgs::msg::Connection;
using vda5050_msgs::msg::ControlPoint;
using vda5050_msgs::msg::Edge;
using vda5050_msgs::msg::EdgeState;
using vda5050_msgs::msg::Error;
using vda5050_msgs::msg::ErrorReference;
using vda5050_msgs::msg::Header;
using vda5050_msgs::msg::Info;
using vda5050_msgs::msg::InfoReference;
using vda5050_msgs::msg::Load;
using vda5050_msgs::msg::LoadDimensions;
using vda5050_msgs::msg::Node;
using vda5050_msgs::msg::NodePosition;
using vda5050_msgs::msg::NodeState;
using vda5050_msgs::msg::Order;
using vda5050_msgs::msg::SafetyState;
using vda5050_msgs::msg::State;
using vda5050_msgs::msg::Trajectory;
using vda5050_msgs::msg::Velocity;

// List of types to be tested for serialization round-trip
using SerializableTypesROS2 = ::testing::Types<
  Action, ActionParameter, ActionState, AGVPosition, BatteryState,
  BoundingBoxReference, Connection, ControlPoint, Edge, EdgeState, Error,
  ErrorReference, Header, Info, InfoReference, Load, LoadDimensions, Node,
  NodePosition, NodeState, Order, SafetyState, State, Trajectory, Velocity>;

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
