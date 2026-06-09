import json
from datetime import datetime
import paho.mqtt.client as paho
from vda5050_fiware.order import OrderMessage, Node, Edge
from vda5050_fiware.state import State, Map
from threading import Lock, Thread
import queue
import copy

from fiware_api.utils import (
    printRed,
    printGreen,
    printYellow,
    printPurple,
    printCyan,
    MQTTClient,
)


class Nodes(dict):
    def __setitem__(self, key: str, value: Node):
        key = key.upper()
        super().__setitem__(key, value)


class Edges(dict):
    def __setitem__(self, key: str, value: Edge):
        key = key.upper()
        super().__setitem__(key, value)


class Vda5050Map:
    def __init__(
        self, map_info: Map, nodes: Nodes, edge_mapping: dict, edges: Edges
    ) -> None:
        self.map_info = map_info
        self.nodes = nodes
        self.edges = edges
        self.edge_mapping = edge_mapping

    def get_edge(self, node1: str, node2: str):
        # Guard: a node with no edges at all (e.g. an isolated station like P619)
        # has no entry in edge_mapping; an unconnected pair maps to no edgeId.
        # Either way return None so the caller can skip it (see conversions.py)
        # instead of dereferencing None and crashing the command listener thread.
        neighbors = self.edge_mapping.get(node1)
        if neighbors is None:
            return None
        edge = copy.deepcopy(self.edges.get(neighbors.get(node2)))
        if edge is None:
            return None
        edge.startNodeId = node1
        edge.endNodeId = node2
        return edge
        # return copy.deepcopy(self.edges.get(node1).get(node2))

    def get_node(self, nodeid: str):
        return copy.deepcopy(self.nodes.get(nodeid))


