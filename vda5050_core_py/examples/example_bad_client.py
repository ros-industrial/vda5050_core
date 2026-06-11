#!/usr/bin/env python3
"""Publish intentionally invalid VDA5050 2.1.0 state messages for schema testing.

Each iteration sends a valid idle state with one required top-level key removed.
Use with the Node-RED monitor flow (nodered-inject-compose); expect red
``state ✗ schema`` lines showing ``Missing: <field>``.

Stop testing_client.py first to avoid mixed valid/invalid traffic.

Usage::

    python3 vda5050_core_py/examples/example_bad_client.py
    python3 vda5050_core_py/examples/example_bad_client.py --once
    python3 vda5050_core_py/examples/example_bad_client.py --interval 3
"""

from __future__ import annotations

import argparse
import itertools
import json
import sys
import time
from datetime import datetime, timezone
from urllib.parse import urlparse

import paho.mqtt.client as mqtt

TOPIC = "uagv/v2/Manufacturer/S001/state"
CLIENT_ID = "bad_state_client"

REQUIRED_FIELDS = [
    "headerId",
    "timestamp",
    "version",
    "manufacturer",
    "serialNumber",
    "orderId",
    "orderUpdateId",
    "lastNodeId",
    "lastNodeSequenceId",
    "nodeStates",
    "edgeStates",
    "driving",
    "actionStates",
    "batteryState",
    "operatingMode",
    "errors",
    "safetyState",
]


def base_valid_state(header_id: int) -> dict:
    """Return a valid VDA5050 2.1.0 idle state (matches testing_client empty state)."""
    return {
        "headerId": header_id,
        "timestamp": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3]
        + "Z",
        "version": "2.1.0",
        "manufacturer": "Manufacturer",
        "serialNumber": "S001",
        "orderId": "",
        "orderUpdateId": 0,
        "lastNodeId": "",
        "lastNodeSequenceId": 0,
        "nodeStates": [],
        "edgeStates": [],
        "driving": False,
        "actionStates": [],
        "batteryState": {"batteryCharge": 0.0, "charging": False},
        "operatingMode": "AUTOMATIC",
        "errors": [],
        "safetyState": {"eStop": "NONE", "fieldViolation": False},
    }


def parse_broker(broker: str) -> tuple[str, int]:
    parsed = urlparse(broker if "://" in broker else f"tcp://{broker}")
    host = parsed.hostname or "localhost"
    port = parsed.port or 1883
    return host, port


def connect_client(broker: str) -> mqtt.Client:
    host, port = parse_broker(broker)
    client = mqtt.Client(client_id=CLIENT_ID, protocol=mqtt.MQTTv311)
    client.connect(host, port, keepalive=60)
    client.loop_start()
    return client


def publish_bad_state(
    client: mqtt.Client,
    omitted_field: str,
    header_id: int,
) -> None:
    payload = base_valid_state(header_id)
    del payload[omitted_field]
    body = json.dumps(payload, separators=(",", ":"))
    client.publish(TOPIC, body, qos=0, retain=False)
    print(f"[BAD] headerId={header_id} omitted {omitted_field}")


def run(broker: str, interval: float, once: bool) -> None:
    client = connect_client(broker)
    header_id = 1
    fields = REQUIRED_FIELDS if once else itertools.cycle(REQUIRED_FIELDS)

    try:
        for omitted_field in fields:
            publish_bad_state(client, omitted_field, header_id)
            header_id += 1
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        client.loop_stop()
        client.disconnect()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Publish VDA5050 state messages missing one required field each."
    )
    parser.add_argument(
        "--broker",
        default="tcp://localhost:1883",
        help="MQTT broker URI (default: tcp://localhost:1883)",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=2.0,
        help="Seconds between publishes (default: 2)",
    )
    parser.add_argument(
        "--once",
        action="store_true",
        help="Publish one message per required field, then exit",
    )
    args = parser.parse_args()

    if args.interval <= 0:
        print("error: --interval must be positive", file=sys.stderr)
        return 1

    run(args.broker, args.interval, args.once)
    return 0


if __name__ == "__main__":
    sys.exit(main())
