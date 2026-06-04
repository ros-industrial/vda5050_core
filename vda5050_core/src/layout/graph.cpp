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

#include "vda5050_core/layout/graph.hpp"

#include <map>
#include <ostream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace vda5050_core::layout {

Graph::Ptr Graph::from_lif(LIF lif)
{
  // Uniqueness pre-check: protects programmatically-constructed inputs
  // (load_from_json already validates, but in-memory builds may not).
  std::unordered_set<std::string> layout_ids;
  std::unordered_set<std::string> node_ids;
  std::unordered_set<std::string> edge_ids;
  std::unordered_set<std::string> station_ids;

  for (const auto& layout : lif.layouts)
  {
    if (!layout_ids.insert(layout.layout_id).second)
    {
      throw std::invalid_argument(
        "Graph::from_lif: duplicate layoutId '" + layout.layout_id + "'");
    }
    for (const auto& node : layout.nodes)
    {
      if (!node_ids.insert(node.node_id).second)
      {
        throw std::invalid_argument(
          "Graph::from_lif: duplicate nodeId '" + node.node_id + "'");
      }
    }
    for (const auto& edge : layout.edges)
    {
      if (!edge_ids.insert(edge.edge_id).second)
      {
        throw std::invalid_argument(
          "Graph::from_lif: duplicate edgeId '" + edge.edge_id + "'");
      }
    }
    for (const auto& station : layout.stations)
    {
      if (!station_ids.insert(station.station_id).second)
      {
        throw std::invalid_argument(
          "Graph::from_lif: duplicate stationId '" + station.station_id + "'");
      }
    }
  }

  auto g = Ptr(new Graph());
  g->lif_ = std::move(lif);
  g->rebuild_indices_();
  return g;
}

void Graph::rebuild_indices_()
{
  node_index_.clear();
  edge_index_.clear();
  station_index_.clear();
  node_outbound_.clear();
  node_inbound_.clear();

  for (std::size_t li = 0; li < lif_.layouts.size(); ++li)
  {
    const auto& layout = lif_.layouts[li];
    for (std::size_t ni = 0; ni < layout.nodes.size(); ++ni)
    {
      const auto& node = layout.nodes[ni];
      node_index_.emplace(node.node_id, std::make_pair(li, ni));
      // Ensure adjacency entries exist (empty for isolated nodes).
      node_outbound_[node.node_id];
      node_inbound_[node.node_id];
    }
    for (std::size_t ei = 0; ei < layout.edges.size(); ++ei)
    {
      const auto& edge = layout.edges[ei];
      edge_index_.emplace(edge.edge_id, std::make_pair(li, ei));
      node_outbound_[edge.start_node_id].push_back(edge.edge_id);
      node_inbound_[edge.end_node_id].push_back(edge.edge_id);
    }
    for (std::size_t si = 0; si < layout.stations.size(); ++si)
    {
      const auto& station = layout.stations[si];
      station_index_.emplace(station.station_id, std::make_pair(li, si));
    }
  }
}

bool Graph::has_node(const std::string& id) const noexcept
{
  return node_index_.find(id) != node_index_.end();
}

bool Graph::has_edge(const std::string& id) const noexcept
{
  return edge_index_.find(id) != edge_index_.end();
}

bool Graph::has_station(const std::string& id) const noexcept
{
  return station_index_.find(id) != station_index_.end();
}

const Node& Graph::get_node(const std::string& id) const
{
  const auto* n = find_node(id);
  if (n == nullptr)
  {
    throw std::out_of_range("Graph::get_node: id '" + id + "' not found");
  }
  return *n;
}

const Edge& Graph::get_edge(const std::string& id) const
{
  const auto* e = find_edge(id);
  if (e == nullptr)
  {
    throw std::out_of_range("Graph::get_edge: id '" + id + "' not found");
  }
  return *e;
}

const Station& Graph::get_station(const std::string& id) const
{
  const auto* s = find_station(id);
  if (s == nullptr)
  {
    throw std::out_of_range("Graph::get_station: id '" + id + "' not found");
  }
  return *s;
}

