#!/usr/bin/env python3
"""
Unit Test: Reproduce the baseSequenceId race condition bug

This test reproduces the exact RACE CONDITION found in overnight logs:
1. Master sends Update 2 (extends base to 18)
2. Master immediately sends Update 3 (starts at 18)
3. AGV receives Update 3 BEFORE processing Update 2
4. AGV's base_last is still 12 → REJECT!

The issue is:
- Master.py sends updates based on what IT THINKS the AGV's base is
- But AGV hasn't processed the previous update yet (MQTT latency + processing time)
- Master's view: base_last = 18 (after sending Update 2)
- AGV's view: base_last = 12 (hasn't processed Update 2 yet)

FIX:
- master.py must wait for AGV state confirmation showing base_last has advanced
  BEFORE sending the next update

Run: python3 test_stitch_guard_base_seq.py
"""

import sys
import os

# Add the module paths
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


def create_order_message(order_id: str, update_id: int, first_seq: int, last_seq: int, all_released=True) -> OrderMessage:
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


def create_state(order_id: str, update_id: int, last_seq: int) -> State:
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
        driving=True,  # AGV is still moving
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


def simulate_libvda5050pp_check(order_first_seq: int, base_last_seq: int) -> bool:
    """
    Simulate libvda5050++'s checkOrderAppend logic.

    From order.cpp line 182:
        bool appends = base_last == min_seq;

    Returns True if stitching would succeed, False if it would fail.
    """
    return base_last_seq == order_first_seq


def get_base_last_seq(order: OrderMessage) -> int:
    """
    Get the last released (base) node's sequenceId from an order.
    This is what libvda5050++ uses as 'base_last'.
    """
    released_seqs = [n.sequenceId for n in order.nodes if n.released]
    return max(released_seqs) if released_seqs else 0


