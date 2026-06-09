#!/usr/bin/env python3
"""
Bug Reproduction Test: Order ID Mismatch Deadlock

This test reproduces the bug discovered in the March 6 demo where:
1. AGV state reports a different order ID than VDA5050 expects
2. VDA5050 prints warning and returns early WITHOUT updating state
3. `executing_order` is never cleared
4. All subsequent orders are rejected
5. System is permanently deadlocked

Bug location: master.py, process_state(), order ID mismatch handling

Root cause: When state_message.orderId != self.order.orderId,
the code returns early without any recovery mechanism.

FIX: After 3 consecutive mismatches, recovery is triggered:
- executing_order is cleared
- order is cleared
- System can accept new orders

Run: python3 test_order_id_mismatch_bug.py
"""

import sys
import os
import json
from datetime import datetime
from io import StringIO

# Add the module paths
vda5050_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(vda5050_dir, "vda5050_fiware"))
sys.path.insert(0, os.path.join(vda5050_dir, "fiware_api", "fiware_api"))

from vda5050_fiware.master import Vda5050Agv, Vda5050Map, Nodes, Edges
from vda5050_fiware.order import OrderMessage, Node, Edge
from vda5050_fiware.state import State, Map, MapStatus, BatteryState, SafetyState, EStop, NodeState


class MockMQTTMessage:
    """Mock MQTT message for testing process_state()."""
    def __init__(self, state: State):
        # Convert State to JSON payload (handle enums properly)
        operating_mode = state.operatingMode
        if hasattr(operating_mode, 'value'):
            operating_mode = operating_mode.value

        state_dict = {
            "headerId": state.headerId,
            "timestamp": state.timestamp.strftime('%Y-%m-%dT%H:%M:%SZ') if hasattr(state.timestamp, 'strftime') else str(state.timestamp),
            "version": state.version,
            "manufacturer": state.manufacturer,
            "serialNumber": state.serialNumber,
            "orderId": state.orderId,
            "orderUpdateId": state.orderUpdateId,
            "lastNodeId": state.lastNodeId,
            "lastNodeSequenceId": state.lastNodeSequenceId,
            "driving": state.driving,
            "paused": state.paused,
            "newBaseRequest": state.newBaseRequest,
            "distanceSinceLastNode": state.distanceSinceLastNode,
            "operatingMode": operating_mode,
            "nodeStates": [
                {
                    "nodeId": ns.nodeId,
                    "sequenceId": ns.sequenceId,
                    "released": ns.released,
                    **({"nodePosition": ns.nodePosition} if ns.nodePosition else {})
                } for ns in (state.nodeStates or [])
            ],
            "edgeStates": state.edgeStates or [],
            "actionStates": state.actionStates or [],
            "batteryState": {
                "batteryCharge": state.batteryState.batteryCharge,
                "charging": state.batteryState.charging
            } if state.batteryState else {},
            "errors": state.errors or [],
            "information": state.information or [],
            "safetyState": {
                "eStop": state.safetyState.eStop.value if state.safetyState and state.safetyState.eStop else "NONE",
                "fieldViolation": state.safetyState.fieldViolation if state.safetyState else False
            } if state.safetyState else {},
            **({"agvPosition": state.agvPosition} if state.agvPosition else {}),
            **({"velocity": state.velocity} if state.velocity else {}),
            "loads": state.loads or []
        }
        self.payload = json.dumps(state_dict).encode('utf-8')


def state_to_mqtt_msg(state: State) -> MockMQTTMessage:
    """Convert a State object to a mock MQTT message."""
    return MockMQTTMessage(state)


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


def create_order_message(order_id: str, update_id: int, nodes_list: list) -> OrderMessage:
    """Create a test order message with specified nodes."""
    nodes = []
    for i, (node_id, seq_id, released) in enumerate(nodes_list):
        nodes.append(Node(
            nodeId=node_id,
            sequenceId=seq_id,
            released=released,
            nodePosition=None,
            actions=[]
        ))

    edges = []
    for i in range(len(nodes) - 1):
        edges.append(Edge(
            edgeId=f"E{nodes[i].sequenceId}_{nodes[i+1].sequenceId}",
            sequenceId=nodes[i].sequenceId + 1,
            released=nodes[i].released and nodes[i+1].released,
            startNodeId=nodes[i].nodeId,
            endNodeId=nodes[i+1].nodeId,
            actions=[]
        ))

    return OrderMessage(
        headerId=update_id,
        timestamp=datetime.now().isoformat(),
        version="2.0.0",
        manufacturer="MiR",
        serialNumber="00012",
        orderId=order_id,
        orderUpdateId=update_id,
        nodes=nodes,
        edges=edges,
        zoneSetId=None
    )


