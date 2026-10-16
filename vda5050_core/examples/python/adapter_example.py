# Copyright (C) 2026 ROS-Industrial Consortium Asia Pacific
# Advanced Remanufacturing and Technology Centre
# A*STAR Research Entities (Co. Registration No. 199702110H)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Run a simulated VDA5050 robot adapter (Python port of adapter_example.cpp)."""

from __future__ import annotations

import logging
import os
import signal
import threading
import time

from vda5050_core.client import Adapter, ProtocolAdapter
from vda5050_core.transport import create_default_client_shared

BROKER_URI = os.environ.get("MQTT_BROKER", "tcp://localhost:1883")
MQTT_CLIENT_ID = os.environ.get("ROBOT_MQTT_CLIENT_ID", "adapter_example")
INTERFACE_NAME = "uagv"
VERSION = "2.0.0"
MANUFACTURER = os.environ.get("VDA5050_MANUFACTURER", "Manufacturer")
SERIAL_NUMBER = os.environ.get("VDA5050_SERIAL_NUMBER", "S001")
LOGGER = logging.getLogger(__name__)

running = threading.Event()
running.set()


def signal_handler(signum, _frame) -> None:
    LOGGER.info("System Signal [%d] received. Shutting down ...", signum)
    running.clear()


def main() -> None:
    logging.basicConfig(
        level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s"
    )
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    mqtt_client = create_default_client_shared(BROKER_URI, MQTT_CLIENT_ID)
    protocol_adapter = ProtocolAdapter.make(
        mqtt_client, INTERFACE_NAME, VERSION, MANUFACTURER, SERIAL_NUMBER
    )
    adapter = Adapter.make(protocol_adapter)
    state_manager = adapter.state_manager()

    def on_navigate(node_request, _edge_request, execution) -> None:
        LOGGER.info("Navigating to node [%s]", node_request.node_id())

        def drive() -> None:
            state_manager.set_driving(True)
            time.sleep(2)
            state_manager.set_driving(False)

            execution.finished()
            LOGGER.info("Reached node [%s]", node_request.node_id())

            position = node_request.node_position()
            if position is not None:
                state_manager.set_position(
                    position.x,
                    position.y,
                    0.0 if position.theta is None else position.theta,
                    position.map_id,
                )

        threading.Thread(target=drive, daemon=True).start()

    def on_action(request, execution) -> None:
        LOGGER.info("Action of type [%s] requested", request.action_type())

        execution.running()
        time.sleep(1)
        execution.finished()

    def on_localize(request, execution) -> None:
        LOGGER.info(
            "Received localization request with point [%s, %s, %s] on map: %s",
            request.x(),
            request.y(),
            request.theta(),
            request.map_id(),
        )

        state_manager.initialize_position(
            request.x(), request.y(), request.theta(), request.map_id()
        )
        execution.finished()

    adapter.on_navigate(on_navigate)
    adapter.on_action(on_action)
    adapter.on_localize(on_localize)

    adapter.start()
    LOGGER.info("Adapter started ...")

    try:
        while running.is_set():
            time.sleep(1)
    finally:
        adapter.stop()


if __name__ == "__main__":
    main()
