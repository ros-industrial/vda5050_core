from datetime import datetime


class DataModelTask:
    ld_link = 'https://smart-data-models.github.io/dataModel.AutonomousMobileRobot/StateMessage/examples/example-normalized.jsonld; rel="https://www.w3.org/ns/json-ld#context"; type="application/ld+json"'

    def create_task(task_update_id, task_type, task_id):
        task_message = {}
        task_message["id"] = "urn:ngsi-ld:Task:" + task_id
        task_message["type"] = "Task"

        task_message["taskUpdateId"] = {}
        task_message["taskUpdateId"]["type"] = "Property"
        task_message["taskUpdateId"]["value"] = task_update_id

        task_message["taskId"] = {}
        task_message["taskId"]["type"] = "Property"
        task_message["taskId"]["value"] = task_id

        task_message["taskType"] = {}
        task_message["taskType"]["type"] = "Property"
        task_message["taskType"]["value"] = {}
        task_message["taskType"]["value"] = task_type

        task_message["taskTime"] = {}
        task_message["taskTime"]["type"] = "Property"
        task_message["taskTime"]["value"] = {}
        task_message["taskTime"]["value"] = datetime.now().strftime(
            "%d/%m/%Y, %H:%M:%S"
        )

    def create_task_status(task_id, task_status):
        task_status_message = {}
        task_status_message["id"] = "urn:ngsi-ld:Task:" + task_id + ":Status"
        task_status_message["type"] = "TaskStatus"

        task_status_message["task_id"] = {}
        task_status_message["task_id"]["type"] = "Property"
        task_status_message["task_id"]["value"] = task_id

        task_status_message["taskTime"] = {}
        task_status_message["taskTime"]["type"] = "Property"
        task_status_message["taskTime"]["value"] = {}
        task_status_message["taskTime"]["value"] = datetime.now().strftime(
            "%d/%m/%Y, %H:%M:%S"
        )

        # Warning: Deviation from open source standard: due to issue with single element lists:
        # https://github.com/ScorpioBroker/ScorpioBroker/issues/575
        task_status_message["status"] = {}
        task_status_message["status"]["type"] = "Property"
        task_status_message["status"]["value"] = task_status
        return task_status_message

    def create_replace_destination_task(task_update_id, task_id, destinations={}):
        base_task = DataModelTask.create_task(
            task_update_id, "replace_destination", task_id
        )

        # Warning: Deviation from open source standard: due to issue with single element lists:
        # https://github.com/ScorpioBroker/ScorpioBroker/issues/575
        base_task["taskDestinations"] = {}
        base_task["taskDestinations"]["type"] = "Property"
        base_task["taskDestinations"]["value"] = destinations
        return base_task

    def create_send_task(task_update_id, task_id, tasks={}):
        base_task = DataModelTask.create_task(task_update_id, "send_task", task_id)
        # Warning: Deviation from open source standard: due to issue with single element lists:
        # https://github.com/ScorpioBroker/ScorpioBroker/issues/575
        base_task["tasks"] = {}
        base_task["tasks"]["type"] = "Property"
        base_task["tasks"]["value"] = tasks
        return base_task
