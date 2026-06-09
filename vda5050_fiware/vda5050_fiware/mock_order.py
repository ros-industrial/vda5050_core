from datetime import datetime
from fiware_api.context_broker.scorpio import ScorpioAPI
from rmf2_agv.CommandMessage import CommandMessage
from rmf2_agv.CommonClasses import Waypoint, Orientation2D, Point2D
import json


def generate_waypoints(num_points: int):
    waypoints = []
    for i in range(num_points):
        waypoints.append(
            Waypoint(
                nodeId="P" + str(i),
                orientation2D=Orientation2D(theta=0.0),
                released=True,
                sequenceId=i,
                point2D=Point2D(x=0.0, y=0.0),
            )
        )
    return waypoints


def main():
    num_robots = 4
    # Default Parameters if nothing is set.
    cb_endpoint = "http://localhost:9090"

    context_broker = ScorpioAPI(cb_endpoint)
    command_message = CommandMessage(
        type="CommandMessage",
        command="move_to",
        commandId="1234",
        commandTime=datetime.now().isoformat(),
        commandUpdateId=0,
        waypoints=generate_waypoints(7),
    )

    manufacturer = "MIR"

    command_message_json = {
        "type": "CommandMessage",
        "commandTime": {"type": "Property", "value": command_message.commandTime},
        "command": {"type": "Property", "value": command_message.command},
        "commandId": {"type": "Property", "value": command_message.commandId},
        "commandUpdateId": {
            "type": "Property",
            "value": command_message.commandUpdateId,
        },
        "waypoints": {"type": "Property", "value": []},
    }

    waypoints = []

    for waypoint in command_message.waypoints:
        waypoints.append(
            {
                "nodeId": waypoint.nodeId,
                "point2D": {"x": waypoint.point2D.x, "y": waypoint.point2D.y},
                "orientation2D": {"theta": waypoint.orientation2D.theta},
                "released": json.dumps(waypoint.released),
                "sequenceId": waypoint.sequenceId,
            }
        )
    command_message_json["waypoints"]["value"] = waypoints

    for robot_count in range(num_robots):
        command_message_json["id"] = (
            "urn:ngsi-ld:Robot:"
            + manufacturer
            + ":"
            + str(robot_count)
            + ":CommandMessage"
        )

        context_broker.send_entity_message(
            json_body=command_message_json,
            headers={
                "Content-Type": "application/json",
                "Link": 'https://smart-data-models.github.io/dataModel.AutonomousMobileRobot/StateMessage/examples/example-normalized.jsonld; rel="https://www.w3.org/ns/json-ld#context"; type="application/ld+json"',
            },
        )


if __name__ == "__main__":
    main()
