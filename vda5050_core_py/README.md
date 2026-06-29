# vda5050_core_py

Python bindings for the [`vda5050_core`](../vda5050_core) C++ Adapter (AGV side).

The C++ adapter owns the MQTT lifecycle, order graph execution, header IDs, and
1Hz state publishing. Python implements only the per-node `on_navigate`
callback and reports state back through `NavigationManager`.

## Build

This is a ROS 2 `ament_cmake` package. Build it with colcon from the workspace
root.

```bash
# 1. Source ROS 2 (Humble or later)
source /opt/ros/humble/setup.bash

# 2. Build vda5050_core and vda5050_core_py
cd ~/vda5050
colcon build --packages-up-to vda5050_core_py

# 3. Source the workspace
source install/setup.bash

# 4. Verify the import
python3 -c "import vda5050_core_py; print(vda5050_core_py.Adapter)"
```

## Usage

```python
import vda5050_core_py as vda

mqtt = vda.create_default_mqtt_client("tcp://localhost:1883", "my_agv")
protocol = vda.ProtocolAdapter.make(mqtt, "uagv", "2.0.0", "Manufacturer", "S001")
adapter = vda.Adapter.make(protocol)
nav = adapter.navigation_manager()


def on_navigate(node):
    print(f"Navigate to {node.node_id} at "
          f"({node.node_position.x}, {node.node_position.y})")
    nav.set_driving(True)

    # ... drive the robot to node.node_position (e.g. via REST) ...

    nav.set_driving(False)
    nav.node_reached(node)


adapter.on_navigate(on_navigate)
adapter.start()
vda.run_until_signal(adapter)  # see Job control below
```

### Job control (`run_until_signal`)

On Unix terminals:

| Input | Effect |
|-------|--------|
| **Ctrl+C** | Graceful exit: `adapter.stop()`, process ends |
| **Ctrl+\\** | Same as Ctrl+C (`SIGQUIT`) |
| **SIGTERM** | Same graceful exit |
| **Ctrl+Z** | `adapter.stop()` (MQTT **OFFLINE**), process suspended; stderr prints `fg` / `bg` hints |
| **`fg`** or **`bg` then `fg`** | `adapter.start()` again (MQTT **ONLINE**); loop keeps running |

On Windows, only Ctrl+C and SIGTERM trigger exit; Ctrl+Z job control is not available.

In-flight `on_navigate` worker threads are not stopped on Ctrl+Z; only the adapter/MQTT
spin loop disconnects.

Alternatively, handle shutdown manually (same pattern as
`vda5050_core/examples/adapter_example.cpp`):

```python
import signal
import time

running = True

def on_shutdown(signum, frame):
    global running
    running = False

signal.signal(signal.SIGINT, on_shutdown)
signal.signal(signal.SIGTERM, on_shutdown)

adapter.start()
try:
    while running:
        time.sleep(0.1)
finally:
    adapter.stop()
```

## Threading

`on_navigate` runs on the C++ spin thread. **Long-running work** (HTTP REST,
polling, sleeps) should be dispatched to a Python worker thread so the spin
loop is not blocked:

```python
import threading

def on_navigate(node):
    threading.Thread(target=drive_robot, args=(node,), daemon=True).start()

def drive_robot(node):
    # ... blocking REST / polling ...
    nav.node_reached(node)
```

The C++ side continues publishing the 1Hz State heartbeat and processing
incoming Orders regardless of how long the Python callback takes.

## Tests

The smoke tests run automatically when `BUILD_TESTING=ON` (the colcon default):

```bash
cd ~/vda5050
colcon test --packages-select vda5050_core_py
colcon test-result --verbose
```

Manual invocation (after sourcing the workspace):

```bash
python3 -m pytest /home/ubuntu/vda5050/vda5050_core_gametl/vda5050_core_py/test
```

## API surface (v1)

- `Adapter.make(protocol_adapter)`, `on_navigate(cb)`, `navigation_manager()`, `start()`, `stop()`
- `run_until_signal(adapter)` — block until SIGINT/SIGTERM/SIGQUIT (Unix), or pause/resume on Ctrl+Z / `fg` (Unix); then call `stop()` on exit
- `NavigationManager.node_reached(node)`, `set_driving(bool)`, `set_agv_position(AGVPosition)`
- `ProtocolAdapter.make(mqtt_client, interface, version, manufacturer, serial_number)`
- `create_default_mqtt_client(broker_address, client_id)`
- Types: `Node`, `NodePosition`, `AGVPosition`, `Action`, `ActionParameter`, `BlockingType`

Not yet bound (add as needed): `set_battery_state`, `set_operating_mode`,
`add_error`, `clear_errors`.
