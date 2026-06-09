from datetime import datetime
from fiware_api.context_broker.scorpio import ScorpioAPI
from rmf2_agv.CommandMessage import CommandMessage
from rmf2_agv.CommonClasses import Waypoint, Orientation2D, Point2D
import json
import copy


def update_order(command: CommandMessage):
    # output_command = command
    output_command = copy.deepcopy(command)

    output_command.commandUpdateId = command.commandUpdateId + 1

    # Add a reversal of waypoints
    temp_waypoints = sorted(
        command.waypoints,
        key=lambda waypoint: waypoint.sequenceId,
    )
    temp_waypoints.reverse()

    for tempoint in temp_waypoints:
        print(
            "Node Name: "
            + tempoint.nodeId
            + "/nSequence ID: "
            + str(tempoint.sequenceId),
            flush=True,
        )

    print(
        "================================================================================",
        flush=True,
    )

    last_seq_id = temp_waypoints[0].sequenceId + 1
    for waypoint in temp_waypoints:
        waypoint.sequenceId = last_seq_id
        output_command.waypoints.append(waypoint)
        last_seq_id = last_seq_id + 1

    for tempoint in output_command.waypoints:
        print(
            "Node Name: "
            + tempoint.nodeId
            + "/nSequence ID: "
            + str(tempoint.sequenceId),
            flush=True,
        )

    return output_command


def main():
    num_robots = 4
    # Default Parameters if nothing is set.
    cb_endpoint = "http://localhost:9090"

    context_broker = ScorpioAPI(cb_endpoint)
    manufacturer = "MIR"
    for robot_count in range(num_robots):
        command_id = (
            "urn:ngsi-ld:Robot:"
            + manufacturer
            + ":"
            + str(robot_count)
            + ":CommandMessage"
        )
        command_message_json = json.loads(
            context_broker.get_entity(entity_name=command_id).text, {}
        )
        print(json.dumps(command_message_json, indent=2))

        command_message = CommandMessage.from_json(command_message_json)
        updated_command = update_order(command_message)
        updated_command_json = CommandMessage.to_json(updated_command)
        updated_command_json["id"] = command_id
        print(json.dumps(updated_command_json, indent=2))
        context_broker.send_entity_message(
            json_body=updated_command_json,
            headers={
                "Content-Type": "application/json",
                "Link": 'https://smart-data-models.github.io/dataModel.AutonomousMobileRobot/StateMessage/examples/example-normalized.jsonld; rel="https://www.w3.org/ns/json-ld#context"; type="application/ld+json"',
            },
        )


if __name__ == "__main__":
    main()