const Node* Graph::find_node(const std::string& id) const noexcept
{
  auto it = node_index_.find(id);
  if (it == node_index_.end()) return nullptr;
  const auto [li, ni] = it->second;
  return &lif_.layouts[li].nodes[ni];
}

const Edge* Graph::find_edge(const std::string& id) const noexcept
{
  auto it = edge_index_.find(id);
  if (it == edge_index_.end()) return nullptr;
  const auto [li, ei] = it->second;
  return &lif_.layouts[li].edges[ei];
}

const Station* Graph::find_station(const std::string& id) const noexcept
{
  auto it = station_index_.find(id);
  if (it == station_index_.end()) return nullptr;
  const auto [li, si] = it->second;
  return &lif_.layouts[li].stations[si];
}

const Layout* Graph::find_layout(const std::string& layout_id) const noexcept
{
  for (const auto& layout : lif_.layouts)
  {
    if (layout.layout_id == layout_id) return &layout;
  }
  return nullptr;
}

const std::vector<std::string>& Graph::outbound_edges(
  const std::string& node_id) const
{
  auto it = node_outbound_.find(node_id);
  if (it == node_outbound_.end())
  {
    throw std::out_of_range(
      "Graph::outbound_edges: node '" + node_id + "' not found");
  }
  return it->second;
}

const std::vector<std::string>& Graph::inbound_edges(
  const std::string& node_id) const
{
  auto it = node_inbound_.find(node_id);
  if (it == node_inbound_.end())
  {
    throw std::out_of_range(
      "Graph::inbound_edges: node '" + node_id + "' not found");
  }
  return it->second;
}

std::vector<std::string> Graph::edges_between(
  const std::string& from_node_id, const std::string& to_node_id) const
{
  std::vector<std::string> out;
  const auto& outbound = outbound_edges(from_node_id);
  for (const auto& edge_id : outbound)
  {
    if (get_edge(edge_id).end_node_id == to_node_id)
    {
      out.push_back(edge_id);
    }
  }
  return out;
}

std::size_t Graph::node_count() const noexcept
{
  return node_index_.size();
}

std::size_t Graph::edge_count() const noexcept
{
  return edge_index_.size();
}

std::size_t Graph::station_count() const noexcept
{
  return station_index_.size();
}

bool Graph::empty() const noexcept
{
  return node_index_.empty() && edge_index_.empty() && station_index_.empty();
}

std::vector<std::string> Graph::unconnected_nodes() const
{
  // Collect station-referenced node IDs first (for O(1) lookup below).
  std::unordered_set<std::string> station_refs;
  for (const auto& layout : lif_.layouts)
  {
    for (const auto& station : layout.stations)
    {
      for (const auto& nid : station.interaction_node_ids)
      {
        station_refs.insert(nid);
      }
    }
  }

  std::vector<std::string> result;
  for (const auto& layout : lif_.layouts)
  {
    for (const auto& node : layout.nodes)
    {
      const auto& out = node_outbound_.at(node.node_id);
      const auto& in = node_inbound_.at(node.node_id);
      if (
        out.empty() && in.empty() &&
        station_refs.find(node.node_id) == station_refs.end())
      {
        result.push_back(node.node_id);
      }
    }
  }
  return result;
}

namespace {

// Find layout index by id in lif_.layouts; returns lif.layouts.size() on miss.
std::size_t find_layout_index(const LIF& lif, const std::string& layout_id)
{
  for (std::size_t i = 0; i < lif.layouts.size(); ++i)
  {
    if (lif.layouts[i].layout_id == layout_id) return i;
  }
  return lif.layouts.size();
}

}  // namespace

void Graph::add_node(Node node, const std::string& layout_id)
{
  if (has_node(node.node_id))
  {
    throw std::invalid_argument(
      "Graph::add_node: duplicate nodeId '" + node.node_id + "'");
  }
  const std::size_t li = find_layout_index(lif_, layout_id);
  if (li == lif_.layouts.size())
  {
    throw std::invalid_argument(
      "Graph::add_node: layout '" + layout_id + "' not found");
  }
  lif_.layouts[li].nodes.push_back(std::move(node));
  rebuild_indices_();
}

