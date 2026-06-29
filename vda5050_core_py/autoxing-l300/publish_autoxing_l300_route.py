#!/usr/bin/env python3
"""Publish GameTL Autoxing L300 demo route order and verify adapter state via MQTT."""

import argparse
import copy
import json
import sys
import threading
import time
from datetime import datetime, timezone

import paho.mqtt.client as mqtt

BROKER = "localhost"
PORT = 1883
ORDER_TOPIC = "uagv/v2/Manufacturer/S001/order"
STATE_TOPIC = "uagv/v2/Manufacturer/S001/state"
ORDER_ID = "gametl_autoxing_l300"
ROUTE_TIMEOUT_S = 600.0

# Autoxing active map (GET /chassis/current-map via spellbook get_current_map):
# {
#   "id": 15,
#   "uid": "69fb0226fe07afec2a1e2c67",
#   "map_name": "LargeARTC",
#   "create_time": 1778057765,
#   "map_version": 0,
#   "overlays_version": 2
# }
MAP_ID = "LargeARTC"

ORDER = {
    "headerId": 1,
    "timestamp": "",
    "version": "2.0.0",
    "manufacturer": "Manufacturer",
    "serialNumber": "S001",
    "orderId": ORDER_ID,
    "orderUpdateId": 0,
    "nodes": [
        {
            "nodeId": "node_table",
            "sequenceId": 0,
            "released": True,
            "actions": [],
            "nodePosition": {"x": -11.0, "y": 1.2, "mapId": MAP_ID},
        },
        {
            "nodeId": "node_row",
            "sequenceId": 2,
            "released": True,
            "actions": [],
            "nodePosition": {"x": -11.0, "y": 3.0, "mapId": MAP_ID},
        },
        {
            "nodeId": "node_west",
            "sequenceId": 4,
            "released": True,
            "actions": [],
            "nodePosition": {"x": -16.0, "y": 3.0, "mapId": MAP_ID},
        },
        {
            "nodeId": "node_east",
            "sequenceId": 6,
            "released": True,
            "actions": [],
            "nodePosition": {"x": -6.0, "y": 3.0, "mapId": MAP_ID},
        },
    ],
    "edges": [
        {
            "edgeId": "edge_table_row",
            "sequenceId": 1,
            "released": True,
            "startNodeId": "node_table",
            "endNodeId": "node_row",
            "actions": [],
        },
        {
            "edgeId": "edge_row_west",
            "sequenceId": 3,
            "released": True,
            "startNodeId": "node_row",
            "endNodeId": "node_west",
            "actions": [],
        },
        {
            "edgeId": "edge_west_east",
            "sequenceId": 5,
            "released": True,
            "startNodeId": "node_west",
            "endNodeId": "node_east",
            "actions": [],
        },
    ],
}


def order_payload() -> str:
    msg = copy.deepcopy(ORDER)
    msg["headerId"] = 4
    msg["timestamp"] = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.000Z")
    return json.dumps(msg)


def accepted(state: dict) -> bool:
    return (
        state.get("orderId") == ORDER_ID
        and state.get("orderUpdateId", 0) == 0
        and len(state.get("nodeStates") or []) == 4
        and len(state.get("edgeStates") or []) == 3
    )


def route_done(state: dict) -> bool:
    return (
        state.get("lastNodeId") == "node_east"
        and state.get("lastNodeSequenceId", 0) == 6
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wait-route", action="store_true")
    parser.add_argument("--broker", default=BROKER)
    parser.add_argument("--port", type=int, default=PORT)
    parser.add_argument(
        "--route-timeout",
        type=float,
        default=ROUTE_TIMEOUT_S,
        help=f"seconds to wait for route completion (default: {ROUTE_TIMEOUT_S})",
    )
    args = parser.parse_args()

    latest = {}
    updated = threading.Event()

    def on_message(_c, _u, msg):
        try:
            latest.clear()
            latest.update(json.loads(msg.payload.decode()))
        except (json.JSONDecodeError, UnicodeDecodeError):
            return
        updated.set()

    client = mqtt.Client()
    client.on_message = on_message
    client.connect(args.broker, args.port, 60)
    client.subscribe(STATE_TOPIC)
    client.loop_start()
    time.sleep(0.3)

    print(f"Publishing order to {ORDER_TOPIC} ...")
    client.publish(ORDER_TOPIC, order_payload())

    def wait_for(check, timeout, label):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            updated.wait(0.25)
            updated.clear()
            if latest and check(latest):
                print(f"{label}: PASS")
                return True
        print(f"{label}: FAIL", file=sys.stderr)
        if latest:
            print(json.dumps(latest, indent=2), file=sys.stderr)
        return False

    print("Phase A: waiting for order acceptance ...")
    if not wait_for(accepted, 5.0, "Phase A"):
        client.loop_stop()
        return 1

    if not args.wait_route:
        client.loop_stop()
        print("Done. Use --wait-route for full navigation check.")
        return 0

    print(f"Phase B: waiting for route completion (timeout {args.route_timeout}s) ...")
    ok = wait_for(route_done, args.route_timeout, "Phase B")
    client.loop_stop()
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
