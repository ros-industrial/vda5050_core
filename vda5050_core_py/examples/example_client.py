from vda5050_core_py import (
    Adapter,
    ProtocolAdapter,
    create_default_mqtt_client,
    run_until_signal,
)

mqtt = create_default_mqtt_client("tcp://localhost:1883", "my_agv")
protocol = ProtocolAdapter.make(mqtt, "uagv", "2.0.0", "Manufacturer", "S001")
adapter = Adapter.make(protocol)
nav = adapter.navigation_manager()


def on_navigate(node):
    # ... drive the robot to node.node_position ...
    print(f"Hello sir I'm walking to the sent node {node}")
    nav.node_reached(node)


adapter.on_navigate(on_navigate)
adapter.start()
run_until_signal(adapter)