class Vda5050Agv:
    def __init__(self, manufacturer: str, serial_number: str, map: Vda5050Map) -> None:
        self.manufacturer = manufacturer
        self.serial_number = serial_number
        self.uuid = self.manufacturer + "_" + self.serial_number

        self.state = None
        self.factsheet = None
        self.visualization = None
        self.connection = None
        self.order = None
        self.map = map
        self.executing_order = None

        # Order ID mismatch recovery tracking
        self.order_id_mismatch_count = 0
        self.ORDER_ID_MISMATCH_THRESHOLD = 3

        # Stitch Guard: Queue for pending order updates
        # This prevents sending order updates before AGV acknowledges previous state
        self.pending_order_updates = []

        self.state_message_callback = None
        self.factsheet_message_callback = None
        self.connection_message_callback = None
        self.visualization_message_callback = None
        self.instant_action_message_callback = None
        self.order_send_callback = None  # Called when queued order needs to be sent

        # Set up Message Queue to be Processed
        self.mqtt_msg_queue = queue.Queue()
        self.connection_msg_queue = queue.Queue()
        self.factsheet_msg_queue = queue.Queue()
        self.instant_actions_msg_queue = queue.Queue()
        self.visualization_msg_queue = queue.Queue()
        self.state_msg_queue = queue.Queue()

        # Set up thread flags (kept for non-state queues)
        self.connection_queue_thread_running = False
        self.factsheet_queue_thread_running = False
        self.instant_actions_queue_thread_running = False
        self.visualization_queue_thread_running = False
        # Note: state_queue_thread_running removed - using persistent worker instead

        # Start persistent worker thread for state processing
        # This eliminates race condition where thread exits before flag is set
        self._state_worker_thread = Thread(target=self._state_worker, daemon=True)
        self._state_worker_thread.start()

        # Tracking of dropped Header IDs
        self.last_connection_id = 0
        self.last_factsheet_id = 0
        self.last_instant_actions_id = 0
        self.last_visualization_id = 0
        self.last_state_id = 0

    def set_custom_state_callback(self, state_callback):
        # self.state_message_callback = copy.deepcopy(state_callback)
        self.state_message_callback = state_callback

    def set_custom_factsheet_callback(self, factsheet_callback):
        # self.factsheet_message_callback = copy.deepcopy(factsheet_callback)
        self.factsheet_message_callback = factsheet_callback

    def set_custom_connection_callback(self, connection_callback):
        # self.connection_message_callback = copy.deepcopy(connection_callback)
        self.connection_message_callback = connection_callback

    def set_custom_visualization_callback(self, visualization_callback):
        # self.visualization_message_callback = copy.deepcopy(visualization_callback)
        self.visualization_message_callback = visualization_callback

    def set_custom_instant_action_callback(self, instant_action_callback):
        # self.instant_action_message_callback = copy.deepcopy(instant_action_callback)
        self.instant_action_message_callback = instant_action_callback

    def set_order_send_callback(self, order_send_callback):
        """Set callback for sending queued orders via MQTT."""
        self.order_send_callback = order_send_callback

    # Callback triggered when AGV message is sent via MQTT Broker
    def on_connection_message(self, client, userdata, msg):
        self.connection_msg_queue.put(msg)
        if not self.connection_queue_thread_running:
            self.queue_thread = Thread(
                target=lambda: self.process_connections()
            ).start()
            self.connection_queue_thread_running = True

    def on_factsheet_message(self, msg):
        self.factsheet_msg_queue.put(msg)
        if not self.factsheet_queue_thread_running:
            self.queue_thread = Thread(target=lambda: self.process_factsheets()).start()
            self.factsheet_queue_thread_running = True

    def on_instant_actions_message(self, msg):
        self.instant_actions_msg_queue.put(msg)
        if not self.instant_actions_queue_thread_running:
            self.queue_thread = Thread(
                target=lambda: self.process_instant_actions()
            ).start()
            self.instant_actions_queue_thread_running = True

    def on_visualization_message(self, msg):
        self.visualization_msg_queue.put(msg)
        if not self.visualization_queue_thread_running:
            self.queue_thread = Thread(
                target=lambda: self.process_visualizations()
            ).start()
            self.visualization_queue_thread_running = True

    def on_state_message(self, msg):
        """Add state message to queue - persistent worker thread will process it."""
        self.state_msg_queue.put(msg)

    def process_connections(self):
        while not self.connection_msg_queue.empty():
            self.process_connection(self.connection_msg_queue.get())
            self.connection_msg_queue.task_done()
        self.connection_queue_thread_running = False

    def process_factsheets(self):
        while not self.factsheet_msg_queue.empty():
            self.process_factsheet(self.factsheet_msg_queue.get())
            self.factsheet_msg_queue.task_done()
        self.factsheet_queue_thread_running = False

    def process_instant_actions(self):
        while not self.instant_actions_msg_queue.empty():
            self.process_instant_action(self.instant_actions_msg_queue.get())
            self.instant_actions_msg_queue.task_done()
        self.instant_actions_queue_thread_running = False

    def process_visualizations(self):
        while not self.visualization_msg_queue.empty():
            self.process_visualization(self.visualization_msg_queue.get())
            self.visualization_msg_queue.task_done()
        self.visualization_queue_thread_running = False

    def _state_worker(self):
        """Persistent worker thread that processes state messages.

        Uses blocking queue.get() to wait for messages efficiently.
        This eliminates the race condition in the old start/stop thread pattern.
        """
        while True:
            try:
                msg = self.state_msg_queue.get(block=True)  # Block until message available
                self.process_state(msg)
                self.state_msg_queue.task_done()
            except Exception as e:
                printRed(f"[{self.uuid}] STATE_WORKER: Error processing state: {e}")

    def update_state_message(self, state_message: State):
        if self.state is not None:
            if self.state.headerId > state_message.headerId:
                printRed("["
                    + self.serial_number
                    + "]" + "Last processed state of headerId " + str(self.state.headerId) + "\nIgnore new state of headerId " + str(state_message.headerId) + "\n")
                return

        self.state = state_message

        # === STITCH GUARD: Check if any queued updates can now be sent ===
        self.process_pending_order_updates()

        if len(state_message.errors) != 0:
            printRed(json.dumps(State.to_json(state_message), indent=2))
            for error in state_message.errors:
                err_string = (
                    "["
                    + self.serial_number
                    + "]"
                    + "\nError Type: "
                    + str(error.errorType)
                    + "\nError Description: "
                    + error.errorDescription
                    + "\nError Level: "
                    + str(error.errorLevel.value)
                    + "\nError References: "
                )
                for error_ref in error.errorReferences:
                    err_string = (
                        err_string
                        + "\n\t"
                        + error_ref.referenceKey
                        + " "
                        + error_ref.referenceValue
                    )
                printRed(err_string)
            return

        # The order ID mismatch is now handled before this method is called (in process_state)
        # Keep a simple early return for safety, but recovery is handled above
        if self.order is not None:
            if self.executing_order is not None:
                if state_message.orderId != self.order.orderId:
                    # Already handled in process_state - just return
                    return
                else:
                    if state_message.orderUpdateId != self.order.orderUpdateId:
                        printRed(
                            "["
                            + self.manufacturer
                            + "_"
                            + self.serial_number
                            + "]"
                            + " WARNING: Current AGV state reflects a different Order Update ID from the existing registered Order "
                            + "\n Order ID: "
                            + state_message.orderId
                            + "\n Order Update ID (State): "
                            + str(state_message.orderUpdateId)
                            + "\n Order Update ID (Stored Order): "
                            + str(self.order.orderUpdateId)
                            + "\n["
                            + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                            + "]"
                        )
                        return
                if len(self.order.nodes) > 1:
                    if (
                        state_message.lastNodeSequenceId
                        == self.order.nodes[-1].sequenceId
                    ):
                        if state_message.lastNodeId == self.order.nodes[-1].nodeId:
                            printCyan(
                                "["
                                + self.manufacturer
                                + "_"
                                + self.serial_number
                                + "]"
                                + " Currently executing Order "
                                + self.order.orderId
                                + ", Order Update ID "
                                + str(self.order.orderUpdateId)
                                + "\n\tSequence: "
                                + str(state_message.lastNodeSequenceId)
                                + "/"
                                + str(self.order.nodes[-1].sequenceId)
                                + "\n["
                                + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                                + "]"
                            )
                            printGreen(
                                "["
                                + self.manufacturer
                                + "_"
                                + self.serial_number
                                + "]"
                                + " Order "
                                + state_message.orderId
                                + " completed."
                                + "\n["
                                + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                                + "]"
                            )
                            self.executing_order = None
                            self.order_id_mismatch_count = 0
                        else:
                            printRed(
                                "["
                                + self.manufacturer
                                + "_"
                                + self.serial_number
                                + "]"
                                + " WARNING: AGV State message shows the lastNodeSequenceId as "
                                + str(state_message.lastNodeSequenceId)
                                + ", but the lastNodeId shows "
                                + state_message.lastNodeId
                                + ", which is different from the Order, which shows that Node "
                                + self.order.nodes[-1].nodeId
                                + " has a sequence ID of "
                                + str(self.order.nodes[-1].sequenceId)
                                + "\n["
                                + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                                + "]"
                            )
                            return
                    else:
                        if len(state_message.nodeStates) == 0:
                            printRed(
                                "["
                                + self.manufacturer
                                + "_"
                                + self.serial_number
                                + "]"
                                + " WARNING: AGV State message shows the lastNodeSequenceId "
                                + str(state_message.lastNodeSequenceId)
                                + " is not the last sequence ID of the stored order ("
                                + str(self.order.nodes[-1].sequenceId)
                                + "), but yet the remaining NodeState array is empty. "
                                + "\n["
                                + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                                + "]"
                            )
                            return
                        printCyan(
                            "["
                            + self.manufacturer
                            + "_"
                            + self.serial_number
                            + "]"
                            + " Currently executing Order "
                            + self.order.orderId
                            + ", Order Update ID "
                            + str(self.order.orderUpdateId)
                            + "\n\tSequence: "
                            + str(state_message.lastNodeSequenceId)
                            + "/"
                            + str(self.order.nodes[-1].sequenceId)
                            + "\n["
                            + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                            + "]"
                        )
                else:
                    printYellow(
                        "["
                        + self.manufacturer
                        + "_"
                        + self.serial_number
                        + "]"
                        + " Order "
                        + self.order.orderId
                        + ", Order Update ID "
                        + str(self.order.orderUpdateId)
                        + " has only 1 node (completed at current position)"
                        + "\n["
                        + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                        + "]"
                    )
                    # Treat as completed - don't clear self.order so updates can still be combined
                    self.executing_order = None
                    self.order_id_mismatch_count = 0
            else:
                if len(state_message.nodeStates) > 0:
                    printGreen(
                        "["
                        + self.manufacturer
                        + "_"
                        + self.serial_number
                        + "]"
                        + " Executing New Order "
                        + state_message.orderId
                        + "\n["
                        + state_message.timestamp.strftime("%m/%d/%Y, %H:%M:%S")
                        + "]"
                    )
                    self.executing_order = state_message.orderId

    def update_factsheet_message(self, factsheet_message):
        self.factsheet = factsheet_message

    def update_visualization_message(self, visualization_message):
        self.visualization = visualization_message

    def update_connection_message(self, connection_message):
        self.connection = connection_message

    def update_order_message(self, order_message: OrderMessage):
        """
        Update the order message for this AGV.
        Returns True if the order should be sent to AGV, False if it was queued.

        NOTE: orderUpdateId reset logic for new orders is now handled by the
        per-robot command queue in vda5050_fiware.py. The queue properly tracks
        updateIds per (robot, orderId) and resets updateId to 0 when the first
        update for a new order arrives with updateId > 0 (lost initial update).

        The previous ORDER FIX here was removed because it caused race conditions:
        - When rapid updates arrived (updateId=0 then updateId=1), the robot state
          might not yet reflect the new order when updateId=1 arrived
        - This caused updateId=1 to be incorrectly reset to 0 (duplicate)
        - The robot would reject the duplicate and get stuck

        See CHANGELOG.md for full documentation.
        """
        if self.order is not None:
            if (
                self.order.orderId == order_message.orderId
                and self.order.orderUpdateId < order_message.orderUpdateId
            ):
                # === STITCH GUARD: Check if AGV is ready for this update ===
                if self.state is not None and len(order_message.nodes) > 0:
                    first_node_seq = order_message.nodes[0].sequenceId

                    # Check 1: Is AGV on the correct order?
                    if self.state.orderId != order_message.orderId:
                        printYellow(
                            "[STITCH GUARD] ["
                            + self.serial_number
                            + "] AGV still on order "
                            + self.state.orderId[:50]
                            + "..., cannot send update for order "
                            + order_message.orderId[:50]
                            + "... - QUEUEING Update "
                            + str(order_message.orderUpdateId)
                        )
                        self.pending_order_updates.append(order_message)
                        return False  # Don't send - update was queued

                    # Check 2a: Has AGV already PASSED the stitch point? (stale update)
                    # Only applies if order is still executing - if completed, this is the next segment
                    if first_node_seq < self.state.lastNodeSequenceId and self.executing_order is not None:
                        printYellow(
                            "[STITCH GUARD] ["
                            + self.serial_number
                            + "] STALE UPDATE - AGV at seq "
                            + str(self.state.lastNodeSequenceId)
                            + ", update starts at seq "
                            + str(first_node_seq)
                            + " - QUEUEING Update "
                            + str(order_message.orderUpdateId)
                            + " (waiting for current order to complete)"
                        )
                        # Queue instead of discard - let current order finish first
                        self.pending_order_updates.append(order_message)
                        return False  # Don't send now - queued for later

                    # Check 2b: Has AGV reached the stitch point yet?
                    if first_node_seq > self.state.lastNodeSequenceId:
                        printYellow(
                            "[STITCH GUARD] ["
                            + self.serial_number
                            + "] AGV at seq "
                            + str(self.state.lastNodeSequenceId)
                            + ", update starts at seq "
                            + str(first_node_seq)
                            + " - QUEUEING Update "
                            + str(order_message.orderUpdateId)
                        )
                        self.pending_order_updates.append(order_message)
                        return False  # Don't send - update was queued

                    # Check 3: Has AGV confirmed the previous update?
                    # This prevents sending updates faster than AGV can process them
                    if self.order.orderUpdateId > self.state.orderUpdateId:
                        printYellow(
                            "[STITCH GUARD] ["
                            + self.serial_number
                            + "] AGV confirmed updateId "
                            + str(self.state.orderUpdateId)
                            + ", master has updateId "
                            + str(self.order.orderUpdateId)
                            + " - QUEUEING Update "
                            + str(order_message.orderUpdateId)
                            + " (waiting for AGV to confirm previous update)"
                        )
                        self.pending_order_updates.append(order_message)
                        return False  # Don't send - update was queued

                printCyan("Combining new update for order " + self.order.orderId)
                self.combine_order(new_order=order_message, automatic_update=True)
                return True  # Send the combined order
            if self.executing_order:
                printRed(
                    "Error, robot is currently still working on Order "
                    + self.order.orderId
                    + ". Cannot take order "
                    + order_message.orderId
                )
                return False  # Don't send - error condition

        self.order = order_message
        printCyan("Adding new order " + self.order.orderId)
        self.executing_order = self.order.orderId
        self.order_id_mismatch_count = 0
        return True  # Send the new order

    def process_pending_order_updates(self):
        """
        Process any pending order updates that were queued by the stitch guard.
        Called when AGV state changes to check if queued updates can now be sent.
        """
        if not self.pending_order_updates or self.state is None:
            return

        # Process pending updates in order
        while self.pending_order_updates:
            next_update = self.pending_order_updates[0]

            if len(next_update.nodes) == 0:
                # Empty nodes, skip
                self.pending_order_updates.pop(0)
                continue

            first_node_seq = next_update.nodes[0].sequenceId

            # Check 1: Is AGV on the correct order?
            if self.state.orderId != next_update.orderId:
                # Still waiting for AGV to switch to new order
                break

            # Check 2a: Has AGV already PASSED the stitch point? (stale update)
            # Only applies if order is still executing - if completed, this is the next segment
            if first_node_seq < self.state.lastNodeSequenceId and self.executing_order is not None:
                # Still executing, wait for current order to complete
                break

            # Check 2b: Has AGV reached the stitch point yet?
            if first_node_seq > self.state.lastNodeSequenceId:
                # Still waiting for AGV to reach stitch point
                break

            # Check 3: Has AGV confirmed the previous update?
            if self.order.orderUpdateId > self.state.orderUpdateId:
                # Still waiting for AGV to confirm previous update
                break

            # AGV is ready - send this update
            self.pending_order_updates.pop(0)
            printGreen(
                "[STITCH GUARD] ["
                + self.serial_number
                + "] AGV now at seq "
                + str(self.state.lastNodeSequenceId)
                + " on correct order - SENDING queued Update "
                + str(next_update.orderUpdateId)
            )
            self.combine_order(new_order=next_update, automatic_update=True)

            # Send the combined order via MQTT
            if self.order_send_callback:
                self.order_send_callback(self)

    def process_connection(self, msg):
        message_json = json.loads(str(msg.payload.decode("utf-8", "ignore")))
        if self.connection_message_callback:
            self.connection_message_callback(
                connection_message_json=message_json,
                robot_id=self.uuid,
            )
        self.update_connection_message(message_json)

    def process_factsheet(self, msg):
        message_json = json.loads(str(msg.payload.decode("utf-8", "ignore")))
        if self.factsheet_message_callback:
            self.factsheet_message_callback(
                factsheet_message_json=message_json,
                robot_id=self.uuid,
            )
        self.update_factsheet_message(message_json)

    def process_instant_action(self, msg):
        message_json = json.loads(str(msg.payload.decode("utf-8", "ignore")))
        if self.instant_action_message_callback:
            self.instant_action_message_callback(
                instant_action_message_json=message_json,
                robot_id=self.uuid,
            )

    def process_visualization(self, msg):
        message_json = json.loads(str(msg.payload.decode("utf-8", "ignore")))
        if self.visualization_message_callback:
            self.visualization_message_callback(
                visualization_message_json=message_json,
                robot_id=self.uuid,
            )
        self.update_visualization_message(message_json)

    # def process_state(self, client, userdata, msg):
    def process_state(self, msg):
        message_json = json.loads(str(msg.payload.decode("utf-8", "ignore")))
        vda_state = State.from_json(message_json)
        # if vda_state.headerId - self.last_state_id != 1:
        #     printRed(
        #         "["
        #         + self.manufacturer
        #         + "_"
        #         + self.serial_number
        #         + "] "
        #         + "Warning (Potential Dropped Message): Previous Header ID: "
        #         + str(self.last_state_id)
        #         + " New Header ID: "
        #         + str(vda_state.headerId)
        #     )
        self.last_state_id = vda_state.headerId

        # Check for order ID mismatch BEFORE forwarding to FIWARE
        should_forward = True
        if self.order is not None and self.executing_order is not None:
            if vda_state.orderId != self.order.orderId:
                self.order_id_mismatch_count += 1
                printRed(
                    "[" + self.uuid + "] WARNING: Order ID mismatch"
                    + " (State: " + vda_state.orderId
                    + ", Expected: " + self.order.orderId + ")"
                    + " [" + str(self.order_id_mismatch_count)
                    + "/" + str(self.ORDER_ID_MISMATCH_THRESHOLD) + "]"
                )

                if self.order_id_mismatch_count >= self.ORDER_ID_MISMATCH_THRESHOLD:
                    # RECOVERY: Clear stale state
                    printYellow("[" + self.uuid + "] RECOVERY: Clearing stale order state")
                    self.executing_order = None
                    self.order = None
                    self.order_id_mismatch_count = 0
                    self.pending_order_updates = []
                    printGreen("[" + self.uuid + "] RECOVERY COMPLETE: Can accept new orders")
                    # Forward this state to sync ADG with recovered state
                    should_forward = True
                else:
                    # Don't forward bad state to FIWARE/ADG
                    should_forward = False
            else:
                # Order IDs match - reset counter
                self.order_id_mismatch_count = 0

        if should_forward and self.state_message_callback:
            self.state_message_callback(
                vda_state=vda_state,
                robot_id=self.uuid,
            )

        self.update_state_message(vda_state)

    def combine_order(self, new_order: OrderMessage, automatic_update=False):
        if not self.order:
            updated_order = new_order
        else:
            if new_order.orderId != self.order.orderId:
                printRed("Error, New Order ID is not the same as existing Order Id")
            if new_order.manufacturer != self.order.manufacturer:
                printRed(
                    "Error, New Order manufacturer is not the same as existing Order's manufacturer"
                )
            if new_order.serialNumber != self.order.serialNumber:
                printRed(
                    "Error, New Order Serial Number is not the same as existing Order's Serial Number"
                )

        # ---------------------- Old Combine and Update Order Algo
        old_waypoint_subset = []

        # Get the last traversed node in the past
        old_base_last_node = None

        for old_order_node in self.order.nodes:
            # If the previous order node is not released, and has been travelled by AGV: ERROR
            if (
                not old_order_node.released
                and old_order_node.sequenceId <= self.state.lastNodeSequenceId
            ):
                printRed(
                    "WARNING: Node is not released \
                            but yet is supposed to be already traversed by AGV"
                )

            # For nodes that are already released, they are considered part of the old
            # base. We just need to get the last node of the old base.

            if old_order_node.released:
                old_base_last_node = old_order_node
            # For nodes that are not released, they are part of the horizon of the new order
            else:
                old_waypoint_subset.append(old_order_node)

        # If there is a base last node that has already been traversed by AGV,
        # add to start of updated order
        if old_base_last_node is not None:
            old_waypoint_subset.insert(0, old_base_last_node)
        else:
            printRed("ERROR: No old node, something is weird")

        path_last_sequence_id = old_waypoint_subset[-1].sequenceId

        # Create a copy of the old waypoint subset to keep. We will update this
        updated_old_waypoint_subset = old_waypoint_subset

        # This array stores the subset of the new order that should add to the updated order
        new_waypoint_subset = []

        for new_order_node in new_order.nodes:
            # For nodes extending beyond old waypoints, just add them on
            if new_order_node.sequenceId > path_last_sequence_id:
                new_waypoint_subset.append(new_order_node)
            else:
                # For nodes already in old waypoints, replace them
                index = 0
                for curr_node in old_waypoint_subset:
                    if new_order_node.sequenceId == curr_node.sequenceId:
                        if curr_node.released and not new_order_node.released:
                            printRed(" FATAL ERROR Cannot unrelease a released node.")
                            return
                        updated_old_waypoint_subset[index] = new_order_node
                        break
                    index += 1
        updated_nodes = updated_old_waypoint_subset + new_waypoint_subset

        self_order_string = (
            "============================= self.order =============================\n"
        )
        for node in self.order.nodes:
            self_order_string = (
                self_order_string
                + "Node Name: "
                + node.nodeId
                + "\n Seq ID: "
                + str(node.sequenceId)
                + "\n"
            )
        printCyan(self_order_string)

        new_order_string = (
            "============================= new_order =============================\n"
        )
        for node in new_order.nodes:
            new_order_string = (
                new_order_string
                + "Node Name: "
                + node.nodeId
                + "\n Seq ID: "
                + str(node.sequenceId)
                + "\n"
            )
        printCyan(new_order_string)

        updated_nodes_string = "============================= updated_nodes =============================\n"
        for node in updated_nodes:
            updated_nodes_string = (
                updated_nodes_string
                + "Node Name: "
                + node.nodeId
                + "\n Seq ID: "
                + str(node.sequenceId)
                + "\n"
            )
        printCyan(updated_nodes_string)

        # ---------------------- End Combine and Update Order Algo

        # Update the sequence IDs
        if len(updated_nodes) < 0:
            printRed("Error: No updated nodes. Something is wrong")
            return
        # count = 0
        count = updated_nodes[0].sequenceId
        for node in updated_nodes:
            node.sequenceId = count
            count += 2

        updated_nodes_string = "============================= updated_nodes =============================\n"
        for node in updated_nodes:
            updated_nodes_string = (
                updated_nodes_string
                + "Node Name: "
                + node.nodeId
                + "\n Seq ID: "
                + str(node.sequenceId)
                + "\n"
            )
        printCyan(updated_nodes_string)

        # Update the edges
        updated_edges = []
        node_before = None
        last_sequence_id = updated_nodes[-1].sequenceId
        for updated_node in updated_nodes:
            if node_before is not None:
                # Catch any repeated nodes
                if node_before == updated_node.nodeId:
                    continue

                # An edge can only be released if the node before and after an edge is released
                edge_released = node_before.released and updated_node.released
                edge = self.map.get_edge(node_before.nodeId, updated_node.nodeId)
                if edge is None:
                    printRed(
                        "Error, no edge found connecting node "
                        + node_before.nodeId
                        + " and "
                        + updated_node.nodeId
                    )
                    return

                edge.sequenceId = node_before.sequenceId + 1
                edge.released = edge_released
                updated_edges.append(edge)
                if edge.sequenceId + 1 == last_sequence_id:
                    break
            node_before = updated_node

        updated_order = OrderMessage(
            headerId=new_order.headerId,
            timestamp=datetime.now().isoformat() + 'Z',
            version=new_order.version,
            manufacturer=new_order.manufacturer,
            serialNumber=new_order.serialNumber,
            orderId=new_order.orderId,
            orderUpdateId=self.order.orderUpdateId + 1,
            nodes=updated_nodes,
            edges=updated_edges,
        )

        if automatic_update:
            self.order = updated_order
        return updated_order