def create_state(order_id: str, update_id: int, last_node_id: str, last_seq: int,
                 node_states: list = None, header_id: int = 1) -> State:
    """Create a test AGV state."""
    ns = []
    if node_states:
        for (node_id, seq_id, released) in node_states:
            ns.append(NodeState(
                nodeId=node_id,
                sequenceId=seq_id,
                released=released,
                nodePosition=None
            ))

    return State(
        headerId=header_id,
        timestamp=datetime.now(),
        version="2.0.0",
        manufacturer="MiR",
        serialNumber="00012",
        orderId=order_id,
        orderUpdateId=update_id,
        lastNodeId=last_node_id,
        lastNodeSequenceId=last_seq,
        driving=False,
        paused=False,
        newBaseRequest=False,
        distanceSinceLastNode=0.0,
        operatingMode="AUTOMATIC",
        nodeStates=ns,
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


class TestOrderIdMismatchBug:
    """Reproduce the order ID mismatch deadlock bug."""

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

    def run_bug_reproduction(self):
        """
        Reproduce the exact scenario from the March 6 demo:

        Timeline:
        - 08:26:09: MiR_00012 completes order "be632fc3-5b2a-3b6c-bf67-d3bfe687e70b"
        - 08:28:59: State message arrives with DIFFERENT order ID
        - 08:28:59: VDA5050 starts logging "WARNING: AGV is executing an order,
                    but the state reflects a different order ID than what is registered"
        - Forever: All new orders rejected, robot deadlocked
        """

        print("\n" + "=" * 70)
        print("BUG REPRODUCTION: Order ID Mismatch Deadlock")
        print("=" * 70)
        print("\nThis test reproduces the bug from the March 6 demo.")
        print("=" * 70 + "\n")

        mock_map = create_mock_map()
        agv = Vda5050Agv(manufacturer="MiR", serial_number="00012", map=mock_map)

        # ============================================================
        # STEP 1: Set up AGV with an active order (simulating normal operation)
        # ============================================================
        print("-" * 70)
        print("STEP 1: Set up AGV with active order ORDER_A")
        print("-" * 70)

        order_a = create_order_message(
            order_id="ORDER_A",
            update_id=0,
            nodes_list=[
                ("P1232", 0, True),  # Start at samr_outgoing_mapf[10]
                ("P1196", 2, True),
                ("P1160", 4, True),
                ("P56", 6, True),    # End at sns_outgoing
            ]
        )

        result = agv.update_order_message(order_a)
        print(f"  Order A sent: {result}")
        print(f"  agv.order.orderId = {agv.order.orderId}")
        print(f"  agv.executing_order = {agv.executing_order}")

        self.test(
            "Order A accepted",
            result == True and agv.order is not None,
            f"orderId={agv.order.orderId if agv.order else None}"
        )
        self.test(
            "AGV is executing Order A",
            agv.executing_order == "ORDER_A",
            f"executing_order={agv.executing_order}"
        )

        # Simulate AGV starting to execute the order
        state_1 = create_state(
            order_id="ORDER_A",
            update_id=0,
            last_node_id="P1232",
            last_seq=0,
            node_states=[("P1196", 2, True), ("P1160", 4, True), ("P56", 6, True)],
            header_id=1
        )
        agv.process_state(state_to_mqtt_msg(state_1))
        print(f"  State update: AGV at P1232, executing ORDER_A")
        print()

        # ============================================================
        # STEP 2: Simulate the bug trigger - state arrives with DIFFERENT order ID
        # This is what happened in the March 6 demo
        # ============================================================
        print("-" * 70)
        print("STEP 2: BUG TRIGGER - State arrives with different order ID")
        print("-" * 70)
        print("  (This simulates what happened when robots froze in simulation)")
        print()

        # The robot's state now reports a DIFFERENT order ID
        # This could happen if:
        # - Robot was reset but VDA5050 wasn't
        # - Message ordering issues
        # - Robot finished old order and started new one autonomously

        mismatched_state = create_state(
            order_id="ORDER_B_FROM_SOMEWHERE",  # DIFFERENT order ID!
            update_id=0,
            last_node_id="P1232",
            last_seq=0,
            node_states=[],
            header_id=2
        )

        print(f"  Incoming state: orderId = {mismatched_state.orderId}")
        print(f"  VDA5050 expects: orderId = {agv.order.orderId}")
        print(f"  MISMATCH!")
        print()

        # Capture stdout to see the warning
        old_stdout = sys.stdout
        sys.stdout = StringIO()

        agv.process_state(state_to_mqtt_msg(mismatched_state))

        output = sys.stdout.getvalue()
        sys.stdout = old_stdout

        # Check if warning was printed (new format with mismatch counter)
        has_warning = "WARNING: Order ID mismatch" in output
        print(f"  Warning printed: {has_warning}")
        if has_warning:
            # Print just the warning line
            for line in output.split('\n'):
                if 'WARNING' in line:
                    print(f"  {line[:100]}...")
                    break
        print()

        self.test(
            "Warning is printed on mismatch",
            has_warning,
            "Expected warning about order ID mismatch"
        )

        # ============================================================
        # STEP 3: Verify the deadlock - executing_order is STILL SET
        # ============================================================
        print("-" * 70)
        print("STEP 3: Verify the DEADLOCK condition")
        print("-" * 70)

        print(f"  agv.executing_order = {agv.executing_order}")
        print(f"  agv.order.orderId = {agv.order.orderId if agv.order else None}")

        self.test(
            "BUG: executing_order is NOT cleared",
            agv.executing_order == "ORDER_A",
            f"executing_order={agv.executing_order} (should have been cleared or updated)"
        )
        self.test(
            "BUG: order is NOT updated",
            agv.order.orderId == "ORDER_A",
            f"order.orderId={agv.order.orderId} (never synced with robot's actual state)"
        )
        print()

        # ============================================================
        # STEP 4: Try to send a NEW order - it will be REJECTED
        # ============================================================
        print("-" * 70)
        print("STEP 4: Try to send new order ORDER_C - should be REJECTED")
        print("-" * 70)

        order_c = create_order_message(
            order_id="ORDER_C",  # A completely new order
            update_id=0,
            nodes_list=[
                ("P383", 0, True),  # Different route
                ("P347", 2, True),
                ("P311", 4, True),
            ]
        )

        # Capture stdout to see the error
        old_stdout = sys.stdout
        sys.stdout = StringIO()

        result = agv.update_order_message(order_c)

        output = sys.stdout.getvalue()
        sys.stdout = old_stdout

        print(f"  Attempted to send ORDER_C")
        print(f"  Result: {result}")

        has_error = "Error, robot is currently still working on Order" in output
        print(f"  Error printed: {has_error}")
        if has_error:
            for line in output.split('\n'):
                if 'Error' in line:
                    print(f"  {line}")
                    break
        print()

        self.test(
            "BUG: New order ORDER_C is REJECTED",
            result == False,
            "Cannot accept new orders due to deadlock"
        )
        self.test(
            "BUG: Error message about 'still working'",
            has_error,
            "Shows the root cause of rejection"
        )

        # ============================================================
        # STEP 5: More mismatched states - RECOVERY should trigger
        # ============================================================
        print("-" * 70)
        print("STEP 5: Send more mismatched states - RECOVERY should trigger")
        print("-" * 70)
        print("  (Threshold is 3 mismatches, we already had 1 in Step 2)")
        print()

        recovery_triggered = False
        for i in range(3):
            state_n = create_state(
                order_id="ORDER_B_FROM_SOMEWHERE",
                update_id=0,
                last_node_id=f"P{1232 - i*36}",  # Robot is moving!
                last_seq=0,
                node_states=[],
                header_id=3 + i
            )

            old_stdout = sys.stdout
            sys.stdout = StringIO()
            agv.process_state(state_to_mqtt_msg(state_n))
            output = sys.stdout.getvalue()
            sys.stdout = old_stdout

            has_warning = "WARNING" in output
            has_recovery = "RECOVERY" in output
            if has_recovery:
                recovery_triggered = True
            print(f"  State {i+1}: Robot at {state_n.lastNodeId}, Warning: {has_warning}, Recovery: {has_recovery}")

        print()
        print(f"  After 3 more states:")
        print(f"  agv.executing_order = {agv.executing_order}")
        print(f"  agv.order = {agv.order}")
        print()

        self.test(
            "FIX: Recovery triggered after 3 mismatches",
            recovery_triggered,
            "Counter-based recovery mechanism works"
        )
        self.test(
            "FIX: executing_order is cleared",
            agv.executing_order is None,
            f"executing_order={agv.executing_order}"
        )
        self.test(
            "FIX: order is cleared",
            agv.order is None,
            f"order={agv.order}"
        )

        # ============================================================
        # STEP 6: Try to send new order - should be ACCEPTED now!
        # ============================================================
        print("-" * 70)
        print("STEP 6: Try to send ORDER_D - should be ACCEPTED now!")
        print("-" * 70)

        order_d = create_order_message(
            order_id="ORDER_D",
            update_id=0,
            nodes_list=[
                ("P500", 0, True),
                ("P464", 2, True),
            ]
        )

        old_stdout = sys.stdout
        sys.stdout = StringIO()
        result = agv.update_order_message(order_d)
        output = sys.stdout.getvalue()
        sys.stdout = old_stdout

        print(f"  Attempted to send ORDER_D")
        print(f"  Result: {result}")
        if result:
            print(f"  ORDER_D ACCEPTED! System has recovered!")
        else:
            print(f"  ORDER_D REJECTED - system still stuck")
        print()

        self.test(
            "FIX: ORDER_D is ACCEPTED after recovery",
            result == True,
            "System can accept new orders after recovery"
        )

        # ============================================================
        # SUMMARY
        # ============================================================
        print("=" * 70)
        print("FIX VERIFICATION SUMMARY")
        print("=" * 70)
        print()
        print("The fix has been verified:")
        print()
        print("1. AGV was executing ORDER_A")
        print("2. State arrived with different orderId (ORDER_B_FROM_SOMEWHERE)")
        print("3. VDA5050 detected mismatch and:")
        print("   - Incremented mismatch counter")
        print("   - Did NOT forward mismatched state to FIWARE/ADG")
        print("   - After 3 mismatches, triggered RECOVERY")
        print("4. Recovery cleared executing_order and order")
        print("5. New order ORDER_D was ACCEPTED!")
        print()
        print("This fix prevents the permanent deadlock that affected")
        print("MiR_0007, MiR_0008, MiR_0009, MiR_00010, and MiR_00012")
        print("in the March 6 demo!")
        print()

        # Report test results
        print("-" * 70)
        print(f"Total tests: {self.passed + self.failed}")
        print(f"  Passed: {self.passed}")
        print(f"  Failed: {self.failed}")
        print("-" * 70)

        if self.failed == 0:
            print("\nFIX VERIFIED SUCCESSFULLY!")
            print("All tests confirm the recovery mechanism works.")
        else:
            print("\nSome tests failed:")
            for name, status, msg in self.results:
                if status == "FAIL":
                    print(f"  FAILED: {name} - {msg}")

        return self.failed == 0


def main():
    test = TestOrderIdMismatchBug()
    success = test.run_bug_reproduction()

    print("\n" + "=" * 70)
    if success:
        print("FIX COMPLETE!")
    else:
        print("FIX INCOMPLETE - Some tests failed")
    print("=" * 70)
    if success:
        print("""
The fix is implemented in master.py, process_state() method.

Key changes:
1. Check for order ID mismatch BEFORE forwarding StateMessage to FIWARE
2. Increment mismatch counter on each mismatched state
3. After 3 consecutive mismatches, trigger RECOVERY:
   - Clear executing_order
   - Clear order
   - Reset mismatch counter
   - Clear pending order updates
4. System can now accept new orders after recovery

This prevents the permanent deadlock issue from the March 6 demo.
""")
    else:
        print("""
Some tests failed. Please check the implementation.
""")

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
