import paho.mqtt.client as paho
from datetime import datetime


# Print Colors
def printRed(skk):
    print(
        "\033[91m {}\033[00m".format(
            "[" + datetime.now().strftime("%H:%M:%S") + "] " + skk
        ),
        flush=True,
    )


def printGreen(skk):
    print(
        "\033[92m {}\033[00m".format(
            "[" + datetime.now().strftime("%H:%M:%S") + "] " + skk
        ),
        flush=True,
    )


def printYellow(skk):
    print(
        "\033[93m {}\033[00m".format(
            "[" + datetime.now().strftime("%H:%M:%S") + "] " + skk
        ),
        flush=True,
    )


def printPurple(skk):
    print(
        "\033[95m {}\033[00m".format(
            "[" + datetime.now().strftime("%H:%M:%S") + "] " + skk
        ),
        flush=True,
    )


def printCyan(skk):
    print(
        "\033[96m {}\033[00m".format(
            "[" + datetime.now().strftime("%H:%M:%S") + "] " + skk
        ),
        flush=True,
    )


def printLightGray(skk):
    print(
        "\033[97m {}\033[00m".format(
            "[" + datetime.now().strftime("%H:%M:%S") + "] " + skk
        ),
        flush=True,
    )


class MQTTClient:
    def __init__(
        self,
        id,
        host,
        port,
        topic,
        qos=1,
        keepalive=60,
        on_message_callback=None,
        on_subscribe_callback=None,
    ) -> None:
        self.mqtt_client = paho.Client(
            client_id=id + "_" + str(datetime.now().isoformat()), clean_session=True
        )

        # Create MQTT Client to Listen to AGV Updates
        self.mqtt_client.on_connect = self.on_connect

        if on_message_callback is not None:
            printYellow("Use Custom on message callback")
            self.mqtt_client.on_message = on_message_callback
        else:
            printPurple("Use default on message callback")
            self.mqtt_client.on_message = self.default_on_message

        if on_message_callback is not None:
            printYellow("Use Custom on subscribe callback")
            self.mqtt_client.on_subscribe = on_subscribe_callback
        else:
            printPurple("Use default on subscribe callback")
            self.mqtt_client.on_subscribe = self.default_on_subscribe

        self.mqtt_client.connect(host, int(port), keepalive=keepalive)
        self.mqtt_client.subscribe(topic, qos=qos)
        self.mqtt_client.loop_start()

    def run(self):
        print("already running")

    def on_connect(self, client, userdata, flags, rc):
        # printGreen("MQTT CLIENT: CONNACK received with code %d." % (rc))
        print("MQTT CLIENT: CONNACK received with code %d." % (rc))

    def default_on_subscribe(self, client, userdata, mid, granted_qos):
        # printGreen("MQTT CLIENT: Subscribed: " + str(mid) + " " + str(granted_qos))
        print("MQTT CLIENT: Subscribed: " + str(mid) + " " + str(granted_qos))

    def default_on_message(self, client, userdata, msg):
        print("Message Received.")
