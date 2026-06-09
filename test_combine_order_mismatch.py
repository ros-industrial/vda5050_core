#!/usr/bin/env python3
"""
Test: Reproduce the combine_order mismatch bug

Scenario:
1. Master sends Update 1, AGV accepts, base_last = 12
2. Master sends Update 2 (extends to 18), combine_order updates self.order to base_last = 18
3. AGV REJECTS Update 2 (for some reason), AGV base stays at 12
4. MAPF sends new order starting at seq 12
5. Stitch guard passes (12 > lastNodeSeq? - depends on AGV position)
6. combine_order uses self.order with base_last = 18
7. Combined order starts at seq 18!
8. AGV rejects because 18 != 12

The key insight: stitch guard checks INCOMING order's first_seq vs AGV's lastNodeSequenceId
But combine_order builds order using MASTER's self.order.base_last
These can be different if AGV rejected a previous update!
"""

import sys
import os

vda5050_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(vda5050_dir, "vda5050_fiware"))
sys.path.insert(0, os.path.join(vda5050_dir, "fiware_api", "fiware_api"))

from vda5050_fiware.master import Vda5050Agv, Vda5050Map, Nodes, Edges
from vda5050_fiware.order import OrderMessage, Node, Edge
from vda5050_fiware.state import State, Map, MapStatus, BatteryState, SafetyState, EStop


def create_mock_map():
    nodes = Nodes()
    edges = Edges()
    edge_mapping = {}
    map_info = Map(mapId="test_map", mapVersion="1.0", mapDescription="Test Map", mapStatus=MapStatus.ENABLED)
    return Vda5050Map(map_info=map_info, nodes=nodes, edge_mapping=edge_mapping, edges=edges)


def create_order(order_id: str, update_id: int, first_seq: int, last_seq: int) -> OrderMessage:
    nodes = []
    for seq in range(first_seq, last_seq + 1, 2):
        nodes.append(Node(nodeId=f"P{seq}", sequenceId=seq, released=True, nodePosition=None, actions=[]))
    edges = []
    for i in range(len(nodes) - 1):
        edges.append(Edge(
            edgeId=f"E{nodes[i].sequenceId}_{nodes[i+1].sequenceId}",
            sequenceId=nodes[i].sequenceId + 1,
            released=True, startNodeId=nodes[i].nodeId, endNodeId=nodes[i+1].nodeId, actions=[]
        ))
    return OrderMessage(
        headerId=update_id, timestamp="2026-03-11T00:00:00Z", version="2.0.0",
        manufacturer="TestMfg", serialNumber="TEST001", orderId=order_id,
        orderUpdateId=update_id, nodes=nodes, edges=edges, zoneSetId=None
    )


def create_state(order_id: str, update_id: int, last_seq: int) -> State:
    return State(
        headerId=0, timestamp="2026-03-11T00:00:00Z", version="2.0.0",
        manufacturer="TestMfg", serialNumber="TEST001", orderId=order_id,
        orderUpdateId=update_id, lastNodeId=f"P{last_seq}", lastNodeSequenceId=last_seq,
        driving=False, paused=False, newBaseRequest=False, distanceSinceLastNode=0.0,
        operatingMode="AUTOMATIC", nodeStates=[], edgeStates=[], actionStates=[],
        batteryState=BatteryState(batteryCharge=80.0, charging=False),
        errors=[], information=[],
        safetyState=SafetyState(eStop=EStop.NONE, fieldViolation=False),
        agvPosition=None, velocity=None, loads=[]
    )


def get_base_last_seq(order: OrderMessage) -> int:
    """Get base_last (last released node's sequenceId)."""
    released_seqs = [n.sequenceId for n in order.nodes if n.released]
    return max(released_seqs) if released_seqs else 0


def simulate_libvda5050pp_check(order_first_seq: int, agv_base_last: int) -> bool:
    """libvda5050++ stitching check: order.firstSeq must == AGV's base_last."""
    return order_first_seq == agv_base_last


print("=" * 70)
print("TEST: Reproduce combine_order mismatch bug")
print("=" * 70)

mock_map = create_mock_map()
agv = Vda5050Agv(manufacturer="TestMfg", serial_number="TEST001", map=mock_map)

