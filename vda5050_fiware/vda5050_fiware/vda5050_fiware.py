import sys
import json
import time
import threading
from collections import defaultdict

from fiware_api.context_broker.scorpio import ScorpioAPI, ScorpioEventSubscriber
from fiware_api.utils import (
    printRed,
    printGreen,
    printYellow,
    printPurple,
    printCyan,
    MQTTClient,
)

from vda5050_fiware.master import Vda5050Master
from rmf2_agv.StateMessage import StateMessage
from vda5050_fiware.conversions import (
    convert_command_message_json_to_vda_order,
    convert_json_to_vda_map,
    convert_vda_state_to_state_message,
)

from rmf2_building.Map import Map, Node


class Vda5050Fiware:
    """
    Bridge between FIWARE/Scorpio context broker and VDA5050 master controller.

    This class subscribes to CommandMessage updates from Scorpio and forwards
    them to the VDA5050 master for delivery to AGVs. It also receives VDA5050
    state messages and publishes them back to Scorpio as StateMessage entities.

    Command Queue System
    --------------------
    A per-robot command queue ensures CommandMessage updates are processed in
    the correct order, handling race conditions that can occur when rapid updates
    arrive from the ADG (Autonomous Driving Graph) system.

    The queue system handles:
    - Out-of-order arrivals (e.g., updateId=1 arrives before updateId=0)
    - Lost initial updates (e.g., updateId=0 lost, only updateId=1 arrives)
    - Gaps in update sequence (e.g., updateId=2 missing between 1 and 3)
    - Stale updates (e.g., updateId=2 arrives after updateId=3 was sent)

    Key Design Decisions:
    - updateId tracking is per (robot_id, orderId), NOT based on robot state
    - If updateId=0 is lost for a new order, the first arriving update is reset to 0
    - Updates are sent in order (2 before 3) to respect VDA5050 stitching semantics
    - A 2-second timeout allows for out-of-order arrivals before skipping gaps

    See CHANGELOG.md for full documentation of the queue system.

    Attributes
    ----------
    vda5050_master : Vda5050Master
        The VDA5050 master controller managing AGV connections
    context_broker : ScorpioAPI
        FIWARE Scorpio context broker API client
    command_queues : dict
        Per-robot queues of pending OrderMessages, keyed by robot_id
    last_sent_updateId : dict
        Tracks last sent orderUpdateId per (robot_id, orderId)
    queue_lock : threading.Lock
        Thread lock for safe concurrent queue access
    missing_update_timeout : float
        Seconds to wait for missing updates before skipping (default: 2.0)
    """

    def __init__(
        self,
        context_broker: ScorpioAPI,
        vda5050_master: Vda5050Master,
    ) -> None:
        self.vda5050_master = vda5050_master
        self.vda5050_master.set_custom_state_callback(self.process_vda5050_state)
        self.context_broker = context_broker
        self.headers = {
            "Content-Type": "application/json",
            "Link": 'https://smart-data-models.github.io/dataModel.AutonomousMobileRobot/StateMessage/examples/example-normalized.jsonld; rel="https://www.w3.org/ns/json-ld#context"; type="application/ld+json"',
        }
        self.command_update_topic = "command_updates"
        self.namespace = "ngsi-ld:default-context/"

        # Per-robot command queue system
        # Tracks pending commands per robot, keyed by robot_id (e.g., "MiR_0001")
        self.command_queues = defaultdict(list)  # {robot_id: [order_messages]}
        # Tracks last sent updateId per robot per orderId
        # {robot_id: {orderId: last_updateId}}
        self.last_sent_updateId = defaultdict(lambda: defaultdict(lambda: -1))
        # Lock for thread-safe queue access
        self.queue_lock = threading.Lock()
        # Timeout for waiting on missing updates (seconds)
        self.missing_update_timeout = 2.0
        # Timer threads for processing queues
        self.queue_timers = {}  # {robot_id: timer_thread}

        self.agv_state_event_subscriber = ScorpioEventSubscriber(
            self.context_broker.endpoint,
            id="command_updates",
            host=self.vda5050_master.mqtt_url,
            port=self.vda5050_master.mqtt_port,
            topic="command_updates",
            message_type="CommandMessage",
            ld_link=self.headers["Link"],
            on_message_callback=self.on_command_message,
        )

        self.agv_state_event_subscriber.run()
        self.vda5050_master.start()

    def get_array_str(self, array):
        array_str = ""
        for entry in array:
            array_str += json.dumps(entry) + "\n"
        return array_str

    def _get_robot_id(self, order_message):
        """Get robot ID from order message (format: manufacturer_serialNumber)"""
        return f"{order_message.manufacturer}_{order_message.serialNumber}"

    # Callback triggered when an updated Command Message is detected
    def on_command_message(self, client, userdata, msg):
        # Wrapped so one malformed/unroutable command can never kill the
        # command_updates listener thread (paho does not catch callback errors).
        try:
            command_message_json = json.loads(str(msg.payload.decode("utf-8", "ignore")))
            order_message = convert_command_message_json_to_vda_order(
                command_message_json, self.vda5050_master.map
            )

            robot_id = self._get_robot_id(order_message)

            printCyan(
                f"[QUEUE] Received command for {robot_id}: "
                f"orderId={order_message.orderId}, updateId={order_message.orderUpdateId}"
            )

            with self.queue_lock:
                # Add to the robot's command queue
                self.command_queues[robot_id].append(order_message)

            # Process the queue for this robot
            self._process_queue(robot_id)
        except Exception as e:
            printRed(f"[command_updates] dropped command, handling failed: {e}")
        return

    def _process_queue(self, robot_id):
        """
        Process the command queue for a specific robot.

        This is the core queue processing logic that ensures commands are sent
        to AGVs in the correct order, handling various edge cases.

        Processing Logic
        ----------------
        1. Sort queue by (orderId, orderUpdateId)
        2. Check the first (lowest updateId) order in the queue
        3. Compare against last_sent_updateId to determine action:

           - updateId <= last_sent: Discard as stale (already sent higher)
           - updateId == expected (last_sent + 1): Send immediately
           - updateId > 0 AND last_sent == -1: Reset to 0 and send (lost initial)
           - updateId > expected: Wait for missing update (start timeout timer)

        Edge Case Handling
        ------------------
        - Out-of-order: If updateId=1 arrives before 0, it's queued. When 0
          arrives (or timeout expires), both are processed in order.

        - Lost initial: If updateId=0 never arrives, we reset the first
          available updateId to 0 (either immediately if last_sent=-1, or
          after timeout).

        - Gap: If we have 1 and 3 but not 2, we wait up to 2 seconds for 2.
          After timeout, we skip the gap and send 3.

        Parameters
        ----------
        robot_id : str
            Robot identifier in format "manufacturer_serialNumber" (e.g., "MiR_0001")

        Notes
        -----
        This method is called:
        - When a new command is added to the queue (from on_command_message)
        - After sending an order (to process next in queue)
        - After a timeout expires (to skip gaps)

        Thread Safety: This method acquires queue_lock and may release/reacquire
        it when sending orders to avoid deadlocks with VDA5050 master.
        """
        with self.queue_lock:
            queue = self.command_queues[robot_id]
            if not queue:
                return

            # Sort queue by (orderId, orderUpdateId)
            queue.sort(key=lambda o: (o.orderId, o.orderUpdateId))

            # Get the first order (lowest updateId for a given orderId)
            order = queue[0]
            order_id = order.orderId
            update_id = order.orderUpdateId
            last_sent = self.last_sent_updateId[robot_id][order_id]

            printCyan(
                f"[QUEUE] Processing {robot_id}: orderId={order_id}, "
                f"updateId={update_id}, lastSent={last_sent}"
            )

            # Discard stale updates (already sent a higher updateId for this order)
            if update_id <= last_sent:
                printYellow(
                    f"[QUEUE] Discarding stale update for {robot_id}: "
                    f"updateId={update_id} <= lastSent={last_sent}"
                )
                queue.pop(0)
                # Continue processing the queue
                if queue:
                    # Release lock before recursive call
                    self.queue_lock.release()
                    try:
                        self._process_queue(robot_id)
                    finally:
                        self.queue_lock.acquire()
                return

            expected_update_id = last_sent + 1

            # Case 1: This is the expected update (or first update for new order)
            if update_id == expected_update_id or (last_sent == -1 and update_id == 0):
                self._send_order(robot_id, order, queue)
                return

            # Case 2: updateId=0 is missing, but we have updateId>0 for a NEW order
            # Reset the first available updateId to 0
            if last_sent == -1 and update_id > 0:
                printYellow(
                    f"[QUEUE] updateId=0 missing for {robot_id}, "
                    f"resetting updateId={update_id} to 0"
                )
                order.orderUpdateId = 0
                self._send_order(robot_id, order, queue)
                return

            # Case 3: Gap in updateIds (e.g., have 3 but need 2)
            # Check if we've been waiting too long for the missing update
            timer_key = f"{robot_id}_{order_id}_{expected_update_id}"
            if timer_key not in self.queue_timers:
                printYellow(
                    f"[QUEUE] Gap detected for {robot_id}: "
                    f"have updateId={update_id}, need updateId={expected_update_id}. "
                    f"Waiting {self.missing_update_timeout}s..."
                )
                # Start a timer to process after timeout
                timer = threading.Timer(
                    self.missing_update_timeout,
                    self._on_timeout,
                    args=[robot_id, order_id, expected_update_id]
                )
                self.queue_timers[timer_key] = timer
                timer.start()

    def _send_order(self, robot_id, order, queue):
        """Send order to AGV and update tracking state."""
        order_id = order.orderId
        # Track the actual updateId that will be sent (may have been reset to 0)
        actual_update_id = order.orderUpdateId

        printGreen(
            f"[QUEUE] Sending order to {robot_id}: "
            f"orderId={order_id}, updateId={actual_update_id}"
        )

        # Update last sent tracking
        self.last_sent_updateId[robot_id][order_id] = actual_update_id

        # Remove from queue
        queue.pop(0)

        # Cancel any pending timeout timer for this order
        for key in list(self.queue_timers.keys()):
            if key.startswith(f"{robot_id}_{order_id}_"):
                timer = self.queue_timers.pop(key, None)
                if timer:
                    timer.cancel()

        # Send to the AGV (release lock first to avoid deadlock)
        self.queue_lock.release()
        try:
            self.vda5050_master.send_order_to_agv(order)

            # Process next item in queue if any
            self._process_queue(robot_id)
        finally:
            self.queue_lock.acquire()

    def _on_timeout(self, robot_id, order_id, expected_update_id):
        """
        Called when we've waited too long for a missing update.
        Skip the gap and send the next available update.
        """
        timer_key = f"{robot_id}_{order_id}_{expected_update_id}"

        with self.queue_lock:
            # Remove the timer reference
            self.queue_timers.pop(timer_key, None)

            queue = self.command_queues[robot_id]
            if not queue:
                return

            # Find the next available order for this orderId
            for i, order in enumerate(queue):
                if order.orderId == order_id and order.orderUpdateId > expected_update_id - 1:
                    last_sent = self.last_sent_updateId[robot_id][order_id]
                    if order.orderUpdateId <= last_sent:
                        # Already sent a higher one, discard
                        queue.pop(i)
                        continue

                    printYellow(
                        f"[QUEUE] Timeout waiting for updateId={expected_update_id}. "
                        f"Skipping gap, sending updateId={order.orderUpdateId}"
                    )
                    self._send_order(robot_id, order, queue)
                    return

    # Converts VDA5050 State Message to rmf2_agv StateMessage
    def process_vda5050_state(self, vda_state, robot_id):
        tokens = robot_id.split("_")
        manufacturer = tokens[0]
        serial_number = tokens[1]
        state_message = convert_vda_state_to_state_message(
            vda_state, self.vda5050_master.map
        )
        state_message = StateMessage.to_json(
            id="urn:ngsi-ld:StateMessage:" + manufacturer + ":" + serial_number,
            input_msg=state_message,
        )
        if self.context_broker:
            response = self.context_broker.send_entity_message(
                state_message, self.headers
            )
        else:
            printRed("Context Broker is not available yet...")


