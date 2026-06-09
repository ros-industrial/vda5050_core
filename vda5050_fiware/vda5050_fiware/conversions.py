import sys
import json
import random
import math

from datetime import datetime
from vda5050_fiware.order import OrderMessage, Node, Edge, NodePosition
from vda5050_fiware.state import State, Map
from rmf2_agv.CommandMessage import CommandMessage
from rmf2_agv.StateMessage import (
    StateMessage,
    Destination,
    Pose,
    Velocity,
    Battery,
)
from rmf2_agv.CommonClasses import Orientation2D, Point2D, Waypoint
from fiware_api.utils import (
    printRed,
    printGreen,
    printYellow,
    printPurple,
    printCyan,
    MQTTClient,
)

from rmf2_building.Map import Map as BuildingMap

from .master import Vda5050Agv, Vda5050Map, Vda5050Master, Edges, Nodes


def convert_command_message_to_vda_order(
    manufacturer: str, serial_number: str, input: CommandMessage, vda_map: Vda5050Map
) -> OrderMessage:
    nodes = []
    edges = []
    node_before = None
    counter = 0

    for waypoint in input.waypoints:
        # Catch any repeated nodes
        if node_before:
            if node_before.nodeId == waypoint.nodeId:
                continue
            edge = vda_map.get_edge(node_before.nodeId, waypoint.nodeId)
            if edge is None:
                printRed(
                    "Error, no edge found connecting node "
                    + node_before.nodeId
                    + " and "
                    + waypoint.nodeId
                )
                continue
            edge.released = node_before.released and waypoint.released
            edge.sequenceId = counter
            counter = counter + 1
            edges.append(edge)
        node = vda_map.get_node(waypoint.nodeId)
        node.released = waypoint.released
        if node is None:
            printRed("Error, no node " + waypoint.nodeId)
            continue
        node.sequenceId = counter
        counter = counter + 1
        node_before = node
        nodes.append(node)

    order_output = OrderMessage(
        headerId=input.commandUpdateId,
        timestamp=datetime.now().isoformat() + 'Z',
        version="2.0.0",
        manufacturer=manufacturer,
        serialNumber=serial_number,
        orderId=input.commandId,
        orderUpdateId=input.commandUpdateId,
        nodes=nodes,
        edges=edges,
    )
    return order_output


def convert_command_message_json_to_vda_order(
    input_msg: json, vda_map: Vda5050Map
) -> OrderMessage:
    manufacturer = ""
    serial_number = ""
    for entry in input_msg["body"]["data"]:
        manufacturer = entry["id"].split(":")[-2]
        serial_number = entry["id"].split(":")[-1]
    return convert_command_message_to_vda_order(
        manufacturer=manufacturer,
        serial_number=serial_number,
        input=CommandMessage.from_subscriber_json(
            input_msg, "ngsi-ld:default-context/"
        ),
        vda_map=vda_map,
    )


