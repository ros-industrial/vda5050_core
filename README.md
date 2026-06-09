# VDA5050 FIWARE Bridge

This repository contains the VDA5050 to FIWARE integration, bridging VDA5050 AGV communication with the [Scorpio Context Broker](https://github.com/ScorpioBroker).

This module contains an MQTT Client subscribing to all related VDA5050 topics and converting them to the
existing [FIWARE AMR Data Model](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot)
and storing it in the [Scorpio Context Broker](https://github.com/ScorpioBroker).

---

## Quick Start

### 1. Clone the Repository

```bash
git clone <your-repo-url> vda5050_fiware
cd vda5050_fiware
```

### 2. Build the Docker Image

```bash
# From repo root
docker build -t vda5050_fiware:latest .
```

Build time: ~5-10 minutes (first build)

### 3. Create Docker Network (if not exists)

```bash
docker network create rmf2_broker_rmf-network
```

### 4. Run with Docker Compose

```bash
docker compose up -d
```

Or run standalone:
```bash
docker run -d \
  --name vda5050_fiware \
  --network rmf2_broker_rmf-network \
  vda5050_fiware:latest \
  bash -c "source /colcon_ws/install/setup.bash && ros2 run vda5050_fiware vda5050_fiware mosquitto 1928 http://scorpio:9090 rmf1"
```

### Prerequisites

This container requires these services on the same Docker network:
- **mosquitto** - MQTT broker (port 1883)
- **scorpio** - FIWARE Context Broker (port 9090)

---

# VDA5050 to FIWARE AMR message mapping

The conversion of the [VDA5050 Json Schemas](https://github.com/VDA5050/VDA5050/tree/main/json_schemas) to the [Fiware AMR Messages](https://github.com/smart-data-models/dataModel.AutonomousMobileRobot) can be found in 
[this Readme](vda5050-fiware.md)

Note that the Scorpio Broker uses the NGSI-LD format, hence we use the [already provided jsonld file for the Fiware AMR standard](https://smart-data-models.github.io/dataModel.AutonomousMobileRobot/StateMessage/examples/example-normalized.jsonld)

# Before running...
The demo assumes that there is an MQTT Broker already running on another container, named `mosquitto-net`

An example snippet that should be run in `docker-compose` is as follows:

```yaml
version: '3'
services:
  mosquitto:
    image: eclipse-mosquitto
    hostname: mosquitto
    container_name: mosquitto
    restart: unless-stopped
    networks:
      - mosquitto-net
    volumes:
      - /etc/mosquitto:/etc/mosquitto
      - /etc/mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf
networks:
  mosquitto-net:
    name: mosquitto-net
    driver: bridge
```
# Running the Demo

## Terminal 1: Running Context Broker

```
docker-compose -f scorpio-compose.yml up
```

## Terminal 2: Running VDA5050_fiware

```bash
docker-compose up --build
```