def get_map(map_name: str, context_broker: ScorpioAPI) -> json:
    status_code = 0
    response = None
    while status_code > 299 or status_code < 200:
        printRed("Context Broker Connection not established yet, cannot set map.")
        response = context_broker.get_entity("urn:ngsi-ld:Map:" + map_name, {})
        status_code = response.status_code
    printGreen("Map Loaded Successfully")
    map_json = json.loads(response.text)
    return map_json


def main():
    # Default Parameters if nothing is set.
    mqtt_url = "0.0.0.0"
    mqtt_port = 1927
    cb_endpoint = "http://scorpio:9090"
    interface_name = "uagv"
    version = "v2"
    map_name = "RMF1"

    # Parse from Python Args
    if len(sys.argv) > 1:
        mqtt_url = sys.argv[1]
    if len(sys.argv) > 2:
        mqtt_port = sys.argv[2]
    if len(sys.argv) > 3:
        cb_endpoint = sys.argv[3]
    if len(sys.argv) > 4:
        map_name = sys.argv[4]

    context_broker = ScorpioAPI(cb_endpoint)

    vda_map = convert_json_to_vda_map(
        Map.from_json(get_map(map_name=map_name, context_broker=context_broker))
    )

    vda5050_master = Vda5050Master(
        version, interface_name, mqtt_url, int(mqtt_port), vda_map
    )

    scorpio = Vda5050Fiware(context_broker, vda5050_master)


if __name__ == "__main__":
    main()
