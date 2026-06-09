from datetime import datetime

from time import sleep

import json

import requests

from requests.models import Response

import paho.mqtt.client as paho

from fiware_api.utils import (
    printRed,
    printGreen,
    printYellow,
    printPurple,
    printCyan,
    MQTTClient,
)


class ScorpioAPI:
    def __init__(self, endpoint) -> None:
        self.endpoint = endpoint

    def send_entity_message(self, json_body, headers):
        response = None
        entity_name = json_body["id"]
        if not self.does_entity_exist(entity_name):
            printYellow("Create new empty entity " + entity_name)
            response = self.query_broker(
                "POST", json_body, self.endpoint + "/ngsi-ld/v1/entities/", headers
            )
        else:
            json_body["last_update"] = datetime.now().strftime("%d/%m/%Y, %H:%M:%S")
            response = self.query_broker(
                "PATCH",
                json_body,
                self.endpoint + "/ngsi-ld/v1/entities/" + entity_name + "/attrs",
                headers,
            )
        return response

    def query_broker(self, method, json_body, endpoint, headers, params={}):
        response = ""
        status_code = 0
        while status_code > 299 or status_code < 200:
            try:
                if method == "POST":
                    response = requests.post(
                        endpoint,
                        data=json.dumps(json_body),
                        headers=headers,
                        timeout=30,
                    )
                elif method == "PATCH":
                    response = requests.patch(
                        endpoint,
                        data=json.dumps(json_body),
                        headers=headers,
                        timeout=30,
                    )
                elif method == "GET":
                    response = requests.get(
                        endpoint, headers=headers, timeout=30, params=params
                    )
                    if response.status_code == 404:
                        # Not Found, stop calling
                        return response
                elif method == "DELETE":
                    response = requests.delete(
                        endpoint,
                        headers=headers,
                        timeout=30,
                    )
                status_code = response.status_code
            except requests.exceptions.ConnectionError as e:
                printRed(str(e))
                pass
            except requests.exceptions.HTTPError as e:
                printRed(str(e))
                pass
            except requests.exceptions.Timeout as e:
                printRed(str(e))
                pass
            except requests.exceptions.TooManyRedirects as e:
                printRed(str(e))
                pass
            except requests.exceptions.RequestException as e:
                printRed(str(e))
                pass
        return response

    def delete_entity(self, entity_name):
        return self.query_broker(
            "DELETE",
            {},
            self.endpoint + "/ngsi-ld/v1/entities/" + entity_name,
            headers={
                "Accept": "application/ld+json",
                "Content-Type": "application/json",
            },
        )

    def get_all_entities_of_type(self, type_name):
        headers = {
            "Accept": "application/ld+json",
            "Content-Type": "application/json",
        }

        return self.query_broker(
            "GET",
            {},
            self.endpoint + "/ngsi-ld/v1/entities",
            headers,
            {"type": type_name},
        )

    def get_entity(self, entity_name, headers):
        return self.query_broker(
            "GET", {}, self.endpoint + "/ngsi-ld/v1/entities/" + entity_name, headers
        )

    def does_entity_exist(self, entity_name):
        headers = {
            "Accept": "application/ld+json",
            "Content-Type": "application/json",
        }
        response = self.get_entity(entity_name, headers)
        response_code = response.status_code
        if response_code == 404:
            return False
        return True

    def create_subscription(self, headers, id, subscription_endpoint, entities):
        msg = {}
        msg["id"] = "urn:subscription:" + id
        msg["type"] = "Subscription"
        msg["entities"] = entities
        msg["notification"] = {
            "endpoint": {"uri": subscription_endpoint, "accept": "application/json"}
        }

        response = self.query_broker(
            "GET", {}, self.endpoint + "/ngsi-ld/v1/subscriptions/" + msg["id"], headers
        )
        if response.status_code == 404:
            printYellow(
                "Create new subscription "
                + self.endpoint
                + "/ngsi-ld/v1/subscriptions/"
                + msg["id"]
            )
            return self.query_broker(
                "POST", msg, self.endpoint + "/ngsi-ld/v1/subscriptions", headers
            )
        else:
            printYellow(
                "Update subscription "
                + self.endpoint
                + "/ngsi-ld/v1/subscriptions/"
                + msg["id"]
            )
            return self.query_broker(
                "PATCH", msg, self.endpoint + "/ngsi-ld/v1/subscriptions", headers
            )

    def delete_subscription(self, headers, id):
        return self.query_broker(
            "DELETE",
            {},
            self.endpoint + "/ngsi-ld/v1/subscriptions/urn:subscription:" + id,
            headers=headers,
        )


class ScorpioEventSubscriber:
    def __init__(
        self,
        cb_endpoint,
        id,
        host,
        port,
        topic,
        message_type,
        ld_link,
        on_message_callback,
        qos=1,
        keepalive=0,
    ) -> None:
        self.context_broker_api = ScorpioAPI(cb_endpoint)
        self.ld_link = ld_link
        self.id = id
        self.message_type = message_type

        self.host = host
        self.port = port
        self.topic = topic

        # Create MQTT Client to Listen to AGV Updates
        self.mqtt_client = MQTTClient(
            id=topic,
            host=host,
            port=port,
            topic=topic,
            qos=qos,
            keepalive=keepalive,
            on_message_callback=on_message_callback,
            on_subscribe_callback=self.on_subscribe,
        )
        self.mqtt_client.run()

        # Create a Subscriber via ScorpioCB
        self.context_broker_api.create_subscription(
            headers={"Content-Type": "application/json", "Link": self.ld_link},
            id=self.id,
            subscription_endpoint="mqtt://"
            + self.host
            + ":"
            + str(self.port)
            + "/"
            + self.topic,
            entities=[{"type": self.message_type}],
        )
        printGreen("Created Fiware Subscription Entities ID: " + str(self.id))

    def on_connect(self, client, userdata, flags, rc):
        print("MQTT CLIENT: CONNACK received with code %d." % (rc))

    def run(self):
        # self.mqtt_client.run()
        print("already running")

    def on_subscribe(self, client, userdata, mid, granted_qos):
        print("MQTT CLIENT: Subscribed: " + str(mid) + " " + str(granted_qos))
