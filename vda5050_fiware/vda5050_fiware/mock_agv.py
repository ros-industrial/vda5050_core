import time
import sys
import json
from datetime import datetime
import paho.mqtt.client as paho
from vda5050_fiware.order import OrderMessage, Node, Edge, NodePosition
from vda5050_fiware.state import (
    State,
    NodeState,
    NodePosition,
    EdgeState,
    Map,
    SafetyState,
    BatteryState,
    OperatingMode,
    AgvPosition,
    EStop,
)
from threading import Thread, Lock
from fiware_api.utils import (
    printRed,
    printGreen,
    printYellow,
    printPurple,
    printCyan,
    MQTTClient,
)

interface_name = "uagv"
topic_version = "v2"
version = "2.0.0"
state_frequency = 2.0
sleep_time = 0.1


class MockOrderHandler:
    def mark_next_as_complete(state: State) -> State:
        output_state = state
        output_state.driving = True
        if (
            len(output_state.nodeStates) == len(output_state.edgeStates)
            and len(output_state.edgeStates) > 0
        ):
            # This means next step to complete is edge
            output_state.edgeStates.pop(0)
        else:
            if len(output_state.nodeStates) > 0:
                node = output_state.nodeStates.pop(0)
                output_state.lastNodeId = node.nodeId
                output_state.lastNodeSequenceId = node.sequenceId
                output_state.agvPosition = AgvPosition(
                    x=node.nodePosition.x,
                    y=node.nodePosition.y,
                    theta=0.0,
                    mapId=node.nodePosition.mapId,
                    positionInitialized=True,
                )
                if node.nodePosition.theta:
                    output_state.agvPosition.theta = node.nodePosition.theta
        return output_state

    def apply_order_to_state(order: OrderMessage, state: State) -> State:
        output_state = state
        output_state.orderId = order.orderId
        output_state.orderUpdateId = order.orderUpdateId

        # Sort the current path to complete based on the Sequence ID
        sorted_nodes = sorted(
            order.nodes,
            key=lambda node: node.sequenceId,
        )

        sorted_edges = sorted(
            order.edges,
            key=lambda edge: edge.sequenceId,
        )

        if state.orderId != order.orderId and not state.driving:
            printPurple(
                "Received Order from:"
                + "\n MQTT Topic: "
                + ". Order ID : "
                + str(order.orderId)
            )
        if (
            state.orderId == order.orderId
            and state.orderUpdateId != order.orderUpdateId
        ):
            printPurple(
                "Received command to update Order "
                + order.orderId
                + ". Order Update ID : "
                + str(order.orderUpdateId)
            )

            # Check where the current node state is at
            temp_nodes = []
            temp_edges = []
            for sorted_node in sorted_nodes:
                if sorted_node.sequenceId > state.lastNodeSequenceId:
                    temp_nodes.append(sorted_node)

            for sorted_edge in sorted_edges:
                if sorted_edge.sequenceId > state.lastNodeSequenceId:
                    temp_edges.append(sorted_edge)

        for node in sorted_nodes:
            output_state.nodeStates.append(
                NodeState(
                    nodeId=node.nodeId,
                    sequenceId=node.sequenceId,
                    released=node.released,
                    nodePosition=NodePosition(
                        x=node.nodePosition.x,
                        y=node.nodePosition.y,
                        theta=node.nodePosition.theta,
                        mapId=node.nodePosition.mapId,
                    ),
                )
            )

        for edge in sorted_edges:
            output_state.edgeStates.append(
                EdgeState(
                    edgeId=edge.edgeId,
                    sequenceId=edge.sequenceId,
                    released=edge.released,
                )
            )

        # Auto Initialize Position to first node. Set to drive
        return MockOrderHandler.mark_next_as_complete(output_state)