void Graph::add_edge(Edge edge, const std::string& layout_id)
{
  if (has_edge(edge.edge_id))
  {
    throw std::invalid_argument(
      "Graph::add_edge: duplicate edgeId '" + edge.edge_id + "'");
  }
  const std::size_t li = find_layout_index(lif_, layout_id);
  if (li == lif_.layouts.size())
  {
    throw std::invalid_argument(
      "Graph::add_edge: layout '" + layout_id + "' not found");
  }
  // startNodeId must be in the specified layout.
  bool start_in_layout = false;
  for (const auto& n : lif_.layouts[li].nodes)
  {
    if (n.node_id == edge.start_node_id)
    {
      start_in_layout = true;
      break;
    }
  }
  if (!start_in_layout)
  {
    throw std::invalid_argument(
      "Graph::add_edge: startNodeId '" + edge.start_node_id +
      "' not in layout '" + layout_id + "'");
  }
  // endNodeId can be in any layout (cross-layout elevator transitions).
  if (!has_node(edge.end_node_id))
  {
    throw std::invalid_argument(
      "Graph::add_edge: endNodeId '" + edge.end_node_id +
      "' does not exist in any layout");
  }
  lif_.layouts[li].edges.push_back(std::move(edge));
  rebuild_indices_();
}

void Graph::add_station(Station station, const std::string& layout_id)
{
  if (has_station(station.station_id))
  {
    throw std::invalid_argument(
      "Graph::add_station: duplicate stationId '" + station.station_id + "'");
  }
  const std::size_t li = find_layout_index(lif_, layout_id);
  if (li == lif_.layouts.size())
  {
    throw std::invalid_argument(
      "Graph::add_station: layout '" + layout_id + "' not found");
  }
  for (const auto& nid : station.interaction_node_ids)
  {
    if (!has_node(nid))
    {
      throw std::invalid_argument(
        "Graph::add_station: interactionNodeId '" + nid +
        "' does not exist in any layout");
    }
  }
  lif_.layouts[li].stations.push_back(std::move(station));
  rebuild_indices_();
}

void Graph::delete_node(const std::string& id)
{
  if (!has_node(id))
  {
    throw std::invalid_argument(
      "Graph::delete_node: nodeId '" + id + "' not found");
  }
  // Check no edge references this node.
  std::vector<std::string> ref_edges;
  for (const auto& layout : lif_.layouts)
  {
    for (const auto& edge : layout.edges)
    {
      if (edge.start_node_id == id || edge.end_node_id == id)
      {
        ref_edges.push_back(edge.edge_id);
      }
    }
  }
  if (!ref_edges.empty())
  {
    std::string list;
    for (const auto& e : ref_edges)
    {
      list += (list.empty() ? "" : ", ") + e;
    }
    throw std::invalid_argument(
      "Graph::delete_node: nodeId '" + id +
      "' is referenced by edges: " + list);
  }
  // Check no station references it.
  std::vector<std::string> ref_stations;
  for (const auto& layout : lif_.layouts)
  {
    for (const auto& station : layout.stations)
    {
      for (const auto& nid : station.interaction_node_ids)
      {
        if (nid == id)
        {
          ref_stations.push_back(station.station_id);
          break;
        }
      }
    }
  }
  if (!ref_stations.empty())
  {
    std::string list;
    for (const auto& s : ref_stations)
    {
      list += (list.empty() ? "" : ", ") + s;
    }
    throw std::invalid_argument(
      "Graph::delete_node: nodeId '" + id +
      "' is referenced by stations: " + list);
  }

  const auto [li, ni] = node_index_.at(id);
  auto& nodes = lif_.layouts[li].nodes;
  nodes.erase(nodes.begin() + static_cast<std::ptrdiff_t>(ni));
  rebuild_indices_();
}

void Graph::delete_edge(const std::string& id)
{
  if (!has_edge(id))
  {
    throw std::invalid_argument(
      "Graph::delete_edge: edgeId '" + id + "' not found");
  }
  const auto [li, ei] = edge_index_.at(id);
  auto& edges = lif_.layouts[li].edges;
  edges.erase(edges.begin() + static_cast<std::ptrdiff_t>(ei));
  rebuild_indices_();
}

