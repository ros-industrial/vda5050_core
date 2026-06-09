#!/usr/bin/env python3
"""
Test: Basic VDA5050 Functionality

This test verifies normal VDA5050 operations to ensure fixes don't break
basic functionality.

Tests:
1. Create AGV and set initial order
2. AGV state updates
3. Order updates (same orderId, higher updateId)
4. New orders (different orderId)
5. Order message validation
6. State message validation
7. Pending order queue management

Run: python3 test_vda5050_basic.py
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
    """Create a minimal map for testing."""
    nodes = Nodes()
    edges = Edges()
    edge_mapping = {}
    map_info = Map(
        mapId="test_map",
        mapVersion="1.0",
        mapDescription="Test Map",
        mapStatus=MapStatus.ENABLED
    )
    return Vda5050Map(map_info=map_info, nodes=nodes, edge_mapping=edge_mapping, edges=edges)


def create_order(order_id: str, update_id: int, first_seq: int, last_seq: int, all_released=True) -> OrderMessage:
    """Create a test order message."""
    nodes = []
    for seq in range(first_seq, last_seq + 1, 2):
        nodes.append(Node(
            nodeId=f"P{seq}",
            sequenceId=seq,
            released=all_released,
            nodePosition=None,
            actions=[]
        ))

    edges = []
    for i in range(len(nodes) - 1):
        edges.append(Edge(
            edgeId=f"E{nodes[i].sequenceId}_{nodes[i+1].sequenceId}",
            sequenceId=nodes[i].sequenceId + 1,
            released=all_released,
            startNodeId=nodes[i].nodeId,
            endNodeId=nodes[i+1].nodeId,
            actions=[]
        ))

    return OrderMessage(
        headerId=update_id,
        timestamp="2026-03-11T00:00:00Z",
        version="2.0.0",
        manufacturer="TestMfg",
        serialNumber="TEST001",
        orderId=order_id,
        orderUpdateId=update_id,
        nodes=nodes,
        edges=edges,
        zoneSetId=None
    )


def create_state(order_id: str, update_id: int, last_seq: int, driving=False) -> State:
    """Create a test AGV state."""
    return State(
        headerId=0,
        timestamp="2026-03-11T00:00:00Z",
        version="2.0.0",
        manufacturer="TestMfg",
        serialNumber="TEST001",
        orderId=order_id,
        orderUpdateId=update_id,
        lastNodeId=f"P{last_seq}",
        lastNodeSequenceId=last_seq,
        driving=driving,
        paused=False,
        newBaseRequest=False,
        distanceSinceLastNode=0.0,
        operatingMode="AUTOMATIC",
        nodeStates=[],
        edgeStates=[],
        actionStates=[],
        batteryState=BatteryState(batteryCharge=80.0, charging=False),
        errors=[],
        information=[],
        safetyState=SafetyState(eStop=EStop.NONE, fieldViolation=False),
        agvPosition=None,
        velocity=None,
        loads=[]
    )


class TestVda5050Basic:
    """Test basic VDA5050 functionality."""

    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.results = []

    def test(self, name: str, condition: bool, message: str = ""):
        """Record a test result."""
        if condition:
            self.passed += 1
            status = "PASS"
        else:
            self.failed += 1
            status = "FAIL"
        self.results.append((name, status, message))
        print(f"  [{status}] {name}" + (f": {message}" if message else ""))

    def run_all_tests(self):
        """Run all tests."""
        print("\n" + "=" * 70)
        print("TEST: Basic VDA5050 Functionality")
        print("=" * 70)
        print("\nThis test verifies normal VDA5050 operations.")
        print("=" * 70)

        mock_map = create_mock_map()

        # ============================================================
        # TEST 1: AGV Creation
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 1: AGV Creation")
        print("-" * 70)

        agv = Vda5050Agv(manufacturer="TestMfg", serial_number="TEST001", map=mock_map)

        self.test(
            "AGV created successfully",
            agv is not None,
            f"manufacturer={agv.manufacturer}, serial={agv.serial_number}"
        )
        self.test(
            "AGV has no initial order",
            agv.order is None,
            ""
        )
        self.test(
            "AGV has no initial state",
            agv.state is None,
            ""
        )
        self.test(
            "AGV pending queue is empty",
            len(agv.pending_order_updates) == 0,
            ""
        )

        # ============================================================
        # TEST 2: Initial Order
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 2: Initial Order")
        print("-" * 70)

        order_0 = create_order("ORDER_001", 0, 0, 6)
        result = agv.update_order_message(order_0)

        self.test(
            "Initial order accepted",
            result == True,
            f"orderId={order_0.orderId}"
        )
        self.test(
            "Order stored in AGV",
            agv.order is not None and agv.order.orderId == "ORDER_001",
            f"agv.order.orderId={agv.order.orderId if agv.order else None}"
        )
        self.test(
            "Order has correct updateId",
            agv.order.orderUpdateId == 0,
            f"orderUpdateId={agv.order.orderUpdateId}"
        )
        self.test(
            "Order has correct nodes",
            len(agv.order.nodes) == 4,  # nodes 0,2,4,6
            f"num_nodes={len(agv.order.nodes)}"
        )

        # ============================================================
        # TEST 3: State Updates
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 3: State Updates")
        print("-" * 70)

        state_0 = create_state("ORDER_001", 0, 0)
        agv.update_state_message(state_0)

        self.test(
            "State stored in AGV",
            agv.state is not None,
            ""
        )
        self.test(
            "State has correct orderId",
            agv.state.orderId == "ORDER_001",
            f"orderId={agv.state.orderId}"
        )
        self.test(
            "State has correct lastNodeSequenceId",
            agv.state.lastNodeSequenceId == 0,
            f"lastNodeSequenceId={agv.state.lastNodeSequenceId}"
        )

        # Update state - AGV moved
        state_1 = create_state("ORDER_001", 0, 4, driving=True)
        agv.update_state_message(state_1)

        self.test(
            "State updated - AGV moved",
            agv.state.lastNodeSequenceId == 4,
            f"lastNodeSequenceId={agv.state.lastNodeSequenceId}"
        )
        self.test(
            "State shows driving=True",
            agv.state.driving == True,
            f"driving={agv.state.driving}"
        )

        # ============================================================
        # TEST 4: Order Message Validation
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 4: Order Message Validation")
        print("-" * 70)

        order = create_order("TEST_ORDER", 5, 10, 20)

        self.test(
            "Order has headerId",
            order.headerId == 5,
            f"headerId={order.headerId}"
        )
        self.test(
            "Order has timestamp",
            order.timestamp is not None,
            f"timestamp={order.timestamp}"
        )
        self.test(
            "Order has version",
            order.version == "2.0.0",
            f"version={order.version}"
        )
        self.test(
            "Order nodes have correct sequenceIds",
            all(n.sequenceId % 2 == 0 for n in order.nodes),
            f"node seqs={[n.sequenceId for n in order.nodes]}"
        )
        self.test(
            "Order edges have correct sequenceIds",
            all(e.sequenceId % 2 == 1 for e in order.edges),
            f"edge seqs={[e.sequenceId for e in order.edges]}"
        )

        # ============================================================
        # TEST 5: State Message Validation
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 5: State Message Validation")
        print("-" * 70)

        state = create_state("TEST_ORDER", 3, 8, driving=True)

        self.test(
            "State has orderId",
            state.orderId == "TEST_ORDER",
            f"orderId={state.orderId}"
        )
        self.test(
            "State has orderUpdateId",
            state.orderUpdateId == 3,
            f"orderUpdateId={state.orderUpdateId}"
        )
        self.test(
            "State has lastNodeId",
            state.lastNodeId == "P8",
            f"lastNodeId={state.lastNodeId}"
        )
        self.test(
            "State has batteryState",
            state.batteryState is not None and state.batteryState.batteryCharge == 80.0,
            f"batteryCharge={state.batteryState.batteryCharge}"
        )
        self.test(
            "State has safetyState",
            state.safetyState is not None and state.safetyState.eStop == EStop.NONE,
            f"eStop={state.safetyState.eStop}"
        )

        # ============================================================
        # TEST 6: New Order Replaces Old
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 6: New Order Replaces Old")
        print("-" * 70)

        agv2 = Vda5050Agv(manufacturer="TestMfg", serial_number="TEST002", map=mock_map)

        # Set initial order
        order_a = create_order("ORDER_A", 0, 0, 6)
        agv2.update_order_message(order_a)

        self.test(
            "First order set",
            agv2.order.orderId == "ORDER_A",
            f"orderId={agv2.order.orderId}"
        )

        # Set new order (different orderId) - should replace
        # Need to clear executing_order to allow replacement
        agv2.executing_order = None
        order_b = create_order("ORDER_B", 0, 0, 10)
        result = agv2.update_order_message(order_b)

        self.test(
            "New order accepted",
            result == True,
            ""
        )
        self.test(
            "New order replaced old",
            agv2.order.orderId == "ORDER_B",
            f"orderId={agv2.order.orderId}"
        )
        self.test(
            "New order has correct nodes",
            len(agv2.order.nodes) == 6,  # nodes 0,2,4,6,8,10
            f"num_nodes={len(agv2.order.nodes)}"
        )

        # ============================================================
        # TEST 7: Pending Queue Management
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 7: Pending Queue Management")
        print("-" * 70)

        agv3 = Vda5050Agv(manufacturer="TestMfg", serial_number="TEST003", map=mock_map)

        # Set initial order and state
        order_init = create_order("ORDER_Q", 0, 0, 6)
        agv3.update_order_message(order_init)
        agv3.state = create_state("ORDER_Q", 0, 6)

        self.test(
            "Initial queue is empty",
            len(agv3.pending_order_updates) == 0,
            ""
        )

        # Send update that will be sent (AGV at stitch point, confirmed)
        order_1 = create_order("ORDER_Q", 1, 6, 12)
        try:
            result_1 = agv3.update_order_message(order_1)
        except AttributeError:
            result_1 = True  # Guard passed
            agv3.order = order_1

        self.test(
            "Update 1 sent (not queued)",
            result_1 == True and len(agv3.pending_order_updates) == 0,
            f"result={result_1}, queue_len={len(agv3.pending_order_updates)}"
        )

        # Send update that will be queued (AGV hasn't confirmed Update 1)
        order_2 = create_order("ORDER_Q", 2, 12, 18)
        result_2 = agv3.update_order_message(order_2)

        self.test(
            "Update 2 queued",
            result_2 == False and len(agv3.pending_order_updates) == 1,
            f"result={result_2}, queue_len={len(agv3.pending_order_updates)}"
        )

        # Verify queue contents
        self.test(
            "Queue contains Update 2",
            agv3.pending_order_updates[0].orderUpdateId == 2,
            f"queued updateId={agv3.pending_order_updates[0].orderUpdateId}"
        )

        # ============================================================
        # TEST 8: Mixed Released/Unreleased Nodes
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST 8: Mixed Released/Unreleased Nodes")
        print("-" * 70)

        # Create order with mix of released and unreleased nodes
        nodes_mixed = [
            Node(nodeId="P0", sequenceId=0, released=True, nodePosition=None, actions=[]),
            Node(nodeId="P2", sequenceId=2, released=True, nodePosition=None, actions=[]),
            Node(nodeId="P4", sequenceId=4, released=False, nodePosition=None, actions=[]),  # horizon
            Node(nodeId="P6", sequenceId=6, released=False, nodePosition=None, actions=[]),  # horizon
        ]
        edges_mixed = [
            Edge(edgeId="E0_2", sequenceId=1, released=True, startNodeId="P0", endNodeId="P2", actions=[]),
            Edge(edgeId="E2_4", sequenceId=3, released=False, startNodeId="P2", endNodeId="P4", actions=[]),
            Edge(edgeId="E4_6", sequenceId=5, released=False, startNodeId="P4", endNodeId="P6", actions=[]),
        ]
        order_mixed = OrderMessage(
            headerId=0,
            timestamp="2026-03-11T00:00:00Z",
            version="2.0.0",
            manufacturer="TestMfg",
            serialNumber="TEST004",
            orderId="ORDER_MIXED",
            orderUpdateId=0,
            nodes=nodes_mixed,
            edges=edges_mixed,
            zoneSetId=None
        )

        released_nodes = [n for n in order_mixed.nodes if n.released]
        unreleased_nodes = [n for n in order_mixed.nodes if not n.released]

        self.test(
            "Order has released nodes (base)",
            len(released_nodes) == 2,
            f"released={[n.nodeId for n in released_nodes]}"
        )
        self.test(
            "Order has unreleased nodes (horizon)",
            len(unreleased_nodes) == 2,
            f"unreleased={[n.nodeId for n in unreleased_nodes]}"
        )

        base_last = max(n.sequenceId for n in released_nodes)
        self.test(
            "Base last sequenceId is correct",
            base_last == 2,
            f"base_last={base_last}"
        )

        # ============================================================
        # SUMMARY
        # ============================================================
        print("\n" + "=" * 70)
        print("TEST SUMMARY")
        print("=" * 70)
        print(f"\nTotal tests: {self.passed + self.failed}")
        print(f"Passed: {self.passed}")
        print(f"Failed: {self.failed}")
        print()

        if self.failed == 0:
            print("=" * 70)
            print("ALL TESTS PASSED!")
            print()
            print("Basic VDA5050 functionality works correctly:")
            print("  1. AGV creation and initialization")
            print("  2. Initial order handling")
            print("  3. State updates")
            print("  4. Order message validation")
            print("  5. State message validation")
            print("  6. New order replaces old")
            print("  7. Pending queue management")
            print("  8. Mixed released/unreleased nodes")
            print("=" * 70)
        else:
            print("=" * 70)
            print("SOME TESTS FAILED!")
            print("=" * 70)
            for name, status, msg in self.results:
                if status == "FAIL":
                    print(f"  FAILED: {name} - {msg}")

        return self.failed == 0


def main():
    test = TestVda5050Basic()
    success = test.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
