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

#include <gmock/gmock.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "vda5050_core/adapter/adapter.hpp"
#include "vda5050_core/adapter/reporter.hpp"
#include "vda5050_core/execution/protocol_adapter.hpp"
#include "vda5050_core/json_utils/serialization.hpp"
#include "vda5050_core/transport/mqtt_client_interface.hpp"
#include "vda5050_core/types/edge.hpp"
#include "vda5050_core/types/node.hpp"
#include "vda5050_core/types/node_position.hpp"
#include "vda5050_core/types/order.hpp"
#include "vda5050_core/types/state.hpp"

namespace {

namespace types = vda5050_core::types;
namespace execution = vda5050_core::execution;
namespace adapter = vda5050_core::adapter;
namespace transport = vda5050_core::transport;

bool ends_with(const std::string& s, const std::string& suffix)
{
  return s.size() >= suffix.size() &&
         s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// In-memory MqttClientInterface: records publishes and lets the test inject
// inbound messages onto whatever topic the adapter subscribed to. No broker.
class FakeMqttClient : public transport::MqttClientInterface
{
public:
  void connect() override
  {
    connected_ = true;
  }
  void disconnect() override
  {
    connected_ = false;
  }
  bool connected() override
  {
    return connected_;
  }

  void publish(
    const std::string& topic, const std::string& message, int /*qos*/,
    bool /*retain*/) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    published_.emplace_back(topic, message);
  }

  void subscribe(
    const std::string& topic, MessageHandler handler, int /*qos*/) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_[topic] = std::move(handler);
  }

  void unsubscribe(const std::string& topic) override
  {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.erase(topic);
  }

  void set_will(const std::string&, const std::string&, int, bool) override {}

  // Deliver `payload` to the handler whose topic ends with `topic_suffix`.
  void inject(const std::string& topic_suffix, const std::string& payload)
  {
    MessageHandler handler;
    std::string topic;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (const auto& entry : handlers_)
      {
        if (ends_with(entry.first, topic_suffix))
        {
          handler = entry.second;
          topic = entry.first;
          break;
        }
      }
    }
    if (handler) handler(topic, payload);
  }

  // All payloads published to topics ending with `topic_suffix`, in order.
  std::vector<std::string> payloads_for(const std::string& topic_suffix)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> out;
    for (const auto& entry : published_)
    {
      if (ends_with(entry.first, topic_suffix)) out.push_back(entry.second);
    }
    return out;
  }

private:
  std::atomic_bool connected_{false};
  std::mutex mutex_;
  std::unordered_map<std::string, MessageHandler> handlers_;
  std::vector<std::pair<std::string, std::string>> published_;
};

types::Node make_node(
  const std::string& node_id, uint32_t sequence_id, double x)
{
  types::Node node;
  node.node_id = node_id;
  node.sequence_id = sequence_id;
  node.released = true;

  types::NodePosition position;
  position.x = x;
  position.y = 0.0;
  position.map_id = "map";
  node.node_position = position;

  return node;
}

types::Edge make_edge(
  const std::string& edge_id, uint32_t sequence_id,
  const std::string& start_node_id, const std::string& end_node_id)
{
  types::Edge edge;
  edge.edge_id = edge_id;
  edge.sequence_id = sequence_id;
  edge.start_node_id = start_node_id;
  edge.end_node_id = end_node_id;
  edge.released = true;
  return edge;
}

// A valid 3-node chain: node_0 -edge_1- node_2 -edge_3- node_4.
types::Order make_order()
{
  types::Order order;
  order.order_id = "smoke-order";
  order.order_update_id = 0;
  order.nodes = {
    make_node("node_0", 0, 0.0), make_node("node_2", 2, 2.0),
    make_node("node_4", 4, 4.0)};
  order.edges = {
    make_edge("edge_1", 1, "node_0", "node_2"),
    make_edge("edge_3", 3, "node_2", "node_4")};
  return order;
}

// End-to-end through the real Context+Strategy pipeline, no broker:
//   inject Order -> OrderAcceptance applies it -> node_reached drives
//   OrderTraversal -> NavigateToNodeEvent -> on_navigate -> StateReporting
//   publishes State.
TEST(AdapterSmokeTest, DrivesOrderThroughNavigateAndPublishesState)
{
  auto fake = std::make_shared<FakeMqttClient>();
  auto protocol =
    execution::ProtocolAdapter::make(fake, "uagv", "2.0.0", "ROS-I", "S001");
  auto vda_adapter = adapter::Adapter::make(protocol);
  auto reporter = vda_adapter->reporter();

  std::mutex mutex;
  std::vector<types::Node> dispatched;

  // Each dispatched node is recorded, then acknowledged as instantly reached to
  // drive the traversal to the next node (node_reached only queues an update,
  // so this is non-reentrant against the spin thread).
  vda_adapter->on_navigate(
    [&](types::Node node, std::optional<types::Edge> /*edge*/) {
      {
        std::lock_guard<std::mutex> lock(mutex);
        dispatched.push_back(node);
      }
      reporter->node_reached(node.node_id, node.sequence_id);
    });

  vda_adapter->start();

  nlohmann::json order_json = make_order();
  fake->inject("/order", order_json.dump());

  // No manual start kick: the adapter auto-reports the order's first node
  // (node_0) reached on acceptance, which starts the traversal.

  // Wait until the route is fully traversed (terminal State published).
  types::State final_state;
  bool route_done = false;
  for (int i = 0; i < 150 && !route_done; ++i)
  {
    auto states = fake->payloads_for("/state");
    if (!states.empty())
    {
      final_state = nlohmann::json::parse(states.back());
      if (
        final_state.node_states.empty() && final_state.last_node_id == "node_4")
      {
        route_done = true;
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  vda_adapter->stop();

  EXPECT_TRUE(route_done) << "route never reached the terminal published State";

  // on_navigate dispatched the two downstream nodes (the start node is reached,
  // never navigated to), with the full positions resolved from the active order.
  std::vector<std::string> ids;
  double node_2_x = -1.0;
  double node_4_x = -1.0;
  {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& node : dispatched)
    {
      ids.push_back(node.node_id);
      ASSERT_TRUE(node.node_position.has_value());
      if (node.node_id == "node_2") node_2_x = node.node_position->x;
      if (node.node_id == "node_4") node_4_x = node.node_position->x;
    }
  }

  EXPECT_THAT(ids, testing::ElementsAre("node_2", "node_4"));
  EXPECT_DOUBLE_EQ(node_2_x, 2.0);
  EXPECT_DOUBLE_EQ(node_4_x, 4.0);

  // The connection came up ONLINE on start.
  EXPECT_FALSE(fake->payloads_for("/connection").empty());
}

}  // namespace