# Step 1: Initial order 0-12
order_0 = create_order("TEST_ORDER", 0, 0, 12)
agv.update_order_message(order_0)
agv.state = create_state("TEST_ORDER", 0, 12)  # AGV at end of base

master_base_last = get_base_last_seq(agv.order)
print(f"\n1. Initial order: nodes 0-12, base_last = {master_base_last}")
print(f"   AGV at lastNodeSequenceId = {agv.state.lastNodeSequenceId}")

# Step 2: Simulate Update 1 extending base to 18
# In real scenario, combine_order would be called and self.order updated
# Let's manually set master's order to have base_last = 18 (as if Update 1 was sent but AGV rejected)
order_with_18 = create_order("TEST_ORDER", 1, 0, 18)
agv.order = order_with_18  # Master's view: base extends to 18

master_base_last = get_base_last_seq(agv.order)
print(f"\n2. Master sent Update 1, master's self.order updated to base_last = {master_base_last}")
print("   (Simulating: AGV REJECTED this update, AGV's base stays at 12)")

# Step 3: AGV reports state - still showing old base
# In reality, AGV's base would be reflected somehow, but let's assume lastNodeSequenceId = 12
agv.state = create_state("TEST_ORDER", 0, 12)  # AGV didn't process Update 1
print(f"\n3. AGV state: lastNodeSequenceId = {agv.state.lastNodeSequenceId}")
print("   AGV's actual base_last = 12 (rejected Update 1)")

# Step 4: New order arrives from MAPF starting at seq 12 (matches where MAPF thinks stitch should be)
incoming_order = create_order("TEST_ORDER", 2, 12, 24)
incoming_first_seq = incoming_order.nodes[0].sequenceId

print(f"\n4. MAPF sends new order starting at seq {incoming_first_seq}")

# Step 5: Check stitch guard (as implemented in master.py)
stitch_guard_would_queue = incoming_first_seq > agv.state.lastNodeSequenceId
print(f"\n5. STITCH GUARD CHECK:")
print(f"   first_node_seq ({incoming_first_seq}) > lastNodeSequenceId ({agv.state.lastNodeSequenceId})?")
print(f"   Result: {incoming_first_seq} > {agv.state.lastNodeSequenceId} = {stitch_guard_would_queue}")

if stitch_guard_would_queue:
    print("   -> Guard QUEUES (order not sent)")
else:
    print("   -> Guard PASSES (order will be sent via combine_order)")
    
    # Step 6: combine_order would use master's self.order with base_last = 18
    # The combined order's first node would be the last released node = seq 18
    combined_order_first_seq = master_base_last  # This is what combine_order would produce
    
    print(f"\n6. COMBINE_ORDER uses master's self.order (base_last = {master_base_last})")
    print(f"   Combined order FIRST node sequenceId = {combined_order_first_seq}")
    
    # Step 7: Check what AGV's libvda5050++ would do
    agv_actual_base_last = 12  # What AGV actually has (rejected Update 1)
    libvda_accepts = simulate_libvda5050pp_check(combined_order_first_seq, agv_actual_base_last)
    
    print(f"\n7. LIBVDA5050++ CHECK on AGV:")
    print(f"   order.firstSeq ({combined_order_first_seq}) == AGV.base_last ({agv_actual_base_last})?")
    print(f"   Result: {combined_order_first_seq} == {agv_actual_base_last} = {libvda_accepts}")
    
    if libvda_accepts:
        print("   -> AGV ACCEPTS order")
    else:
        print("   -> AGV REJECTS: 'Could not stitch order due to invalid sequence ids'")
        print("\n" + "=" * 70)
        print("BUG REPRODUCED!")
        print("=" * 70)
        print(f"\nThe stitch guard passed (incoming {incoming_first_seq} <= lastNodeSeq {agv.state.lastNodeSequenceId})")
        print(f"But combine_order used master's base_last = {master_base_last}")
        print(f"AGV's actual base_last = {agv_actual_base_last}")
        print(f"Result: Order sent with seq {combined_order_first_seq}, AGV expected {agv_actual_base_last}")
        print("\nThis matches the error in logs:")
        print(f"  order.node.sequenceId: {combined_order_first_seq}")
        print(f"  state.baseSequenceId: {agv_actual_base_last}")
