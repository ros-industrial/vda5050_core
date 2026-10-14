# VDA5050 Library and Support Tools

[![CI](https://github.com/ros-industrial/rmf_scheduler/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/ros-industrial/rmf_scheduler/actions/workflows/build.yml)

This repository contains the source code for the C++ and Python libraries for
implementing the [VDA5050 specification](https://github.com/VDA5050/VDA5050) across AGVs, AMRs and fleet control systems.

The library can be embedded directly into standalone native C++ drivers, ROS 2 packages, or Python-based systems.

> [!NOTE]
> This project is under active development. API stability is guaranteed across minor releases.

## Content

- [Features](#features)
- [Detailed guides](#detailed-guides)
- [Installation](#installation)
  - [Binary packages](#binary-packages)
  - [Build from source](#build-from-source)
- [Examples](#examples)
  - [AGV Client Integration](#agv-client-integration)
  - [Master Control Integration](#master-control-integration)
- [Project Layout](#project-layout)
- [Testing](#testing)
- [Contributing](#contributing)
- [License](#license)


## Features

- Support for VDA5050 v2.0.0
- Support for Layout Interchange Format (LIF)
- Core functionality
  - MQTT transport abstraction
  - Protocol specific message types
  - JSON (de)serialization
  - Specification validation
  - Asynchronous execution

- Client library
  - order processing
  - navigation callback
  - action callbacks
  - localization callbacks
  - robot state reporting
  - Open-RMF migration

- Master library
  - robot onboarding / offboarding
  - layout loading
  - order assignment
  - instant actions assignment
  - event-based callback handling

## Detailed Guides

| Guide                                                              | Description                                               |
| ------------------------------------------------------------------ | --------------------------------------------------------- |
| **[Client Adapter Guide](docs/client-adapter.md)**                 | Step-by-step integration guide for AGV/AMR                |
| **[Master Guide](docs/master.md)**                                 | Step-by-step guide to building a master control           |
| **[Master API Reference](docs/master-api.md)**                     | Master commands, types and callbacks                      |
| **[Types and Serialization Guide](docs/types.md)**                 | Message structures, validation rules and JSON conversion  |
| **[Validation Guide](docs/validation.md)**                         | Validator checks, required inputs and results             |
| **[Open-RMF Migration Guide](docs/rmf-migration.md)**              | Migrating an Open-RMF fleet adapter to a VDA5050 Adapter  |
| **[Architecture and Design](docs/design.md)**                      | Architecture and design rationale                         |

To connect an existing robot SDK, REST API or ROS 2 navigation system, start with the [Client Adapter Guide](docs/client-adapter.md).

To build a master control, or integrate one into an existing application, start with the [Master Guide](docs/master.md).

## Installation

### Binary packages

> [!NOTE]
> Binary packages are only available for Python right now

#### Install from PyPI

Supported Platforms
- **Python**: 3.10+
- **OS:** Linux, MacOS (15+)
- **Architecture:** x86_64, arm64

```shell
pip install vda5050_core
```

### Build from source

The following instructions are for Ubuntu 22.04+

1. Install the required build dependencies:

```bash
sudo apt update
sudo apt install \
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    libfmt-dev \
    nlohmann-json3-dev \
    pybind11-json-dev
```

1. Create a workspace and clone the repository:

```bash
mkdir -p ~/vda5050_ws/src
cd ~/vda5050_ws/src

git clone https://github.com/ros-industrial/vda5050_core.git
```

1. Build the package:

```bash
cd ~/vda5050_ws
colcon build
```

#### Build Options

Pass these flags through `colcon build --cmake-args -D<OPTION>=<VALUE>` or directly in CMake.

| Option           | Default | Effect                                                  |
| ---------------- | ------- | ------------------------------------------------------- |
| `ENABLE_ROS2`    | `OFF`   | Enables support for ROS 2 `vda5050_interfaces` messages |
| `BUILD_PYTHON`   | `ON`    | Builds the Python bindings                              |
| `BUILD_EXAMPLES` | `ON`    | Builds the examples                                     |
| `BUILD_TESTING`  | `ON`    | Builds the tests and configured linters                 |

### Examples

Sample applications can be found in the source repository at [vda5050_core/examples/](./vda5050_core/examples).

These can all be build along with the library by specifying the CMake flag: `-DBUILD_EXAMPLES=ON` when configuring the build.

You can launch a local MQTT broker to test them.

```shell
mosquitto -v -p 1883
```

Below are some quick examples

#### AGV Client Integration

The following example shows the basic setup for an AGV-side client.

It creates an MQTT transport and a VDA5050 client adapter, then registers a navigation callback.
In a real application, the callback should forward the request to the robot's navigation system.

**In C++**

```cpp
#include <iostream>

#include "vda5050_core/client/adapter/adapter.hpp"
#include "vda5050_core/execution/protocol_adapter.hpp"
#include "vda5050_core/transport/mqtt_client_interface.hpp"

using namespace vda5050_core;

int main()
{
  auto mqtt_client = transport::create_default_client_unique(
    "tcp://localhost:1883",
    "agv_1");

  auto protocol_adapter = execution::ProtocolAdapter::make(
    std::move(mqtt_client),
    "uagv",
    "2.0.0",
    "Manufacturer",
    "S001");

  auto adapter = client::adapter::Adapter::make(protocol_adapter);

  adapter->on_navigate(
    [](auto node_request, auto edge_request, auto execution)
    {
      // Forward the request to the robot navigation system.
      //
      // This demonstration reports completion immediately.
      // A real integration should only report completion after
      // the robot reaches the requested node.
      execution->finished();
    });

  adapter->start();

  // Keep processing orders until Enter is pressed.
  std::cin.get();

  adapter->stop();
  return 0;
}
```

Linking with CMake

```cmake
find_package(vda5050_core REQUIRED)

target_link_libraries(agv_application
  PRIVATE
    vda5050_core::client
    vda5050_core::transport
    vda5050_core::logger
)
```

**In Python**

```python
from vda5050_core.rmf_migration import (
    Adapter,
    FleetConfiguration,
    RobotCallbacks,
    RobotConfiguration,
    RobotState,
)

MAP_ID = "demo-map"


def main() -> None:
    adapter = Adapter.make()
    fleet_config = FleetConfiguration(
        fleet_name="demo",
        broker_uri="tcp://localhost:1883",
        client_id_prefix="agv_1",
    )
    fleet = adapter.add_vda5050_fleet(fleet_config)

    robot_config = RobotConfiguration(
        manufacturer="Manufacturer",
        serial_number="S001",
        interface_name="uagv",
        version="2.0.0",
    )
    initial_state = RobotState(MAP_ID, [0.0, 0.0, 0.0], 1.0)

    robot_handle = None

    def navigate(destination, execution) -> None:
        # Forward the request to the robot navigation system.
        #
        # This demonstration reports completion immediately.
        # A real integration should only report completion after
        # the robot reaches the requested node.
        execution.finished()

    def stop() -> None:
        pass

    def execute_action(action_type, action_id, execution) -> None:
        execution.finished()

    callbacks = RobotCallbacks(navigate, stop, execute_action)
    robot_handle = fleet.add_robot(
        "robot-1",
        initial_state,
        robot_config,
        callbacks,
    )

    adapter.start()

    # Keep processing orders until Enter is pressed.
    input()
    adapter.stop()


if __name__ == "__main__":
    main()
```

For a complete integration covering navigation, actions, localization, cancellation and state reporting,
see the [Client Adapter Guide](docs/client-adapter.md) and [Open-RMF Migration Guide](docs/rmf-migration.md).

#### Master Control Integration

The following example shows the basic setup for a master.

It creates an MQTT transport and a master, onboards one AGV, and assigns it a two-node order once
the AGV reports itself ready. In a real application, the completion callback assigns the next order.

This assumes an AGV that is already localized and reporting state. See the
[Master Guide](docs/master.md) for bringing an unlocalized vehicle up with an
`initPosition` instant action.

**In C++**

```cpp
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "vda5050_core/logger/logger.hpp"
#include "vda5050_core/master/master.hpp"
#include "vda5050_core/transport/mqtt_client_interface.hpp"

using namespace vda5050_core;

int main()
{
  auto mqtt_client = transport::create_default_client_shared(
    "tcp://localhost:1883",
    "master_1");

  auto master = master::VDA5050Master::make(mqtt_client);

  master->on_order_complete(
    [](const std::string& agv_id, const std::string& order_id)
    {
      VDA5050_INFO("[{}] completed order [{}]", agv_id, order_id);

      // A real integration would assign this AGV's next order here, with a
      // new order id, or return the AGV to its task queue.
    });

  master->connect();
  master->onboard_agv("uagv", "Manufacturer", "S001");

  // Wait until the AGV is online, localized and idle.
  auto agv = master->get_agv("Manufacturer", "S001");
  while (agv->get_operational_state() != master::AGVState::AVAILABLE)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  // A simple order: drive from node N0 to node N1.
  // Nodes take even sequence ids, the edges between them the odd ones.
  types::Order order;
  order.order_id = "order-1";
  order.order_update_id = 0;
  order.nodes = {{"N0", 0, true}, {"N1", 2, true}};
  order.edges = {{"E0", 1, "N0", "N1", true}};

  auto result = master->assign_order("Manufacturer", "S001", order);

  if (result.decision != master::OrderAssignmentDecision::ASSIGNED)
  {
    // A real integration should read result.decision and result.errors to
    // decide whether to retry, hand the task to another AGV, or raise it to
    // an operator.
    VDA5050_WARN(
      "Order [{}] not assigned ({} error(s))", order.order_id,
      result.errors.size());
  }

  // Keep the master running until Enter is pressed.
  std::cin.get();

  master->disconnect();
  return 0;
}
```

Linking with CMake

```cmake
find_package(vda5050_core REQUIRED)

target_link_libraries(master_application
  PRIVATE
    vda5050_core::master
    vda5050_core::transport
    vda5050_core::logger
)
```

**In Python**

```python
from logging import getLogger
from time import sleep

from vda5050_core.master import VDA5050Master, AGVState, OrderAssignmentDecision
from vda5050_core.transport import create_default_client_shared
from vda5050_core.types import Order, Node, Edge

LOGGER = getLogger(__name__)


def main() -> None:
    mqtt_client = create_default_client_shared("tcp://localhost:1883", "master_1")
    master = VDA5050Master.make(mqtt_client)

    def on_order_complete(agv_id, order_id):
        LOGGER.info(f"[{agv_id} completed order {order_id}]")

        # A real integration would assign this AGV's next order here, with a
        # new order id, or return the AGV to its task queue.


    master.on_order_complete(on_order_complete)

    master.connect()
    master.onboard_agv("Manufacturer", "S001")

    # Wait until the AGV is online, localized and idle.
    agv = master.get_agv("Manufacturer", "S001")
    while agv.get_operational_state() != AGVState.AVAILABLE:
        sleep(0.2)

    # A simple order: drive from node N0 to node N1.
    # Nodes take even sequence ids, the edges between them the odd ones.
    order = Order()
    order.order_id = "order-1"
    order.order_update_id = 0
        position = state.agv_position
        pose = None if position is None else (position.x, position
    order.nodes = [
            Node.from_json({"nodeId": "N0", "sequenceId": 0, "released": True, "actions": []}),
            Node.from_json({"nodeId": "N1", "sequenceId": 2, "released": True, "actions": []}),
    ]

    order.edges = [
        Edge.from_json({
            "edgeId": "E0",
            "sequenceId": 1,
            "startNodeId": "N0",
            "endNodeId": "N1",
            "released": True,
            "actions": []
        })
    ]

    result = master.assign_order("Manufacturer", "S001", order)

    if result.decision != OrderAssignmentDecision.ASSIGNED:
        # A real integration should read result.decision and result.errors to
        # decide whether to retry, hand the task to another AGV, or raise it to
        # an operator.
        LOGGER.warning(f"Order [{order.order_id}] not assigned ({len(result.errors)})")

    # Keep the master running until Enter is pressed.
    input()

    master.disconnect()

if __name__ == "__main__":
    main()
```

For a complete integration covering order construction, validation, event handling and multi-AGV
dispatch, see the [Master Guide](docs/master.md).


## Project Layout

```
vda5050_core
└── vda5050_core
    ├── docs                 # Guides and architectural documentation
    ├── examples             # Ready-to-run executables
    ├── include
    │   └── vda5050_core
    │       ├── client       # High-level AGV client adapter
    │       ├── errors       # Error definitions
    │       ├── execution    # Reactive execution framework
    │       ├── json_utils   # JSON serialization and traits
    │       ├── layout       # Layout Interchange Format (LIF) support and tools
    │       ├── logger       # Logging utilities
    │       ├── master       # Master control components
    │       ├── transport    # MQTT client interface and default implementation
    │       ├── types        # VDA5050 message structs
    │       └── validation   # VDA5050 specification compliance checks
    ├── python               # pybind11 modules and migration tools
    └── test                 # Unit and integration tests
```

## Testing

Run the unit and integration tests using `colcon`.

```bash
colcon test --event-handlers console_direct+ --packages-select vda5050_core
```

> [!NOTE]
> Some integration tests require an active MQTT broker listening on `localhost:1883`.

## Contributing

Contributions are welcome!

See [CONTRIBUTING.md](CONTRIBUTING.md) for development and contribution guidelines.

Commits must include a `Signed-off-by` line certifying the [Developer Certificate of Origin](https://developercertificate.org/).

## License

Licensed under the Apache License 2.0. See [LICENSE](LICENSE) for details.
