from vda5050_core_py import RobotRuntime, run_until_signal

runtime = RobotRuntime(
    broker="tcp://localhost:1883",
    client_id="my_agv",
    manufacturer="Manufacturer",
    serial_number="S001",
)
rep = runtime.reporter()


def on_navigate(node, edge):
    # ... drive the robot to node.node_position (edge has the trajectory) ...
    print(f"Hello sir I'm walking to the sent node {node}")
    rep.node_reached(node.node_id, node.sequence_id)


runtime.on_navigate(on_navigate)
runtime.start()
run_until_signal(runtime)