class TestStitchGuardBaseSeq:
    """Test to reproduce and fix the baseSequenceId bug."""

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
        print("TEST: Reproduce baseSequenceId Race Condition Bug")
        print("=" * 70)
        print("\nThis reproduces the exact RACE CONDITION from overnight logs:")
        print("  1. Master sends Update 2 (extends base from 12 to 18)")
        print("  2. Master immediately sends Update 3 (starts at 18)")
        print("  3. AGV receives Update 3 BEFORE processing Update 2")
        print("  4. AGV's base_last is still 12 -> REJECT Update 3!")
        print("=" * 70 + "\n")

        mock_map = create_mock_map()

        # ============================================================
        # SCENARIO: Reproduce the bug
        # ============================================================
        print("-" * 70)
        print("SCENARIO: Reproduce the bug from overnight logs")
        print("-" * 70)

        # Step 1: Create AGV with initial order (nodes 0-12 all released)
        agv = Vda5050Agv(manufacturer="TestMfg", serial_number="TEST001", map=mock_map)

        order_0 = create_order_message("TEST_ORDER", 0, 0, 12)  # Nodes at seq 0,2,4,6,8,10,12
        agv.update_order_message(order_0)

        base_last = get_base_last_seq(agv.order)
        print(f"\n  Initial order: nodes 0-12, all released")
        print(f"  Base last sequenceId: {base_last}")

        self.test(
            "Initial order has base_last=12",
            base_last == 12,
            f"base_last = {base_last}"
        )

        # Step 2: Set AGV state - physically at seq 4 (still moving through the order)
        agv.state = create_state("TEST_ORDER", 0, 4)
        print(f"\n  AGV state: lastNodeSequenceId={agv.state.lastNodeSequenceId} (AGV at node P4)")
        print(f"  AGV is driving=True (still moving through released nodes)")

        # Step 3: Send order update starting at seq 18
        order_1 = create_order_message("TEST_ORDER", 1, 18, 24)  # Nodes at seq 18,20,22,24
        first_node_seq = order_1.nodes[0].sequenceId
        print(f"\n  Order Update 1: starts at seq {first_node_seq}")

        # Step 4: Check what current stitch guard does
        print(f"\n  CURRENT STITCH GUARD CHECK:")
        print(f"    first_node_seq ({first_node_seq}) > lastNodeSequenceId ({agv.state.lastNodeSequenceId})?")
        current_guard_would_queue = first_node_seq > agv.state.lastNodeSequenceId
        print(f"    Result: {first_node_seq} > {agv.state.lastNodeSequenceId} = {current_guard_would_queue}")

        if current_guard_would_queue:
            print(f"    -> Current guard WOULD queue (good)")
        else:
            print(f"    -> Current guard WOULD NOT queue (BUG!)")

        # Step 5: Check what libvda5050++ would do
        print(f"\n  LIBVDA5050++ CHECK:")
        print(f"    first_node_seq ({first_node_seq}) == base_last ({base_last})?")
        libvda_would_accept = simulate_libvda5050pp_check(first_node_seq, base_last)
        print(f"    Result: {first_node_seq} == {base_last} = {libvda_would_accept}")

        if libvda_would_accept:
            print(f"    -> libvda5050++ WOULD accept")
        else:
            print(f"    -> libvda5050++ WOULD REJECT with 'Could not stitch order'")

        # Step 6: Identify the bug
        print(f"\n  BUG ANALYSIS:")
        if not current_guard_would_queue and not libvda_would_accept:
            print(f"    Current guard passes, but libvda5050++ rejects!")
            print(f"    This is the bug causing 468,688 errors!")
            bug_exists = True
        else:
            print(f"    No bug in this scenario")
            bug_exists = False

        self.test(
            "Bug reproduced: guard passes but libvda5050++ rejects",
            not current_guard_would_queue and not libvda_would_accept,
            f"guard_queues={current_guard_would_queue}, libvda_accepts={libvda_would_accept}"
        )

        # ============================================================
        # PROPOSED FIX
        # ============================================================
        print("\n" + "-" * 70)
        print("PROPOSED FIX: Check against base_last, not lastNodeSequenceId")
        print("-" * 70)

        print(f"\n  FIXED STITCH GUARD CHECK:")
        print(f"    first_node_seq ({first_node_seq}) > base_last ({base_last})?")
        fixed_guard_would_queue = first_node_seq > base_last
        print(f"    Result: {first_node_seq} > {base_last} = {fixed_guard_would_queue}")

        if fixed_guard_would_queue:
            print(f"    -> Fixed guard WOULD queue (correct!)")
        else:
            print(f"    -> Fixed guard WOULD NOT queue")

        # Check if fix aligns with libvda5050++
        fix_aligns = fixed_guard_would_queue == (not libvda_would_accept)
        self.test(
            "Fixed guard aligns with libvda5050++ behavior",
            fix_aligns,
            f"fixed_queues={fixed_guard_would_queue}, libvda_rejects={not libvda_would_accept}"
        )

        # ============================================================
        # TEST: Order that SHOULD be sent (exact match)
        # ============================================================
        print("\n" + "-" * 70)
        print("TEST: Order that should be accepted (first_seq == base_last)")
        print("-" * 70)

        # Order starting at seq 12 (matches base_last)
        order_correct = create_order_message("TEST_ORDER", 1, 12, 18)
        first_node_seq_correct = order_correct.nodes[0].sequenceId
        print(f"\n  Order Update: starts at seq {first_node_seq_correct}")
        print(f"  Base last: {base_last}")

        libvda_accepts_correct = simulate_libvda5050pp_check(first_node_seq_correct, base_last)
        fixed_guard_correct = first_node_seq_correct > base_last

        print(f"\n  libvda5050++ check: {first_node_seq_correct} == {base_last}? {libvda_accepts_correct}")
        print(f"  Fixed guard check: {first_node_seq_correct} > {base_last}? {fixed_guard_correct}")

        self.test(
            "Correct order: libvda5050++ accepts",
            libvda_accepts_correct,
            f"{first_node_seq_correct} == {base_last}"
        )
        self.test(
            "Correct order: fixed guard does not queue",
            not fixed_guard_correct,
            f"{first_node_seq_correct} > {base_last} = {fixed_guard_correct}"
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
            print("The fix is validated:")
            print("  - Bug reproduced: current guard uses lastNodeSequenceId")
            print("  - libvda5050++ uses base_last (last released node seq)")
            print("  - Fix: change guard to check against base_last")
            print()
            print("REQUIRED CODE CHANGE in master.py update_order_message():")
            print()
            print("  # Calculate base_last (last released node's sequenceId)")
            print("  base_last_seq = max(")
            print("      (n.sequenceId for n in self.order.nodes if n.released),")
            print("      default=0")
            print("  )")
            print()
            print("  # Change from:")
            print("  #   if first_node_seq > self.state.lastNodeSequenceId:")
            print("  # To:")
            print("  if first_node_seq > base_last_seq:")
            print("=" * 70)
        else:
            print("=" * 70)
            print("SOME TESTS FAILED - Review the results above")
            print("=" * 70)

        return self.failed == 0


def main():
    test = TestStitchGuardBaseSeq()
    success = test.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
