"""Python bindings for the vda5050_core C++ Adapter.

The C++ adapter owns MQTT, order graph execution, and 1Hz state publishing.
Python implements the per-node navigation callback and reports state via
NavigationManager.

Typical usage:

    from vda5050_core_py import (
        Adapter, ProtocolAdapter, create_default_mqtt_client,
        run_until_signal,
    )

    mqtt = create_default_mqtt_client("tcp://localhost:1883", "my_agv")
    protocol = ProtocolAdapter.make(mqtt, "uagv", "2.0.0", "Manufacturer", "S001")
    adapter = Adapter.make(protocol)
    nav = adapter.navigation_manager()

    def on_navigate(node):
        # ... drive the robot to node.node_position ...
        nav.node_reached(node)

    adapter.on_navigate(on_navigate)
    adapter.start()
    run_until_signal(adapter)
"""

import os
import signal
import sys
import time

from ._core import (
    Action,
    ActionParameter,
    ActionState,
    ActionStatus,
    Adapter,
    AGVPosition,
    BatteryState,
    BlockingType,
    MqttClient,
    NavigationManager,
    Node,
    NodePosition,
    ProtocolAdapter,
    create_default_mqtt_client,
)

_PAUSE_MSG = (
    "Paused (Ctrl+Z). Resume in this shell: fg   "
    "(or: bg, then fg). Ctrl+C still exits cleanly.\n"
)
_RESUME_MSG = "Resumed (fg/bg).\n"


def run_until_signal(adapter, poll_interval: float = 0.1) -> None:
    """Block until a shutdown signal, then stop the adapter.

    Graceful exit (Unix): SIGINT (Ctrl+C), SIGTERM, SIGQUIT (Ctrl+\\).
    On Windows only SIGINT and SIGTERM are used for exit.

    Job control (Unix): SIGTSTP (Ctrl+Z) calls ``adapter.stop()`` (MQTT
    OFFLINE), suspends the process, and prints a hint. SIGCONT (``fg`` /
    ``bg`` then ``fg``) calls ``adapter.start()`` again without exiting.

    Mirrors the shutdown pattern in vda5050_core/examples/adapter_example.cpp:
    a signal handler clears a flag, the main thread exits its wait loop, and
    adapter.stop() runs on the main thread (safe for pybind/C++ teardown).
    """
    running = True
    paused = False

    def _on_shutdown(signum, frame):
        nonlocal running
        running = False

    def _on_tstp(signum, frame):
        nonlocal paused
        if paused:
            return
        adapter.stop()
        paused = True
        print(_PAUSE_MSG, file=sys.stderr, end="", flush=True)
        signal.signal(signal.SIGTSTP, signal.SIG_DFL)
        os.kill(os.getpid(), signal.SIGTSTP)

    def _on_cont(signum, frame):
        nonlocal paused
        if not paused:
            return
        adapter.start()
        paused = False
        print(_RESUME_MSG, file=sys.stderr, end="", flush=True)

    for sig in (signal.SIGINT, signal.SIGTERM):
        signal.signal(sig, _on_shutdown)

    if sys.platform != "win32":
        if hasattr(signal, "SIGQUIT"):
            signal.signal(signal.SIGQUIT, _on_shutdown)
        if hasattr(signal, "SIGTSTP"):
            signal.signal(signal.SIGTSTP, _on_tstp)
        if hasattr(signal, "SIGCONT"):
            signal.signal(signal.SIGCONT, _on_cont)
    # so handler for windows byebye

    try:
        while running:
            time.sleep(poll_interval)
    finally:
        adapter.stop()


__all__ = [
    "Action",
    "ActionParameter",
    "ActionState",
    "ActionStatus",
    "Adapter",
    "AGVPosition",
    "BatteryState",
    "BlockingType",
    "MqttClient",
    "NavigationManager",
    "Node",
    "NodePosition",
    "ProtocolAdapter",
    "create_default_mqtt_client",
    "run_until_signal",
]
