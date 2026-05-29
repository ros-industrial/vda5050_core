#!/usr/bin/env python3
"""GameTL POC client: vda5050_core_py Adapter + Autoxing REST/WebSocket I/O.

Implements diagram steps ⑤–⑨: on_navigate → POST /chassis/moves → poll pose/status
→ set_agv_position/set_driving → node_reached. Node iteration stays in C++.

Usage::

    # Terminal 1 — AGV adapter + Autoxing bridge
    python3 vda5050_core_py/autoxing-l300/example_autoxing_client.py

    # Terminal 2 — publish demo order (requires mosquitto)
    python3 vda5050_core_py/autoxing-l300/publish_autoxing_l300_route.py --wait-route

Prerequisites: spellbook CONSTANTS.yml configured; robot map loaded and localized.

Active Autoxing map (``get_current_map``) — VDA5050 ``nodePosition.mapId`` must use ``map_name``:

.. code-block:: json

    {
      "id": 15,
      "uid": "69fb0226fe07afec2a1e2c67",
      "map_name": "LargeARTC",
      "create_time": 1778057765,
      "map_version": 0,
      "overlays_version": 2
    }

Map id flows: order JSON ``mapId`` → ``node.node_position.map_id`` → published state
``agvPosition.mapId``. Autoxing navigate uses x/y only; map must already be loaded on robot.

``on_navigate`` must return quickly — blocking work runs on a worker thread so C++
``suspend_for<NodeAckUpdate>`` can receive ``node_reached`` and advance to the next node.
"""

# TODO & State of the current example
# - NodePosition w/o theta, fixed map_id

from __future__ import annotations

import sys
import threading
import traceback

import vda5050_core_py as vda

from autoxing_bridge import dispatch_move, wait_for_arrival

CONFIG = {
    "broker": "tcp://localhost:1883",
    "client_id": "autoxing_l300_agv",
    "interface": "uagv",
    "protocol_version": "2.0.0",
    "manufacturer": "Manufacturer",
    "serial_number": "S001",
    "poll_interval": 0.5,
    "nav_timeout": 300.0,
}


def _drive_to_node(
    node: vda.Node,
    nav: vda.NavigationManager,
    *,
    nav_timeout: float,
    poll_interval: float,
) -> None:
    """Blocking Autoxing navigate + poll; calls node_reached on the worker thread."""
    try:
        pos = node.node_position
        print(
            f"Navigate to {node.node_id} "
            f"({pos.x}, {pos.y}, theta={pos.theta}) map={pos.map_id!r}",
            file=sys.stderr,
        )
        nav.set_driving(True)

        move = dispatch_move(node)
        if move is None:
            nav.set_driving(False)
            raise RuntimeError(f"Autoxing navigate failed for node {node.node_id}")

        move_id = move.get("id")
        print(f"  move id={move_id}", file=sys.stderr)

        map_id = pos.map_id or ""

        def mirror_pose(agv: vda.AGVPosition, move_state: str | None) -> None:
            if agv.position_initialized:
                nav.set_agv_position(agv)
            if move_state:
                print(f"  move_state={move_state}", file=sys.stderr)

        terminal = wait_for_arrival(
            pos.x,
            pos.y,
            move_id=move_id,
            map_id=map_id,
            timeout_s=nav_timeout,
            poll_interval_s=poll_interval,
            on_poll=mirror_pose,
        )

        nav.set_driving(False)
        if terminal != "succeeded":
            raise RuntimeError(
                f"Move to {node.node_id} ended with state {terminal!r}"
            )

        print(f"  node_reached({node.node_id})", file=sys.stderr)
        nav.node_reached(node)
    except Exception as exc:
        nav.set_driving(False)
        print(
            f"Navigation failed for {node.node_id}: {exc}",
            file=sys.stderr,
        )
        traceback.print_exc(file=sys.stderr)


def main() -> int:
    mqtt = vda.create_default_mqtt_client(CONFIG["broker"], CONFIG["client_id"])
    protocol = vda.ProtocolAdapter.make(
        mqtt,
        CONFIG["interface"],
        CONFIG["protocol_version"],
        CONFIG["manufacturer"],
        CONFIG["serial_number"],
    )
    adapter = vda.Adapter.make(protocol)
    nav = adapter.navigation_manager()

    def on_navigate(node: vda.Node) -> None:
        threading.Thread(
            target=_drive_to_node,
            args=(node, nav),
            kwargs={
                "nav_timeout": CONFIG["nav_timeout"],
                "poll_interval": CONFIG["poll_interval"],
            },
            daemon=True,
        ).start()

    adapter.on_navigate(on_navigate)
    adapter.start()
    print(
        f"Adapter started ({CONFIG['interface']}/v2/"
        f"{CONFIG['manufacturer']}/{CONFIG['serial_number']})",
        file=sys.stderr,
    )
    vda.run_until_signal(adapter)
    return 0


if __name__ == "__main__":
    sys.exit(main())

