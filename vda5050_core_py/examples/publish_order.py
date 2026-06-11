#!/usr/bin/env python3
"""Publish a demo VDA5050 order over MQTT and print the AGV's State back.

The master side of the interactive demo. Run an AGV client first (e.g.
examples/example_client.py), then run this to send it an order and watch the
state messages it publishes.

    mosquitto -p 1883 &
    # Terminal 1:
    source install/setup.bash
    python3 vda5050_core_py/examples/example_client.py
    # Terminal 2:
    python3 vda5050_core_py/examples/publish_order.py

Targets the uagv/2.0.0/... topics that example_client.py / RobotRuntime use by
default. No vda5050_core_py import needed here — this is a plain MQTT publisher.
"""
from __future__ import annotations

import json
import time
from datetime import datetime, timezone

import paho.mqtt.client as mqtt

INTERFACE, VERSION, MANUF, SERIAL = "uagv", "2.0.0", "Manufacturer", "S001"
PREFIX = f"{INTERFACE}/{VERSION}/{MANUF}/{SERIAL}"
ORDER_TOPIC = f"{PREFIX}/order"
STATE_TOPIC = f"{PREFIX}/state"
MAP_ID = "demo_map"

ORDER = {
    "headerId": 1,
    "timestamp": "",
    "version": VERSION,
    "manufacturer": MANUF,
    "serialNumber": SERIAL,
    "orderId": "demo_order",
    "orderUpdateId": 0,
    "nodes": [
        {"nodeId": "node_table", "sequenceId": 0, "released": True,
         "actions": [], "nodePosition": {"x": -11.0, "y": 1.2, "mapId": MAP_ID}},
        {"nodeId": "node_row", "sequenceId": 2, "released": True,
         "actions": [], "nodePosition": {"x": -11.0, "y": 3.0, "mapId": MAP_ID}},
        {"nodeId": "node_west", "sequenceId": 4, "released": True,
         "actions": [], "nodePosition": {"x": -16.0, "y": 3.0, "mapId": MAP_ID}},
        {"nodeId": "node_east", "sequenceId": 6, "released": True,
         "actions": [], "nodePosition": {"x": -6.0, "y": 3.0, "mapId": MAP_ID}},
    ],
    "edges": [
        {"edgeId": "e1", "sequenceId": 1, "released": True,
         "startNodeId": "node_table", "endNodeId": "node_row", "actions": []},
        {"edgeId": "e3", "sequenceId": 3, "released": True,
         "startNodeId": "node_row", "endNodeId": "node_west", "actions": []},
        {"edgeId": "e5", "sequenceId": 5, "released": True,
         "startNodeId": "node_west", "endNodeId": "node_east", "actions": []},
    ],
}


def make_mqtt(client_id):
    try:
        return mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id)
    except (AttributeError, TypeError):
        return mqtt.Client(client_id)


def on_state(client, userdata, msg):
    s = json.loads(msg.payload)
    print(
        f"[state ] lastNode={s.get('lastNodeId') or '-':<10} "
        f"nodesLeft={len(s.get('nodeStates') or [])} "
        f"driving={s.get('driving')}",
        flush=True,
    )


def main() -> int:
    client = make_mqtt("demo_master")
    client.on_message = on_state
    client.connect("localhost", 1883, 60)
    client.subscribe(STATE_TOPIC)
    client.loop_start()

    ORDER["timestamp"] = datetime.now(timezone.utc).strftime(
        "%Y-%m-%dT%H:%M:%S.000Z"
    )
    print(f"[master] publishing order -> {ORDER_TOPIC}", flush=True)
    client.publish(ORDER_TOPIC, json.dumps(ORDER))

    # Watch the state stream for a few seconds as the AGV traverses.
    time.sleep(5.0)
    client.loop_stop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
