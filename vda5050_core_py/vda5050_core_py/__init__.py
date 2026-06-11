"""Python bindings for the vda5050_core C++ runtime.

C++ owns the runtime — MQTT, protocol handling, order execution, threading and
1Hz state publishing. Python implements only robot behaviour: a navigation
callback and state reporting via Reporter.

Typical usage:

    from vda5050_core_py import RobotRuntime, run_until_signal

    runtime = RobotRuntime(
        broker="tcp://localhost:1883",
        client_id="my_agv",
        manufacturer="MyCompany",
        serial_number="AGV_001",
    )
    rep = runtime.reporter()

    def on_navigate(node, edge):
        # ... drive the robot to node.node_position (edge has trajectory) ...
        rep.node_reached(node.node_id, node.sequence_id)

    runtime.on_navigate(on_navigate)
    runtime.start()
    run_until_signal(runtime)
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
    AGVPosition,
    BatteryState,
    BlockingType,
    Edge,
    Node,
    NodePosition,
    Order,
    Reporter,
    RobotRuntime,
)

_PAUSE_MSG = (
    "Paused (Ctrl+Z). Resume in this shell: fg   "
    "(or: bg, then fg). Ctrl+C still exits cleanly.\n"
)
_RESUME_MSG = "Resumed (fg/bg).\n"


def run_until_signal(runtime, poll_interval: float = 0.1) -> None:
    """Block until a shutdown signal, then stop the runtime.

    Graceful exit (Unix): SIGINT (Ctrl+C), SIGTERM, SIGQUIT (Ctrl+\\).
    On Windows only SIGINT and SIGTERM are used for exit.

    Job control (Unix): SIGTSTP (Ctrl+Z) calls ``runtime.stop()`` (MQTT
    OFFLINE), suspends the process, and prints a hint. SIGCONT (``fg`` /
    ``bg`` then ``fg``) calls ``runtime.start()`` again without exiting.

    A signal handler clears a flag, the main thread exits its wait loop, and
    runtime.stop() runs on the main thread (safe for pybind/C++ teardown).
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
        runtime.stop()
        paused = True
        print(_PAUSE_MSG, file=sys.stderr, end="", flush=True)
        signal.signal(signal.SIGTSTP, signal.SIG_DFL)
        os.kill(os.getpid(), signal.SIGTSTP)

    def _on_cont(signum, frame):
        nonlocal paused
        if not paused:
            return
        runtime.start()
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

    try:
        while running:
            time.sleep(poll_interval)
    finally:
        runtime.stop()


__all__ = [
    "Action",
    "ActionParameter",
    "ActionState",
    "ActionStatus",
    "AGVPosition",
    "BatteryState",
    "BlockingType",
    "Edge",
    "Node",
    "NodePosition",
    "Order",
    "Reporter",
    "RobotRuntime",
    "run_until_signal",
]
