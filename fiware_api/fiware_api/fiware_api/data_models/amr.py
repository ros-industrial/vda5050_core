from datetime import datetime


class DataModelAMR:
    ld_link = 'https://smart-data-models.github.io/dataModel.AutonomousMobileRobot/StateMessage/examples/example-normalized.jsonld; rel="https://www.w3.org/ns/json-ld#context"; type="application/ld+json"'

    def create_2d_pose(x, y, theta, map_id):
        pose = {}
        pose["type"] = "Property"

        pose_value = {}
        pose_value["point2D"] = {}
        pose_value["point2D"]["x"] = x
        pose_value["point2D"]["y"] = y
        orientation_2d = {}
        orientation_2d["theta"] = theta
        pose_value["orientation2D"] = orientation_2d
        pose_value["mapId"] = map_id
        pose["value"] = pose_value
        return pose

    def create_command_message(
        command_id, command_update_id, manufacturer, robot_id, waypoints={}
    ):
        command_message = {}
        command_message["id"] = (
            "urn:ngsi-ld:Robot:" + manufacturer + ":" + robot_id + ":CommandMessage"
        )
        command_message["type"] = "CommandMessage"
        command_message["commandTime"] = {}
        command_message["command"] = {}
        command_message["commandId"] = {}
        command_message["commandUpdateId"] = {}
        command_message["waypoints"] = {}

        command_message["commandTime"]["type"] = "Property"
        command_message["command"]["type"] = "Property"
        command_message["commandUpdateId"]["type"] = "Property"
        command_message["commandId"]["type"] = "Property"

        # Warning: Deviation from open source standard: due to issue with single element lists:
        # https://github.com/ScorpioBroker/ScorpioBroker/issues/575
        command_message["waypoints"]["type"] = "Property"
        command_message["waypoints"]["value"] = waypoints

        command_message["commandTime"]["value"] = {}
        command_message["command"]["value"] = {}

        command_message["commandTime"]["value"] = datetime.now().strftime(
            "%d/%m/%Y, %H:%M:%S"
        )
        command_message["commandId"]["value"] = command_id
        command_message["commandUpdateId"]["value"] = command_update_id
        command_message["command"]["value"] = "move_to"
        # command_message["command"] = json.dumps(json_input)
        return command_message

    def create_state_message(manufacturer, robot_id):
        state_message = {}
        state_message["id"] = (
            "urn:ngsi-ld:StateMessage:" + manufacturer + ":" + robot_id
        )

        state_message["type"] = "StateMessage"

        state_message["battery"] = {}
        state_message["battery"]["type"] = "Property"
        state_message["battery"]["value"] = {}

        # TODO: Warning: Deviation from open source standard: due to issue with single element lists:
        # https://github.com/ScorpioBroker/ScorpioBroker/issues/575
        state_message["waypoints"] = {}
        state_message["waypoints"]["type"] = "Property"
        state_message["waypoints"]["value"] = []

        state_message["status"] = {}
        state_message["status"]["type"] = "Property"
        state_message["status"]["value"] = {}

        state_message["velocity"] = {}
        state_message["velocity"]["type"] = "Property"
        state_message["velocity"]["value"] = {}

        return state_message

    def create_command_return_message(command_time, manufacturer, robot_id):
        command_return_message = {}
        command_return_message["id"] = (
            "urn:ngsi-ld:CommandReturnMessage:" + manufacturer + ":" + robot_id
        )
        command_return_message["type"] = "CommandReturnMessage"
        command_return_message["commandTime"] = command_time
        command_return_message["receivedTime"] = datetime.now().strftime(
            "%d/%m/%Y, %H:%M:%S"
        )
        #   command_return_message['errors'] = errors
        #   if errors:
        #     command_return_message['result'] = 'err'
        #   else:
        #     command_return_message['result'] = 'ack'
        return command_return_message
