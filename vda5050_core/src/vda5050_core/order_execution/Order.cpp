/**
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

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "vda5050_core/order_execution/Node.hpp"
#include "vda5050_core/order_execution/Edge.hpp"
#include "vda5050_core/order_execution/Order.hpp"

namespace vda5050_core {
namespace order {

/// TODO: Look into using move semantics here
Order::Order(std::string order_id, uint32_t order_update_id, std::vector<node::Node> nodes, std::vector<edge::Edge> edges) 
    : order_id_ {order_id}
    , order_update_id_ {order_update_id}
    , nodes_ {nodes}
    , edges_ {edges}
{
    populate_graph();
    populate_base();
    populate_horizon();
};

void Order::set_horizon(std::vector<order_graph_element::OrderGraphElement>& new_horizon)
{
    horizon_ = new_horizon;
}

void Order::stitch_and_set_order_update_id(order::Order order)
{
    try
    {
        stitch_order(order);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    set_order_update_id(order.order_update_id());
}

void Order::stitch_order(order::Order order)
{
    /// stitch this order with another order. stitching node could be horizon or base node. 
    /// TODO: (shawnkchan) should we still check that the stitchin node matches? This is likely only ever called by OrderManager, which will check that the stiching node id matches.

    /// check that decision point of this order matches with the first node of the incoming order and if the first node of the incoming order is released
    node::Node stitching_node {order.nodes().front()};
    if (decision_point().node_id() != stitching_node.node_id() || !stitching_node.released())
    {
        throw std::runtime_error("Order error: Stitching node of the incoming order does not match the current order's decision point.");
    }

    /// concatenate the current and incoming graphs
    graph_.pop_back(); /// pop the last node from the current graph
    graph_.insert(graph_.end(), order.graph().begin(), order.graph().end());
    
    /// update the base and horizon of the current order
    populate_base();
    populate_horizon();
}

void Order::set_order_update_id(uint32_t new_order_update_id)
{
    order_update_id_ = new_order_update_id;
}

void Order::populate_graph()
{
    for (node::Node n : nodes_)
    {
        graph_.push_back(n);
    }

    for (edge::Edge e : edges_)
    {
        graph_.push_back(e);
    }

    std::sort(graph_.begin(), graph_.end());    
}

void Order::populate_base()
{
    std::vector<order_graph_element::OrderGraphElement> base_temp {};

    for (order_graph_element::OrderGraphElement graph_element : graph_)
    {
        if (!graph_element.released())
        {
            break;
        }
        
        base_temp.push_back(graph_element);
    }

    base_ = base_temp;
}

void Order::populate_horizon()
{
    std::vector<order_graph_element::OrderGraphElement> horizon_temp {};
    /// assuming that we're more likely to have fewer unreleased nodes than released nodes, but i may be overthinking this
    for (auto it = graph_.rbegin(); it != graph_.rend(); ++it)
    {
        if (it->released())
        {
            break;
        }

        horizon_temp.insert(horizon_temp.begin(), *it);
    }

    horizon_ = horizon_temp;
}

} // namespace order
} // namespace vda5050_core

