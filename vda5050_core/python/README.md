# VDA5050 Library and Support Tools

This package contains Python bindings for [vda5050_core](https://github.com/ros-industrial/vda5050_core)

See the [documentation](https://github.com/ros-industrial/vda5050_core/blob/main/README.md) for more information.

Supported Platforms
- **Python**: 3.10+
- **OS:** Linux, MacOS (15+)
- **Architecture:** x86_64, arm64

```shell
pip install vda5050_core
```

### Examples

Sample applications can be found in the source repository at
[`vda5050_core/examples/python`](https://github.com/ros-industrial/vda5050_core/tree/main/vda5050_core/examples/python).

You can launch a local MQTT broker to test them.

```shell
mosquitto -v -p 1883
```

Below are some quick examples

#### AGV Client Integration

The following example shows the basic setup for an AGV-side client.

```python
from vda5050_core.rmf_migration import (
    Adapter,
    FleetConfiguration,
    RobotCallbacks,
    RobotConfiguration,
    RobotState,
)

MAP_ID = "demo-map"


def main() -> None:
    adapter = Adapter.make()
    fleet_config = FleetConfiguration(
        fleet_name="demo",
        broker_uri="tcp://localhost:1883",
        client_id_prefix="agv_1",
    )
    fleet = adapter.add_vda5050_fleet(fleet_config)

    robot_config = RobotConfiguration(
        manufacturer="Manufacturer",
        serial_number="S001",
        interface_name="uagv",
        version="2.0.0",
    )
    initial_state = RobotState(MAP_ID, [0.0, 0.0, 0.0], 1.0)

    robot_handle = None

    def navigate(destination, execution) -> None:
        # Forward the request to the robot navigation system.
        #
        # This demonstration reports completion immediately.
        # A real integration should only report completion after
        # the robot reaches the requested node.
        execution.finished()

    def stop() -> None:
        pass

    def execute_action(action_type, action_id, execution) -> None:
        execution.finished()

    callbacks = RobotCallbacks(navigate, stop, execute_action)
    robot_handle = fleet.add_robot(
        "robot-1",
        initial_state,
        robot_config,
        callbacks,
    )

    adapter.start()

    # Keep processing orders until Enter is pressed.
    input()
    adapter.stop()


if __name__ == "__main__":
    main()
```

For a complete integration covering navigation, actions, localization, cancellation and state reporting,
see the [Client Adapter Guide](https://github.com/ros-industrial/vda5050_core/tree/main/docs/client-adapter.md) and [Open-RMF Migration Guide](https://github.com/ros-industrial/vda5050_core/tree/main/docs/rmf-migration.md).

#### Master Control Integration

The following example shows the basic setup for a master.

```python
from logging import getLogger
from time import sleep

from vda5050_core.master import VDA5050Master, AGVState, OrderAssignmentDecision
from vda5050_core.transport import create_default_client_shared
from vda5050_core.types import Order, Node, Edge

LOGGER = getLogger(__name__)


def main() -> None:
    mqtt_client = create_default_client_shared("tcp://localhost:1883", "master_1")
    master = VDA5050Master.make(mqtt_client)

    def on_order_complete(agv_id, order_id):
        LOGGER.info(f"[{agv_id} completed order {order_id}]")

        # A real integration would assign this AGV's next order here, with a
        # new order id, or return the AGV to its task queue.


    master.on_order_complete(on_order_complete)

    master.connect()
    master.onboard_agv("Manufacturer", "S001")

    # Wait until the AGV is online, localized and idle.
    agv = master.get_agv("Manufacturer", "S001")
    while agv.get_operational_state() != AGVState.AVAILABLE:
        sleep(0.2)

    # A simple order: drive from node N0 to node N1.
    # Nodes take even sequence ids, the edges between them the odd ones.
    order = Order()
    order.order_id = "order-1"
    order.order_update_id = 0
        position = state.agv_position
        pose = None if position is None else (position.x, position
    order.nodes = [
            Node.from_json({"nodeId": "N0", "sequenceId": 0, "released": True, "actions": []}),
            Node.from_json({"nodeId": "N1", "sequenceId": 2, "released": True, "actions": []}),
    ]

    order.edges = [
        Edge.from_json({
            "edgeId": "E0",
            "sequenceId": 1,
            "startNodeId": "N0",
            "endNodeId": "N1",
            "released": True,
            "actions": []
        })
    ]

    result = master.assign_order("Manufacturer", "S001", order)

    if result.decision != OrderAssignmentDecision.ASSIGNED:
        # A real integration should read result.decision and result.errors to
        # decide whether to retry, hand the task to another AGV, or raise it to
        # an operator.
        LOGGER.warning(f"Order [{order.order_id}] not assigned ({len(result.errors)})")

    # Keep the master running until Enter is pressed.
    input()

    master.disconnect()

if __name__ == "__main__":
    main()
```

For a complete integration covering order construction, validation, event handling and multi-AGV
dispatch, see the [Master Guide](https://github.com/ros-industrial/vda5050_core/tree/main/docs/master.md).
