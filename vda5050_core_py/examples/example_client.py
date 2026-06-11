import threading
import time

from vda5050_core_py import RobotRuntime, run_until_signal

runtime = RobotRuntime(
    broker="tcp://localhost:1883",
    client_id="my_agv",
    manufacturer="Manufacturer",
    serial_number="S001",
)
rep = runtime.reporter()


def _drive(node):
    # Model a real robot move: report driving, travel, arrive, stop.
    print(f"Hello sir I'm walking to node {node.node_id}")
    rep.set_driving(True)
    time.sleep(0.5)  # simulate travel time
    rep.set_driving(False)  # arrived / stopped
    rep.node_reached(node.node_id, node.sequence_id)


def on_navigate(node, edge):
    # Return immediately; drive on a worker thread so the spin loop stays free.
    threading.Thread(target=_drive, args=(node,), daemon=True).start()


runtime.on_navigate(on_navigate)
runtime.start()
run_until_signal(runtime)