class Vda5050MockAgv:
    def __init__(self, manufacturer: str, serial_number: str) -> None:
        self.state = State(
            headerId=0,
            timestamp=datetime.now(),
            version=version,
            manufacturer=manufacturer,
            serialNumber=serial_number,
            orderId="",
            orderUpdateId=0,
            lastNodeId="",
            lastNodeSequenceId=0,
            driving=False,
            operatingMode="AUTOMATIC",
            safetyState=SafetyState(eStop="NONE", fieldViolation=False),
            batteryState=BatteryState(batteryCharge=0.8, charging=False),
            nodeStates=[],
            edgeStates=[],
            actionStates=[],
            errors=[],
            information=[],
        )
        printCyan(json.dumps(State.to_json(self.state)))
        self.manufacturer = manufacturer
        self.serial_number = serial_number
        self.last_published_state = time.time()
        self.state_frequency = state_frequency
        self.sleep_time = sleep_time
        self.mutex = Lock()

    def setup_mqtt(self, mqtt_url: str, mqtt_port: int):
        self.mqtt_url = mqtt_url
        self.mqtt_port = mqtt_port
        printCyan(
            "Listening for Orders on Topic "
            + interface_name
            + "/"
            + topic_version
            + "/"
            + self.manufacturer
            + "/"
            + self.serial_number
            + "/order"
        )
        # Mqtt Client to read from message broker
        self.order_client = MQTTClient(
            id="Mock AGV Client " + self.manufacturer + "_" + self.serial_number,
            host=self.mqtt_url,
            port=self.mqtt_port,
            topic=interface_name
            + "/"
            + topic_version
            + "/"
            + self.manufacturer
            + "/"
            + self.serial_number
            + "/order",
            qos=0,
            keepalive=0,
            on_message_callback=self.handle_order_message,
        )
        Thread(target=lambda: self.state_thread()).start()

    def handle_order_message(self, client, userdata, msg):
        message = str(msg.payload.decode("utf-8", "ignore"))
        order_json = json.loads(str(message))
        order = OrderMessage.from_json(order_json)
        if self.state.orderId != order.orderId and self.state.driving:
            printRed(
                "AGV is currently executing "
                + " Order "
                + self.state.orderId
                + " , cannot accept new order "
                + order.orderId
            )
            return
        if (
            self.state.orderId == order.orderId
            and self.state.orderUpdateId == order.orderUpdateId
        ):
            printRed(
                "AGV received same Order Id "
                + str(self.state.orderId)
                + " and same orderUpdateId "
                + str(self.state.orderUpdateId)
                + " . Nothing New to Execute"
            )
            return

        with self.mutex:
            self.state = MockOrderHandler.apply_order_to_state(order, self.state)
        while len(self.state.nodeStates) > 0:
            time.sleep(state_frequency + 2.0)
            with self.mutex:
                printYellow(
                    self.state.manufacturer
                    + "_"
                    + self.state.serialNumber
                    + ": Traversing to Next Node"
                )
                self.state = MockOrderHandler.mark_next_as_complete(self.state)
                # printRed(json.dumps(State.to_json(self.state), indent=2))
        printGreen(
            self.state.manufacturer + "_" + self.state.serialNumber + ": Completed"
        )

        # Stop AGV
        self.state.driving = False

    def state_thread(self):
        while True:
            self.publish_state()

    def publish_state(self):
        time_now = time.time()
        time_elapsed = time_now - self.last_published_state
        state_topic = (
            interface_name
            + "/"
            + topic_version
            + "/"
            + self.manufacturer
            + "/"
            + self.serial_number
            + "/state"
        )
        # printRed("Time elapsed: " + str(time_elapsed))
        if time_elapsed > self.state_frequency:
            with self.mutex:
                self.state.headerId = self.state.headerId + 1
                self.state.timestamp = datetime.now()

            self.state_client = paho.Client(
                client_id="Mock AGV State Pub ("
                + self.manufacturer
                + " "
                + self.serial_number
                + ")"
                + str(datetime.now())
            )
            self.state_client.connect(self.mqtt_url, self.mqtt_port)
            self.state_client.loop_start()
            self.state_client.loop_stop()
            self.state_client.publish(
                state_topic, json.dumps(State.to_json(self.state)), qos=0
            )
            self.last_published_state = time.time()

            # printCyan(
            #     "Sending State to:"
            #     + "\n\t MQTT Topic: "
            #     + str(state_topic)
            #     + "\n\t Content: "
            #     + json.dumps(State.to_json(self.state), indent=2)
            # )

            time.sleep(self.sleep_time)

    # def on_publish(self, client, userdata, mid):
    #     print("mid: " + str(mid))


def main():
    # Default Parameters if nothing is set.
    mqtt_url = "172.20.0.1"
    mqtt_port = 1928
    num_robots = 4

    # Parse from Python Args
    if len(sys.argv) > 1:
        mqtt_url = sys.argv[1]
    if len(sys.argv) > 2:
        mqtt_port = sys.argv[2]
    if len(sys.argv) > 3:
        num_robots = sys.argv[3]

    manufacturer = "MIR"
    robots = []
    for i in range(int(num_robots)):
        serial_number = str(i)
        robots.append(
            Vda5050MockAgv(
                manufacturer=manufacturer, serial_number=serial_number
            ).setup_mqtt(mqtt_port=int(mqtt_port), mqtt_url=mqtt_url)
        )


if __name__ == "__main__":
    main()