void Graph::delete_station(const std::string& id)
{
  if (!has_station(id))
  {
    throw std::invalid_argument(
      "Graph::delete_station: stationId '" + id + "' not found");
  }
  const auto [li, si] = station_index_.at(id);
  auto& stations = lif_.layouts[li].stations;
  stations.erase(stations.begin() + static_cast<std::ptrdiff_t>(si));
  rebuild_indices_();
}

void Graph::update_node_id(const std::string& old_id, const std::string& new_id)
{
  if (!has_node(old_id))
  {
    throw std::invalid_argument(
      "Graph::update_node_id: nodeId '" + old_id + "' not found");
  }
  if (old_id == new_id) return;
  if (has_node(new_id))
  {
    throw std::invalid_argument(
      "Graph::update_node_id: new nodeId '" + new_id + "' already exists");
  }
  // Rewrite the node itself.
  const auto [li, ni] = node_index_.at(old_id);
  lif_.layouts[li].nodes[ni].node_id = new_id;
  // Rewrite all edge references.
  for (auto& layout : lif_.layouts)
  {
    for (auto& edge : layout.edges)
    {
      if (edge.start_node_id == old_id) edge.start_node_id = new_id;
      if (edge.end_node_id == old_id) edge.end_node_id = new_id;
    }
    for (auto& station : layout.stations)
    {
      for (auto& nid : station.interaction_node_ids)
      {
        if (nid == old_id) nid = new_id;
      }
    }
  }
  rebuild_indices_();
}

void Graph::update_edge_id(const std::string& old_id, const std::string& new_id)
{
  if (!has_edge(old_id))
  {
    throw std::invalid_argument(
      "Graph::update_edge_id: edgeId '" + old_id + "' not found");
  }
  if (old_id == new_id) return;
  if (has_edge(new_id))
  {
    throw std::invalid_argument(
      "Graph::update_edge_id: new edgeId '" + new_id + "' already exists");
  }
  const auto [li, ei] = edge_index_.at(old_id);
  lif_.layouts[li].edges[ei].edge_id = new_id;
  rebuild_indices_();
}

void Graph::update_station_id(
  const std::string& old_id, const std::string& new_id)
{
  if (!has_station(old_id))
  {
    throw std::invalid_argument(
      "Graph::update_station_id: stationId '" + old_id + "' not found");
  }
  if (old_id == new_id) return;
  if (has_station(new_id))
  {
    throw std::invalid_argument(
      "Graph::update_station_id: new stationId '" + new_id +
      "' already exists");
  }
  const auto [li, si] = station_index_.at(old_id);
  lif_.layouts[li].stations[si].station_id = new_id;
  rebuild_indices_();
}

std::vector<std::string> Graph::prune()
{
  auto victims = unconnected_nodes();
  for (const auto& id : victims)
  {
    delete_node(id);
  }
  return victims;
}

bool Graph::operator==(const Graph& rhs) const
{
  return lif_ == rhs.lif_;
}

bool Graph::operator!=(const Graph& rhs) const
{
  return !(*this == rhs);
}

void Graph::dump_dot(std::ostream& os) const
{
  os << "digraph G {\n";
  // Deterministic ordering for stable output (tests + diffability).
  std::map<std::string, const Node*> ordered_nodes;
  for (const auto& layout : lif_.layouts)
  {
    for (const auto& n : layout.nodes)
    {
      ordered_nodes.emplace(n.node_id, &n);
    }
  }
  for (const auto& [id, _] : ordered_nodes)
  {
    os << "  \"" << id << "\";\n";
  }
  std::map<std::string, const Edge*> ordered_edges;
  for (const auto& layout : lif_.layouts)
  {
    for (const auto& e : layout.edges)
    {
      ordered_edges.emplace(e.edge_id, &e);
    }
  }
  for (const auto& [id, ep] : ordered_edges)
  {
    os << "  \"" << ep->start_node_id << "\" -> \"" << ep->end_node_id
       << "\" [label=\"" << id << "\"];\n";
  }
  os << "}\n";
}

}  // namespace vda5050_core::layout