class Vda5050AgvDict(dict):
    def __setitem__(self, key: str, value: Vda5050Agv):
        super().__setitem__(key, value)

    def __getitem__(self, key: str):
        return super().__getitem__(key)


class Vda5050Agvs(dict):
    def __setitem__(self, key: str, value: Vda5050AgvDict):
        super().__setitem__(key, value)

    def __getitem__(self, key: str):
        return super().__getitem__(key)


class Vda5050Master:
    def __init__(
        self, version, interface, mqtt_url: str, mqtt_port: int, map: Vda5050Map
    ) -> None:
        self.state_message_callback = None
        self.factsheet_message_callback = None
        self.connection_message_callback = None
        self.visualization_message_callback = None
        self.instant_action_message_callback = None

        self.version = version
        self.interface = interface
        self.mqtt_url = mqtt_url
        self.mqtt_port = mqtt_port

        self.map = map

        # Set up Message Queue to be Processed
        self.state_msg_queue = queue.Queue()

        # Set up thread flags
        self.state_queue_thread_running = False

        # Storage of VDA5050 Robot and robot types
        self.robots = Vda5050Agvs()

    def on_publish(self, client, userdata, mid):
        print("mid: " + str(mid))

    def start(self):
        # Mqtt Client to read from message broker
        self.mqtt_client_connection = MQTTClient(
            id="VDA5050 Master Mqtt Client connection",
            host=self.mqtt_url,
            port=self.mqtt_port,
            topic=self.interface + "/" + self.version + "/+/+/connection",
            on_message_callback=self.process_connection,
        )

        self.mqtt_client_factsheet = MQTTClient(
            id="VDA5050 Master Mqtt Client factsheet",
            host=self.mqtt_url,
            port=self.mqtt_port,
            topic=self.interface + "/" + self.version + "/+/+/factsheet",
            on_message_callback=self.process_factsheets,
            qos=1,
        )

        self.mqtt_client_instant_actions = MQTTClient(
            id="VDA5050 Master Mqtt Client instant_actions",
            host=self.mqtt_url,
            port=self.mqtt_port,
            topic=self.interface + "/" + self.version + "/+/+/instantActions",
            on_message_callback=self.process_instant_actions,
            qos=1,
        )

        self.mqtt_client_visualization = MQTTClient(
            id="VDA5050 Master Mqtt Client visualization",
            host=self.mqtt_url,
            port=self.mqtt_port,
            topic=self.interface + "/" + self.version + "/+/+/visualization",
            on_message_callback=self.process_visualization,
            qos=1,
        )

        self.mqtt_client_state = MQTTClient(
            id="VDA5050 Master Mqtt Client state",
            host=self.mqtt_url,
            port=self.mqtt_port,
            topic=self.interface + "/" + self.version + "/+/+/state",
            on_message_callback=self.process_state,
            qos=0,
        )

        self.order_pub_client = paho.Client()
        self.order_pub_client.on_publish = self.on_publish
        self.order_pub_client.connect(self.mqtt_url, self.mqtt_port, keepalive=0)
        Thread(target=lambda: self.order_pub_client.loop_forever()).start()

    def process_state(self, client, userdata, msg):
        topic_tokens = str(msg.topic).split("/")
        manufacturer = topic_tokens[2]
        serial_number = topic_tokens[3]

        if manufacturer not in self.robots:
            self.robots[manufacturer] = Vda5050AgvDict()
        if serial_number not in self.robots.get(manufacturer):
            new_agv = Vda5050Agv(
                manufacturer=manufacturer, serial_number=serial_number, map=self.map
            )

            new_agv.set_custom_connection_callback(self.connection_message_callback)
            new_agv.set_custom_factsheet_callback(self.factsheet_message_callback)
            new_agv.set_custom_instant_action_callback(
                self.instant_action_message_callback
            )
            new_agv.set_custom_visualization_callback(
                self.visualization_message_callback
            )
            new_agv.set_custom_state_callback(self.state_message_callback)
            new_agv.set_order_send_callback(self.send_agv_current_order)

            self.robots[manufacturer][serial_number] = new_agv
            count = 0
            for manufacturer_ in self.robots:
                for robot in self.robots.get(manufacturer_):
                    count = count + 1
            printGreen(
                "============= Discovered New AGV at =============\n"
                + "Manufacturer: "
                + manufacturer
                + "\n"
                + "Serial Number: "
                + serial_number
                + "\n"
                + "Total AGV Count: "
                + str(count)
                + "\n"
                + "==============================================\n"
            )

        self.robots.get(manufacturer)[serial_number].on_state_message(msg)

    def process_visualization(self, client, userdata, msg):
        topic_tokens = str(msg.topic).split("/")
        manufacturer = topic_tokens[2]
        serial_number = topic_tokens[3]

        if manufacturer in self.robots and serial_number in self.robots.get(
            manufacturer
        ):
            self.robots.get(manufacturer)[serial_number].on_visualization_message(msg)

    def process_instant_actions(self, client, userdata, msg):
        topic_tokens = str(msg.topic).split("/")
        manufacturer = topic_tokens[2]
        serial_number = topic_tokens[3]

        if manufacturer in self.robots and serial_number in self.robots.get(
            manufacturer
        ):
            self.robots.get(manufacturer)[serial_number].on_instant_actions_message(msg)

    def process_factsheets(self, client, userdata, msg):
        topic_tokens = str(msg.topic).split("/")
        manufacturer = topic_tokens[2]
        serial_number = topic_tokens[3]

        if manufacturer in self.robots and serial_number in self.robots.get(
            manufacturer
        ):
            self.robots.get(manufacturer)[serial_number].on_factsheet_message(msg)

    def process_connection(self, client, userdata, msg):
        topic_tokens = str(msg.topic).split("/")
        manufacturer = topic_tokens[2]
        serial_number = topic_tokens[3]

        # if manufacturer not in self.robots:
        #     self.robots[manufacturer] = Vda5050AgvDict()
        # if serial_number not in self.robots.get(manufacturer):
        #     new_agv = Vda5050Agv(
        #         manufacturer=manufacturer, serial_number=serial_number, map=self.map
        #     )

        #     new_agv.set_custom_connection_callback(self.connection_message_callback)
        #     new_agv.set_custom_factsheet_callback(self.factsheet_message_callback)
        #     new_agv.set_custom_instant_action_callback(
        #         self.instant_action_message_callback
        #     )
        #     new_agv.set_custom_visualization_callback(
        #         self.visualization_message_callback
        #     )
        #     new_agv.set_custom_state_callback(self.state_message_callback)

        #     self.robots[manufacturer][serial_number] = new_agv
        #     count = 0
        #     for manufacturer_ in self.robots:
        #         for robot in self.robots.get(manufacturer_):
        #             count = count + 1
        #     printGreen(
        #         "============= Discovered New AGV at =============\n"
        #         + "Manufacturer: "
        #         + manufacturer
        #         + "\n"
        #         + "Serial Number: "
        #         + serial_number
        #         + "\n"
        #         + "Total AGV Count: "
        #         + str(count)
        #         + "\n"
        #         + "==============================================\n"
        #     )

    # Combines the order nodes from a previous order with an updated version.
    def combine_and_update_order_nodes(
        self,
        old_order_node_array,
        new_order_node_array,
        agv_last_traversed_node_sequence,
    ):
        # This array stores the nodes that will be appended to the beginning of the new order
        old_waypoint_subset = []

        # Get the last traversed node in the past
        old_base_last_node = None
        for old_order_node in old_order_node_array:
            # If the previous order node is not released, and has been travelled by AGV: ERROR
            if (
                not old_order_node["released"]
                and old_order_node["sequenceId"] <= agv_last_traversed_node_sequence
            ):
                printRed(
                    "WARNING: Node is not released \
                            but yet is supposed to be already traversed by AGV"
                )

            # For nodes that are already released, they are considered part of the old
            # base. We just need to get the last node of the old base.

            if old_order_node["released"]:
                old_base_last_node = old_order_node
            # For nodes that are not released, they are part of the horizon of the new order
            else:
                old_waypoint_subset.append(old_order_node)

        # If there is a base last node that has already been traversed by AGV,
        # add to start of updated order
        if old_base_last_node is not None:
            old_waypoint_subset.insert(0, old_base_last_node)
        else:
            printRed("ERROR: No old node, something is weird")

        path_last_sequence_id = old_waypoint_subset[-1]["sequenceId"]

        # Create a copy of the old waypoint subset to keep. We will update this
        updated_old_waypoint_subset = old_waypoint_subset

        # This array stores the subset of the new order that should add to the updated order
        new_waypoint_subset = []

        for new_order_node in new_order_node_array:
            # For nodes extending beyond old waypoints, just add them on
            if new_order_node["sequenceId"] > path_last_sequence_id:
                new_waypoint_subset.append(new_order_node)
            else:
                # For nodes already in old waypoints, replace them
                index = 0
                for curr_node in old_waypoint_subset:
                    if new_order_node["sequenceId"] == curr_node["sequenceId"]:
                        if curr_node["released"] and not new_order_node["released"]:
                            printRed(" FATAL ERROR Cannot unrelease a released node.")
                            return
                        updated_old_waypoint_subset[index] = new_order_node
                        break
                    index += 1

        combined_nodes = updated_old_waypoint_subset + new_waypoint_subset

        return combined_nodes

    # Method that sends a VDA5050 Order to Master
    def send_order_to_agv(self, vda5050_order: OrderMessage):
        printYellow("Attempting to send VDA5050 Order...")
        if vda5050_order.manufacturer in self.robots:
            if vda5050_order.serialNumber in self.robots.get(
                vda5050_order.manufacturer
            ):
                # Update order message and check if we should send
                should_send = self.robots.get(vda5050_order.manufacturer).get(
                    vda5050_order.serialNumber
                ).update_order_message(vda5050_order)

                # If update was queued by stitch guard, don't send now
                if not should_send:
                    printYellow(
                        "[STITCH GUARD] Order update queued for "
                        + vda5050_order.serialNumber
                        + ", not sending now"
                    )
                    return

                order_json = (
                    self.robots.get(vda5050_order.manufacturer)
                    .get(vda5050_order.serialNumber)
                    .order.json(exclude_none=True)
                )

                # order_json = vda5050_order.json(exclude_none=True)

                order_topic = (
                    self.interface
                    + "/"
                    + self.version
                    + "/"
                    + vda5050_order.manufacturer
                    + "/"
                    + vda5050_order.serialNumber
                    + "/order"
                )

                printYellow(
                    "Sending Order to:"
                    + "\n\t MQTT Topic: "
                    + str(order_topic)
                    + "\n\t Order ID: "
                    + str(vda5050_order.orderId)
                    + "\n\t Update ID: "
                    + str(vda5050_order.orderUpdateId)
                    + json.dumps(order_json, indent=2)
                )

                self.order_pub_client.publish(order_topic, order_json, qos=0)
            else:
                printRed(
                    "No such robot with manufacturer "
                    + vda5050_order.manufacturer
                    + " and serial number "
                    + vda5050_order.serialNumber
                    + " exists, cannot send order"
                )
                return
        else:
            printRed(
                "No such robot with manufacturer "
                + vda5050_order.manufacturer
                + " and serial number "
                + vda5050_order.serialNumber
                + " exists, cannot send order"
            )
            return

    def send_agv_current_order(self, agv: Vda5050Agv):
        """
        Send the current order of an AGV via MQTT.
        Called by stitch guard when a queued update is ready to be sent.
        """
        if agv.order is None:
            printRed("[STITCH GUARD] Cannot send order - AGV has no order")
            return

        order_json = agv.order.json(exclude_none=True)

        order_topic = (
            self.interface
            + "/"
            + self.version
            + "/"
            + agv.manufacturer
            + "/"
            + agv.serial_number
            + "/order"
        )

        printGreen(
            "[STITCH GUARD] Sending queued order to:"
            + "\n\t MQTT Topic: "
            + str(order_topic)
            + "\n\t Order ID: "
            + str(agv.order.orderId)
            + "\n\t Update ID: "
            + str(agv.order.orderUpdateId)
        )

        self.order_pub_client.publish(order_topic, order_json, qos=0)

    def set_custom_state_callback(self, state_callback):
        self.state_message_callback = state_callback

    def set_custom_factsheet_callback(self, factsheet_callback):
        self.factsheet_message_callback = factsheet_callback

    def set_custom_connection_callback(self, connection_callback):
        self.connection_message_callback = connection_callback

    def set_custom_visualization_callback(self, visualization_callback):
        self.visualization_message_callback = visualization_callback

    def set_custom_instant_action_callback(self, instant_action_callback):
        self.instant_action_message_callback = instant_action_callback
