# Client Adapter (Python)

This guide explains how to connect an existing robot stack to a VDA5050 master control
using the Python bindings of the client adapter provided by `vda5050_core.client`.
The client adapter acts as a high level bridge by managing VDA5050 communication over MQTT,
validation of incoming messages, order processing and state update.

It **does not** directly control the hardware - you provide the callbacks that forward
commands to your robot's lower-level drivers (ROS 2, vendor SDK, REST, etc.)

The Python API mirrors the C++ API described in [client-adapter.md](client-adapter.md),
so both guides share the same concepts. Method names are identical (`on_navigate`,
`on_action`, `on_localize`, `state_manager()`, ...).

You can find a [complete integration checklist](#complete-integration-checklist) at the end.

## Table of Contents

- [Client Adapter (Python)](#client-adapter-python)
  - [Table of Contents](#table-of-contents)
  - [1. Start from the Existing Example](#1-start-from-the-existing-example)
  - [2. Build and Run the Packaged Example](#2-build-and-run-the-packaged-example)
  - [3. Create Your Own Robot Integration](#3-create-your-own-robot-integration)
    - [3.1 Change MQTT Configuration and Robot Identity](#31-change-mqtt-configuration-and-robot-identity)
    - [3.2 Replace Simulated Navigation and Report Navigation Completion](#32-replace-simulated-navigation-and-report-navigation-completion)
    - [3.3 Replace Simulated Actions](#33-replace-simulated-actions)
    - [3.4 Connect Localization and Map Calibration](#34-connect-localization-and-map-calibration)
      - [Method A: Direct Initialization using `initialize_position`](#method-a-direct-initialization-using-initialize_position)
      - [Method B: Adding a Transformation into `StateManager`](#method-b-adding-a-transformation-into-statemanager)
    - [3.5 Replace Simulated State with Real Robot State](#35-replace-simulated-state-with-real-robot-state)
    - [3.6 Configure the Factsheet](#36-configure-the-factsheet)
    - [3.7 Adapter Lifecycle and Entrypoint](#37-adapter-lifecycle-and-entrypoint)
    - [3.8 Installing the Python Package](#38-installing-the-python-package)
  - [4. Build and Test Your Robot Integration](#4-build-and-test-your-robot-integration)
  - [Complete Integration Checklist](#complete-integration-checklist)

## 1. Start from the Existing Example

Use the following file as the starting template: `examples/python/adapter_example.py`

The example already handles:
- MQTT communication
- VDA5050 order processing
- adapter startup and shutdown
- navigation callbacks
- action callbacks
- localization callbacks
- robot state reporting

To integrate a real robot, copy the example into your robot integration package and replace the simulated behavior with the robot's control API.

> Calls such as `robot_driver.navigate_to()` in this guide are placeholders. They are not part of `vda5050_core`.

## 2. Build and Run the Packaged Example

Build the package with the Python bindings enabled (`BUILD_PYTHON` is `ON` by default):

```bash
colcon build \
  --packages-select vda5050_core \
  --cmake-args -DBUILD_PYTHON=ON
```

Start an MQTT broker:

```bash
mosquitto -d
```

Source the workspace:

```bash
source install/setup.bash
```

Run the packaged example:

```bash
python3 vda5050_core/examples/python/adapter_example.py
```

The example can be configured with the environment variables `MQTT_BROKER`,
`ROBOT_MQTT_CLIENT_ID`, `VDA5050_MANUFACTURER` and `VDA5050_SERIAL_NUMBER`.

Run this example first to confirm that the MQTT connection and client-adapter flow work before connecting a physical robot.

## 3. Create Your Own Robot Integration

After making sure the packaged example works, copy `adapter_example.py` into the directory of your choice and start working!

### 3.1 Change MQTT Configuration and Robot Identity

Set broker endpoints and align manufacturer strings with master control:

```python
from vda5050_core.client import Adapter, ProtocolAdapter
from vda5050_core.transport import create_default_client_shared

mqtt_client = create_default_client_shared(
    "tcp://192.168.1.10:1883",
    "robot_1_vda5050_adapter",
)

protocol_adapter = ProtocolAdapter.make(
    mqtt_client,
    "uagv",        # Interface domain
    "2.0.0",       # VDA5050 specification version
    "MyCompany",   # Manufacturer string
    "AGV-001",     # Serial number
)

adapter = Adapter.make(protocol_adapter)
state_manager = adapter.state_manager()
```

### 3.2 Replace Simulated Navigation and Report Navigation Completion

Forward navigation requests to local controllers. If a transformation for
`map_id` exists, the incoming position is automatically transformed
to local AGV coordinates:

```python
active_navigation = None


def on_navigate(node_request, edge_request, execution):
    global active_navigation

    position = node_request.node_position()

    if position is None:
        execution.failed("Requested node does not contain a position")
        return

    active_navigation = execution
    state_manager.set_driving(True)

    # Coordinates are automatically in local AGV frame if a transform exists!
    robot_driver.navigate_to(
        position.x,
        position.y,
        0.0 if position.theta is None else position.theta,
        position.map_id,
    )


adapter.on_navigate(on_navigate)

# In main loop or event listener
if active_navigation is not None and robot_driver.navigation_completed():
    state_manager.set_driving(False)
    active_navigation.finished()
    active_navigation = None

if active_navigation is not None and robot_driver.navigation_failed():
    state_manager.set_driving(False)
    active_navigation.failed(robot_driver.navigation_failure_reason())
    active_navigation = None
```

Every navigation request must end with either:

```python
execution.finished()
```

or:

```python
execution.failed("Failure reason")
```

Do not call `execution.finished()` immediately after sending the
navigation command, instead store the execution handle until the robot
reaches the destination or reports a failure.

The optional `edge_request` (`None` when there is no edge) may contain additional movement constraints,
such as speed (`max_speed()`) or trajectory (`trajectory()`) information. Use it only when required by the robot.

> **Do not block inside callbacks.** Callbacks are invoked by the adapter, so a
> long `time.sleep()` or a blocking driver call delays other message handling.
> Either store the execution handle and poll (as above), or hand the work to a
> thread and call `finished()` / `failed()` from there, as `adapter_example.py` does:
>
> ```python
> threading.Thread(target=drive, daemon=True).start()
> ```

The adapter example copies the requested node position into
`StateManager` because movement is simulated.
A real integration should report position using the robot's localization
or odometry data.

### 3.3 Replace Simulated Actions

Map VDA5050 action types to hardware commands:

```python
active_action = None


def on_action(request, execution):
    global active_action

    if request.action_type() == "customActionA":
        execution.running()
        active_action = execution

        # Replace with the robot action interface.
        robot_driver.start_custom_action_a()
        return

    execution.failed(f"Unsupported action: {request.action_type()}")


adapter.on_action(on_action)

# For long-running actions, keep the execution handle
# and report the result later
if active_action is not None and robot_driver.action_completed():
    active_action.finished()
    active_action = None

if active_action is not None and robot_driver.action_failed():
    active_action.failed(robot_driver.action_failure_reason())
    active_action = None
```

`ActionExecution` also provides `initializing()`, `paused(result_description=None)` and
`finished(result_description)` for reporting intermediate states and a result message.

### 3.4 Connect Localization and Map Calibration

Handle `on_localize` requests from master control to calibrate frame offsets.
There are two pathways to initialize position in `StateManager`:

#### Method A: Direct Initialization using `initialize_position`

Use when the local frame matches the VDA5050 world frame and no
transformation is required:

```python
# Sets position in StateManager and flags position_initialized = true
def on_localize(request, execution):
    accepted = robot_driver.set_initial_pose(
        request.x(), request.y(), request.theta(), request.map_id()
    )

    if not accepted:
        execution.failed("Robot rejected localization request")
        return

    agv_pose = robot_driver.get_local_pose()
    state_manager.initialize_position(
        agv_pose.x, agv_pose.y, agv_pose.theta, agv_pose.map_id
    )

    execution.finished("Localization calibrated and accepted")


adapter.on_localize(on_localize)
```

#### Method B: Adding a Transformation into `StateManager`

Use when local odometry/map coordinates differ from master world coordinates:

```python
from vda5050_core.client import Pose2D, Transformation


def on_localize(request, execution):
    execution.running()

    # Calibrate transformation matrix
    local = robot_driver.get_local_pose()
    agv_pose = Pose2D()
    agv_pose.x, agv_pose.y, agv_pose.theta = local.x, local.y, local.theta

    world_pose = Pose2D()
    world_pose.x, world_pose.y, world_pose.theta = (
        request.x(), request.y(), request.theta()
    )
    map_id = request.map_id()

    # Calibrate and store transformation in StateManager
    tf = Transformation.calibrate(world_pose, agv_pose)
    state_manager.set_transformation(tf, map_id)

    # Calling set_position converts local pose to world pose
    # AND automatically marks position_initialized = true
    state_manager.set_position(agv_pose.x, agv_pose.y, agv_pose.theta, map_id)

    execution.finished()


adapter.on_localize(on_localize)
```

### 3.5 Replace Simulated State with Real Robot State

Keep the state manager returned by `adapter.state_manager()` and continuously feed
local position and status metrics into it.

```python
from vda5050_core.types import BatteryState, OperatingMode

state_manager = adapter.state_manager()


def update_robot_state():
    # Position: Pass local AGV coordinates (x_agv, y_agv, theta_agv).
    # If a transform exists for map_id, StateManager converts local pose
    # back to world pose for MQTT state messages automatically.
    pose = robot_driver.current_local_pose()
    state_manager.set_position(pose.x, pose.y, pose.theta, pose.map_id)

    state_manager.set_driving(robot_driver.is_moving())
    state_manager.set_operating_mode(OperatingMode.AUTOMATIC)

    battery = BatteryState()
    battery.battery_charge = robot_driver.battery_percentage()
    battery.charging = robot_driver.is_charging()
    state_manager.set_battery_state(battery)
```

The integration may also update:

- velocity through `set_velocity(...)`
- paused state through `set_paused(...)`
- safety state through `set_safety_state(...)`
- distance since the last node through `set_distance_since_last_node(...)`
- errors through `add_error(...)`, `set_errors(...)`, `clear_errors()`
- information through `add_information(...)`, `set_information(...)`, `remove_information()`

Order-related fields such as the current order, node states, edge states, and last reached node are managed by the adapter.

> The exact set of `StateManager` methods exposed to Python is listed in the
> generated stubs at `python/vda5050_core/_core/client.pyi`.

### 3.6 Configure the Factsheet

Create a factsheet describing the real robot:

```python
from vda5050_core.types import Factsheet

factsheet = Factsheet()

# Populate the supported robot capabilities.
adapter.set_factsheet(factsheet)
```

The factsheet may include:

- supported actions
- robot dimensions
- load capabilities
- velocity limits
- acceleration limits
- supported protocol features

If a factsheet is not supplied during adapter initialization, a default
one will be created for you.

### 3.7 Adapter Lifecycle and Entrypoint

The startup and shutdown section of the example can remain mostly unchanged.
Make sure to register the callbacks first.

```python
import signal
import threading
import time

running = threading.Event()
running.set()


def signal_handler(signum, _frame):
    running.clear()


signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

# Adapter initialization
adapter.on_navigate(on_navigate)
adapter.on_action(on_action)
adapter.on_localize(on_localize)

# Start the adapter
adapter.start()

# Lifecycle Loop
try:
    while running.is_set():
        update_robot_state()
        update_navigation_status()
        time.sleep(0.1)
finally:
    # Stop the adapter
    adapter.stop()
```

Keep references to `adapter`, `protocol_adapter` and `mqtt_client` for as long as the adapter
is running so they are not garbage collected.

### 3.8 Installing the Python Package

The bindings are built with `scikit-build-core` and `pybind11` and require Python 3.10+.
Install the native dependencies first (`libpaho-mqtt-dev`, `libpaho-mqttpp-dev`,
`nlohmann-json3-dev`, `libfmt-dev`), then build the package with `colcon`:

```bash
colcon build --packages-select vda5050_core
source install/setup.bash
```

When using your own integration package in the same workspace, add the package dependency to `package.xml`:

```xml
<exec_depend>vda5050_core</exec_depend>
```

## 4. Build and Test Your Robot Integration

Build your workspace, source it, start a broker and run your integration:

```bash
colcon build --packages-select robot_vda5050_adapter
source install/setup.bash
mosquitto -d
python3 robot_vda5050_adapter.py
```

Then send an order from a master control (for example the one described in [master.md](master.md))
and verify that your navigation and action callbacks are invoked and that state messages are published.

## Complete Integration Checklist
Use this checklist to track your progress when replacing the template implementation with real robot integration code:

- [ ] 1. MQTT & Identity Configuration

    - Update MQTT broker URL and port (`tcp://<broker-ip>:1883`).

    - Set unique MQTT client ID.

    - Set `manufacturer` and `serial_number` strings to match master control expectations.

- [ ] 2. Navigation Dispatching and Completion

    - Hook `on_navigate` to robot drivers navigation function.

    - Verify local target coordinates from `node_request.node_position()`.

    - Report completion through `execution.finished()` or failure through `execution.failed(...)`.

    - Do not block inside the callback; use a stored handle or a worker thread.

- [ ] 3. Action Execution and Status Update

    - Map supported VDA 5050 action types in `on_action`.

    - Transition execution states properly (`running()`, `finished()`, `failed()`, etc.).

    - Reject unsupported action types explicitly.

- [ ] 4. Localization and Coordinate Transforms

    - Register `on_localize` handler to calibrate world-to-AGV frame transformation using `Transformation.calibrate()`.

    - Store transform in `StateManager` using `set_transformation()`.

    - Pass calibrated pose to local robot controller if required.

- [ ] 5. State Reporting

    - Stream local pose into `state_manager.set_position(...)` (automatically converted to world coordinates if transforms available).

    - Continuously update driving status (`set_driving`), operating mode (`set_operating_mode`), and battery metrics (`set_battery_state`).

- [ ] 6. Packaging

    - Build `vda5050_core` with `colcon` and declare it as a `<exec_depend>` in your integration's `package.xml`.