def convert_vda_state_to_state_message(
    input_msg: State, map: Vda5050Map
) -> StateMessage:
    waypoints = []
    mapId = ""
    for node_state in input_msg.nodeStates:
        waypoint = Waypoint(
            nodeId=node_state.nodeId,
            point2D=Point2D(x=0.0, y=0.0),
            released=node_state.released,
            sequenceId=int(node_state.sequenceId / 2),
            orientation2D=Orientation2D(theta=0.0),
        )

        if node_state.nodePosition:
            waypoint.point2D.x = node_state.nodePosition.x
            waypoint.point2D.y = node_state.nodePosition.y
            if node_state.nodePosition.theta:
                waypoint.orientation2D.theta = node_state.nodePosition.theta
            mapId = node_state.nodePosition.mapId
        else:
            node = map.get_node(nodeid=node_state.nodeId)
            waypoint.point2D.x = node.nodePosition.x
            waypoint.point2D.y = node.nodePosition.y
        waypoints.append(waypoint)

    pose = Pose(
        mapId=mapId,
        orientation2D=Orientation2D(theta=0.0),
        point2D=Point2D(x=0.0, y=0.0),
        lastNodeSequenceId=int(input_msg.lastNodeSequenceId / 2),
    )

    destination = Destination(
        mapId="",
        nodeId="",
        orientation2D=pose.orientation2D,
        point2D=pose.point2D,
        released=True,
        sequenceId=pose.lastNodeSequenceId,
    )
    status = "IDLE"

    # agvPosition is not a required field.
    if input_msg.agvPosition:
        pose = Pose(
            mapId=input_msg.agvPosition.mapId,
            orientation2D=Orientation2D(theta=input_msg.agvPosition.theta),
            point2D=Point2D(x=input_msg.agvPosition.x, y=input_msg.agvPosition.y),
            lastNodeSequenceId=int(input_msg.lastNodeSequenceId / 2),
        )
    elif len(waypoints) > 0:
        pose = Pose(
            mapId=mapId,
            orientation2D=Orientation2D(theta=0.0),
            point2D=Point2D(
                x=waypoints[0].point2D.x,
                y=waypoints[0].point2D.y,
            ),
            lastNodeSequenceId=int(waypoints[0].sequenceId / 2),
        )

    if len(waypoints) > 0:
        destination = Destination(
            mapId=pose.mapId,
            nodeId=waypoints[-1].nodeId,
            orientation2D=waypoints[-1].orientation2D,
            point2D=waypoints[-1].point2D,
            released=waypoints[-1].released,
            sequenceId=waypoints[-1].sequenceId,
        )
        status = "ACTIVE"
    else:
        destination = Destination(
            mapId=pose.mapId,
            nodeId="",
            orientation2D=pose.orientation2D,
            point2D=pose.point2D,
            released=True,
            sequenceId=pose.lastNodeSequenceId,
        )

    battery = Battery(remainingPercentage=random.randint(0, 100))
    if input_msg.batteryState.batteryHealth:
        battery.remainingPercentage = input_msg.batteryState.batteryHealth

    velocity = Velocity(vx=1.0, vy=1.0, omega=1.0)

    # Provide a random value for now
    if status == "IDLE":
        velocity = Velocity(vx=0.0, vy=0.0, omega=0.0)

    if input_msg.velocity:
        if input_msg.velocity.vx:
            velocity.vx = input_msg.velocity.vx
        if input_msg.velocityy:
            velocity.vy = input_msg.velocity.vy
        if input_msg.velocity.omega:
            velocity.omega = input_msg.velocity.omega

    return StateMessage(
        type="StateMessage",
        battery=battery,
        destination=destination,
        pose=pose,
        velocity=velocity,
        waypoints=waypoints,
        status=status,
        orderId=input_msg.orderId,  # CRITICAL: Pass orderId from VDA5050 State for command validation
    )


def convert_json_to_vda_map(map_input: BuildingMap):
    vda_nodes = Nodes()
    vda_edges = Edges()
    vda_edge_mapping = {}

    for edge in map_input.edges:
        if len(edge.connectedNodes) != 2:
            printYellow("Warning, edge not connected to 2 nodes")
            continue
        node_1 = edge.connectedNodes[0].nodeID
        node_2 = edge.connectedNodes[1].nodeID
        if node_1 in vda_edge_mapping:
            vda_edge_mapping[node_1][node_2] = edge.edgeId
        else:
            vda_edge_mapping[node_1] = {node_2: edge.edgeId}

        if node_2 in vda_edge_mapping:
            vda_edge_mapping[node_2][node_1] = edge.edgeId
        else:
            vda_edge_mapping[node_2] = {node_1: edge.edgeId}

        # vda_edge_mapping[node_2][node_1] = edge_name
        vda_edges[edge.edgeId] = Edge(
            edgeId=edge.edgeId,
            sequenceId=0,
            released=False,
            startNodeId=node_1,
            endNodeId=node_2,
            actions=[],
        )

    for node in map_input.nodes:
        vda_nodes[node.nodeId] = Node(
            nodeId=node.nodeId,
            sequenceId=0,
            released=False,
            nodePosition=NodePosition(
                x=node.point2D.x,
                y=node.point2D.y,
                mapId=map_input.id,
                allowedDeviationTheta=0.1,
                allowedDeviationXY=0.1,
            ),
            actions=[],
        )

    map = Vda5050Map(
        map_info=Map(
            mapId=map_input.id, mapVersion="", mapDescription="", mapStatus="ENABLED"
        ),
        nodes=vda_nodes,
        edges=vda_edges,
        edge_mapping=vda_edge_mapping,
    )
    printGreen("Loaded VDA5050 Map")
    return map
