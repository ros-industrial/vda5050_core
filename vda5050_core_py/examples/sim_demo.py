#!/usr/bin/env python3
"""One-shot, no-robot end-to-end demo of the VDA5050 MQTT flow.

Runs entirely against a local MQTT broker (mosquitto on localhost:1883), with a
*stub* robot (instant arrival) instead of a real AGV:

    publish Order  ->  ProtocolAdapter (MQTT in)  ->  C++ Context+Strategies
      ->  on_navigate (stub: snap position + report reached)
      ->  StateReporting  ->  State published back on MQTT (out)

No Autoxing, no robot, no spellbook. To drive a real robot, replace the stub
on_navigate body with real drive calls (see autoxing-l300/example_autoxing_client.py).

Usage:
    mosquitto -p 1883 &          # any local broker
    python3 sim_demo.py
"""
from __future__ import annotations

import json
import time
from datetime import datetime, timezone

import paho.mqtt.client as mqtt

import vda5050_core_py as vda

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
    "orderId": "sim_demo",
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
    # Compatible with paho-mqtt 1.x and 2.x.
    try:
        return mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id)
    except (AttributeError, TypeError):
        return mqtt.Client(client_id)


def main() -> int:
    # --- AGV side: real runtime + stub robot -------------------------------
    runtime = vda.RobotRuntime(
        broker="tcp://localhost:1883",
        client_id="sim_agv",
        manufacturer=MANUF,
        serial_number=SERIAL,
        interface=INTERFACE,
        version=VERSION,
    )
    rep = runtime.reporter()

    visited = []

    def on_navigate(node, edge):
        p = node.node_position
        print(f"[robot ] drive -> {node.node_id} @ ({p.x:.1f}, {p.y:.1f})", flush=True)
        pos = vda.AGVPosition()
        pos.x, pos.y, pos.map_id = p.x, p.y, p.map_id
        pos.position_initialized = True
        rep.set_agv_position(pos)
        visited.append(node.node_id)
        rep.node_reached(node.node_id, node.sequence_id)  # stub: instant arrival

    runtime.on_navigate(on_navigate)
    runtime.start()

    # --- Master side: collect State, then publish the Order ----------------
    states = []
    sub = make_mqtt("sim_master_sub")
    sub.on_message = lambda c, u, m: states.append(json.loads(m.payload))
    sub.connect("localhost", 1883, 60)
    sub.subscribe(STATE_TOPIC)
    sub.loop_start()

    pub = make_mqtt("sim_master_pub")
    pub.connect("localhost", 1883, 60)
    pub.loop_start()

    time.sleep(0.5)
    ORDER["timestamp"] = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.000Z")
    print(f"[master] publish order -> {ORDER_TOPIC}", flush=True)
    pub.publish(ORDER_TOPIC, json.dumps(ORDER))

    # No manual start kick: the adapter auto-reports node[0] reached on order
    # acceptance, so traversal begins navigating on its own.
    time.sleep(2.0)
    runtime.stop()
    sub.loop_stop()
    pub.loop_stop()

    # --- Report ------------------------------------------------------------
    print("\n=== RESULT ===")
    print("on_navigate dispatched :", visited)
    print("State messages received:", len(states))
    if states:
        last = states[-1]
        print("final lastNodeId       :", last.get("lastNodeId"))
        print("final nodeStates left  :", len(last.get("nodeStates") or []))
        ok = (
            visited == ["node_row", "node_west", "node_east"]
            and last.get("lastNodeId") == "node_east"
            and len(last.get("nodeStates") or []) == 0
        )
        print("\nFULL MQTT LOOP:", "PASS ✅" if ok else "INCOMPLETE ❌")
        return 0 if ok else 1
    print("\nFULL MQTT LOOP: NO STATE RECEIVED ❌")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
