# VDA5050 - FiWare integration

## Fiware Datamodel

The datamodel that will be used for The Context Broker will be identical to the smart-data-models schema for [Autonomous Mobile Robot](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json) This document aims to map the information available by the [VDA5050 data models](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas) to the Smart Data Model Schema. Note that not all mappings are currently provided by the VDA5050 schemas directly

## Usage of the Data Model with the Orion Context Broker.

Currently the integration will be done in alignment with the [iotagent-node-lib](https://github.com/telefonicaid/iotagent-node-lib/blob/master/doc/api.md).

<!-- ### Local Demonstration

Included in this repository is a sample `docker-compose` file that spins up the Orion Context Broker and the IOT Agent -->

## Fiware Data Model Definition

| [Fiware Autonomous Mobile Robot Schema](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json)| Supported | [VDA5050 Mappings](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas)|
| :---------------------------------------------------- | :--| ----------------------------- |
| `command`                                             | Y | Order, or instant action message as string                            |
| `commandTime`                                         | Y | `order.timestamp` OR `instantActions.timestamp`                       | 
| `errors`                                              | Y | `state.errors`                                                        |
| `stopCommand`                                         |   |                                                                       |
| `resultsOfStopCommand`                                |   |                                                                       |
| `result`                                              |   |                                                                       |
| `receivedTime`                                        |   |                                                                       |
| `mode`                                                |   |                                                                       |
| `accuracy`                                            |   |                                                                       |
| `battery          `                                   | Y | [Battery Definition](#battery-defintion)                              |
| `destination    `                                     |   | [Destination Definition](#destination-node-defintion)                 |
| `pose                 `                               |   | [Pose Definition](#agv-pose-defintion)                                |
| `receivedWaypoints`                                   | Y | [Received Waypoints Definition](#received-waypoint-array-defintion)   |
| `waypoints`                                           | Y | [Waypoints Definition](#agv-waypoints-array-defintion)                |

### Battery Defintion

The following describes the structure of the battery definition, that should be expressed in a `dump`-ed json string

| [Fiware Autonomous Mobile Robot Schema](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json)| Supported | [VDA5050 Mappings](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas)|
| :---------------------------------------------------- | :--| ------------------------------------------------ |
| `battery.voltage`                                     | Y | `batteryState.batteryVoltage`                     |
| `battery.current`                                     |   |                                                   |
| `battery.remainingTime`                               |   |                                                   |
| `battery.remainingPercentage`                         | Y | `batteryState.batteryHealth`                      |

### Destination Node Defintion

The following describes the structure of the destination node definition, that should be expressed in a `dump`-ed json string

| [Fiware Autonomous Mobile Robot Schema](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json)| Supported | [VDA5050 Mappings](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas)|
| :---------------------------------------------------- | :--| ------------------------------------------------ |
| `destination.mapId`                                   | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.mapID` |
| `destination.speed`                                   |   |                                                   |
| `destination.orientation2D.theta`                     | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.theta` |
| `destination.orientation3D.roll`                      |   |                                                   |
| `destination.orientation3D.pitch`                     |   |                                                   |
| `destination.orientation3D.yaw`                       | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.theta` |
| `destination.point2D.x`                               | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.x`     |
| `destination.point2D.y`                               | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.y`     |
| `destination.point3D.x`                               | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.x`     |
| `destination.point3D.y`                               | Y | `order.nodes[LAST_NODE_INDEX].nodePosition.y`     |
| `destination.point3D.z`                               |   |                                                   |
| `destination.geographicPoint.latitude`                |   |                                                   |
| `destination.geographicPoint.longitude`               |   |                                                   |
| `destination.geographicPoint.altitude`                |   |                                                   |

### AGV Pose Defintion

The following describes the structure of the AGV pose definition, that should be expressed in a `dump`-ed json string

| [Fiware Autonomous Mobile Robot Schema](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json)| Supported | [VDA5050 Mappings](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas)|
| :---------------------------------------------------- | :--| ------------------------------------------------ |
| `pose.mapId`                                          | Y | `state.agvPosition.mapId`                         |
| `pose.orientation2D.theta`                            | Y | `state.agvPosition.theta`                         |
| `pose.orientation3D.roll`                             |   |                                                   |
| `pose.orientation3D.pitch`                            |   |                                                   |
| `pose.orientation3D.yaw`                              | Y | `state.agvPosition.theta`                         |
| `pose.point2D.x`                                      | Y | `state.agvPosition.x`                             |
| `pose.point2D.y`                                      | Y | `state.agvPosition.y`                             |
| `pose.point3D.x`                                      | Y | `state.agvPosition.x`                             |
| `pose.point3D.y`                                      | Y | `state.agvPosition.y`                             |
| `pose.point3D.z`                                      |   |                                                   |
| `pose.geographicPoint.latitude`                       |   |                                                   |
| `pose.geographicPoint.longitude`                      |   |                                                   |
| `pose.geographicPoint.altitude`                       |   |                                                   |

### Received Waypoint Array Defintion

The following describes the structure of the received waypoint array definition. From a VDA5050 Context, what will be provided here would be the `nodes` component of the VDA5050 `Order`, that should be expressed in a `dump`-ed json array string

| [Fiware Autonomous Mobile Robot Schema](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json)| Supported | [VDA5050 Mappings](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas)|
| :---------------------------------------------------- | :--| ------------------------------------------------ |
| `receivedWaypoints[INDEX].mapId`                      | Y | `order.nodes[INDEX].nodePosition.mapId`           |
| `receivedWaypoints[INDEX].speed`                      |   |                                                   |
| `receivedWaypoints[INDEX].orientation2D.theta`        | Y | `order.nodes[INDEX].nodePosition.theta`           |
| `receivedWaypoints[INDEX].orientation3D.roll`         |   |                                                   |
| `receivedWaypoints[INDEX].orientation3D.pitch`        |   |                                                   |
| `receivedWaypoints[INDEX].orientation3D.yaw`          | Y | `order.nodes[INDEX].nodePosition.theta`           |
| `receivedWaypoints[INDEX].point2D.x`                  | Y | `order.nodes[INDEX].nodePosition.x`               |
| `receivedWaypoints[INDEX].point2D.y`                  | Y | `order.nodes[INDEX].nodePosition.y`               |
| `receivedWaypoints[INDEX].point3D.x`                  | Y | `order.nodes[INDEX].nodePosition.x`               |
| `receivedWaypoints[INDEX].point3D.y`                  | Y | `order.nodes[INDEX].nodePosition.y`               |
| `receivedWaypoints[INDEX].point3D.z`                  |   |                                                   |
| `receivedWaypoints[INDEX].geographicPoint.latitude`   |   |                                                   |
| `receivedWaypoints[INDEX].geographicPoint.longitude`  |   |                                                   |
| `receivedWaypoints[INDEX].geographicPoint.altitude`   |   |                                                   |

### AGV Waypoints Array Defintion

The following describes the structure of the waypoint array definition. From a VDA5050 Context, what will be provided here would be the `nodeStates` component of the VDA5050 AGV `State`, that should be expressed in a `dump`-ed json array string

| [Fiware Autonomous Mobile Robot Schema](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot/blob/master/AutonomousMobileRobot-schema.json)| Supported | [VDA5050 Mappings](https://github.com/VDA5050/VDA5050/tree/2.0.0/json_schemas)|
| :---------------------------------------------------- | :--| ------------------------------------------------ |
| `waypoints[INDEX].mapId`                              | Y | `state.nodeStates[INDEX].nodePosition.mapId`      |
| `waypoints[INDEX].speed`                              |   |                                                   |
| `waypoints[INDEX].orientation2D.theta`                | Y | `state.nodeStates[INDEX].nodePosition.theta`      |
| `waypoints[INDEX].orientation3D.roll`                 |   |                                                   |
| `waypoints[INDEX].orientation3D.pitch`                |   |                                                   |
| `waypoints[INDEX].orientation3D.yaw`                  | Y | `state.nodeStates[INDEX].nodePosition.theta`      |
| `waypoints[INDEX].point2D.x`                          | Y | `state.nodeStates[INDEX].nodePosition.x`          |
| `waypoints[INDEX].point2D.y`                          | Y | `state.nodeStates[INDEX].nodePosition.y`          |
| `waypoints[INDEX].point3D.x`                          | Y | `state.nodeStates[INDEX].nodePosition.x`          |
| `waypoints[INDEX].point3D.y`                          | Y | `state.nodeStates[INDEX].nodePosition.y`          |
| `waypoints[INDEX].point3D.z`                          |   |                                                   |
| `waypoints[INDEX].geographicPoint.latitude`           |   |                                                   |
| `waypoints[INDEX].geographicPoint.longitude`          |   |                                                   |
| `waypoints[INDEX].geographicPoint.altitude`           |   |                                                   |