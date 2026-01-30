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

#ifndef VDA5050_EXECUTION__EVENT_HPP_
#define VDA5050_EXECUTION__EVENT_HPP_

#include <memory>
#include <typeindex>
#include <utility>
#include <vector>

#include <vda5050_types/edge.hpp>
#include <vda5050_types/node.hpp>

namespace vda5050_execution {

struct EventBase
{
  virtual ~EventBase() = default;
  virtual std::type_index get_type() const = 0;
};

struct NavigationNodeReady : public EventBase
{
  std::shared_ptr<const vda5050_types::Node> target_node;
  std::shared_ptr<const vda5050_types::Edge> traversal_edge;

  NavigationNodeReady(
    std::shared_ptr<const vda5050_types::Node> node,
    std::shared_ptr<const vda5050_types::Edge> edge)
  : target_node(std::move(node)), traversal_edge(std::move(edge))
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(NavigationNodeReady));
  }
};

struct NavigationSegmentReady : public EventBase
{
  std::vector<std::shared_ptr<const vda5050_types::Node>> target_segment;
  std::vector<std::shared_ptr<const vda5050_types::Edge>> traversal_edges;

  NavigationSegmentReady(
    std::vector<std::shared_ptr<const vda5050_types::Node>> nodes,
    std::vector<std::shared_ptr<const vda5050_types::Edge>> edges)
  : target_segment(std::move(nodes)), traversal_edges(std::move(edges))
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(NavigationSegmentReady));
  }
};

struct NavigationStatusChange : public EventBase
{
  bool paused = false;

  explicit NavigationStatusChange(bool pause) : paused(pause)
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(NavigationStatusChange));
  }
};

struct NavigationReset : public EventBase
{
  NavigationReset()
  {
    // Nothing to do here ...
  }

  std::type_index get_type() const override
  {
    return std::type_index(typeid(NavigationReset));
  }
};

}  // namespace vda5050_execution

#endif  // VDA5050_EXECUTION__EVENT_HPP_
