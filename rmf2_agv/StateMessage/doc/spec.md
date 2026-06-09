
Entity: StateMessage 
======================
<!-- /10-Header -->
    
<!-- 15-License -->
[License](../LICENSE.md)    
<!-- /15-License -->
    
<!-- 20-Description -->
Global description: **State message**    

version: 0.0.1    
<!-- /20-Description -->
    
<!-- 30-PropertiesList -->
    

## List of properties    

<sup><sub>[*] If there is not a type in an attribute is because it could have several types or different formats/patterns</sub></sup>    

- `battery[object]`: Battery State of the AGV
  - `remainingPercentage[number]`: Remaining Battery Charge
- `destination[object]`: Destination Node the AGV needs to reach
  - `mapId[string]`: Map ID      
  - `nodeId[string]`: Corresponding Node ID
  - `orientation2D[object]`: 2D Angle of an element           
  - `point2D[object]`: Point in 2D as a two simple coordinates x and y
  - `released[bool]`: Determines if AGV is allowed to reach this point
  - `sequenceId[number]`: Sequence ID of the destination node with respect to the route
- `pose[object]`: Current pose of AGV
  - `lastNodeSequenceId[number]`: The last traversed node's sequence ID with respect to the route  
  - `mapId[string]`: Map ID    
  - `orientation2D[object]`: 2D Angle of the AGV           
  - `point2D[object]`: Point in 2D as a two simple coordinates x and y
- `velocity[object]`: Velocity of AGV
  - `omega[number]`: The AGVs turning speed around its z axis.
  - `vx[number]`: The AGVs velocity in its x direction
  - `vy[number]`: The AGVs velocity in its y direction
- `status[string]`: Status of AGV. 
- `type[string]`: NGSI Entity type. It has to be StateMessage
- `waypoints[array]`: Route to be traversed by AGV
<!-- /30-PropertiesList -->
    
<!-- 35-RequiredProperties -->
    

Required properties    
- `battery`
- `destination`
- `pose` 
- `velocity`
- `status`  
- `type`  
- `waypoints`  
<!-- /35-RequiredProperties -->
    
<!-- 40-RequiredProperties -->
    
<!-- /40-RequiredProperties -->
    
<!-- 50-DataModelHeader -->
    

## Data Model description of properties    

Sorted alphabetically (click for details)    
<!-- /50-DataModelHeader -->
    
<!-- 60-ModelYaml -->
    
<details><summary><strong>full yaml details</strong></summary>      

```yaml    
StateMessage:
  description: State message
  properties:
    battery:
      description: Battery State of the AGV
      items:
        properties:
          remainingPercentage:
            description: Remaining Battery Charge
            type: string
            x-ngsi:
              type: Property
    destination:
      description: Destination Node the AGV needs to reach
      items:
        properties:
          mapId:
            description: Map ID
            type: string
            x-ngsi:
              type: Property
          nodeId:
            description: Destination's corresponding Node ID
            type: string
            x-ngsi:
              type: Property
          orientation2D:
            description: 2D Angle of an element
            properties:
              theta:
                default: 0.0
                description: Simple measurement of an angle
                type: number
                x-ngsi:
                  type: Property
            required:
              - theta
            type: object
            x-ngsi:
              type: Property
          point2D:
            description: Point in 2D as a two simple coordinates x and y
            properties:
              x:
                default: 0.0
                description: Simple coordinate of a point
                type: number
                x-ngsi:
                  type: Property
              y:
                default: 0.0
                description: Simple coordinate of a point
                type: number
                x-ngsi:
                  type: Property
            required:
              - x
              - y
            type: object
            x-ngsi:
              type: Property
          released:
            description: Determines if the waypoint is allowed to be traversed on yet
            type: bool
            x-ngsi:
              type: Property
          sequenceId:
            description: Waypoint sequence relative to entire path
            type: number
            x-ngsi:
              type: Property
    pose:
      description: Current pose of AGV
      items:
        properties:
          lastNodeSequenceId:
            description: The last traversed node's sequence ID with respect to the route
            type: number
            x-ngsi:
              type: Property
          mapId:
            description: Map ID
            type: string
            x-ngsi:
              type: Property
          orientation2D:
            description: 2D Angle of the AGV
            properties:
              theta:
                default: 0.0
                description: Simple measurement of an angle
                type: number
                x-ngsi:
                  type: Property
            required:
              - theta
            type: object
            x-ngsi:
              type: Property
          point2D:
            description: Point in 2D as a two simple coordinates x and y
            properties:
              x:
                default: 0.0
                description: Simple coordinate of a point
                type: number
                x-ngsi:
                  type: Property
              y:
                default: 0.0
                description: Simple coordinate of a point
                type: number
                x-ngsi:
                  type: Property
            required:
              - x
              - y
            type: object
            x-ngsi:
              type: Property
    velocity:
      description: Velocity of AGV
      items:
        properties:
          omega:
            description: The AGVs turning speed around its z axis.
            type: number
            x-ngsi:
              type: Property
          vx:
            description: The AGVs velocity in its x direction
            type: number
            x-ngsi:
              type: Property
          vy:
            description: The AGVs velocity in its y direction
            type: number
            x-ngsi:
              type: Property
    status:
      description: Status of AGV.
      type: string
      x-ngsi:
        type: Property
    type:
      description: NGSI Entity type. It has to be StateMessage
      enum:
        - StateMessage
      type: string
      x-ngsi:
        type: Property
    waypoints:
      description: Route to be traversed by AGV
      items:
        properties:
          nodeId:
            description: ID of the corresponding map node of a waypoint
            type: string
            x-ngsi:
              type: Property
          released:
            description: Determines if the waypoint is allowed to be traversed on yet
            type: bool
            x-ngsi:
              type: Property
          orientation2D:
            description: 2D Angle of the AGV
            properties:
              theta:
                default: 0.0
                description: Simple measurement of an angle
                type: number
                x-ngsi:
                  type: Property
            required:
              - theta
            type: object
            x-ngsi:
              type: Property
          point2D:
            description: Point in 2D as a two simple coordinates x and y
            properties:
              x:
                default: 0.0
                description: Simple coordinate of a point
                type: number
                x-ngsi:
                  type: Property
              y:
                default: 0.0
                description: Simple coordinate of a point
                type: number
                x-ngsi:
                  type: Property
            required:
              - x
              - y
            type: object
            x-ngsi:
              type: Property
          sequenceId:
            description: Waypoint sequence relative to entire path
            type: number
            x-ngsi:
              type: Property
        type: object
      type: array
      x-ngsi:
        type: Property
  required:
    - battery
    - destination
    - pose
    - velocity
    - status
    - type
    - waypoints
  type: object
  x-derived-from: ""
  x-disclaimer: ""
  x-license-url: ""
  x-model-schema: ""
  x-model-tags: ""
  x-version: 0.0.1

```    
</details>      
<!-- /60-ModelYaml -->
    
<!-- 70-MiddleNotes -->
    
<!-- /70-MiddleNotes -->
    
<!-- 80-Examples -->
    

## Example payloads      

<!-- #### CommandMessage NGSI-LD key-values Example      

Here is an example of a CommandMessage in JSON-LD format as key-values. This is compatible with NGSI-LD when  using `options=keyValues` and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
To be populated
```  
</details>     -->

#### StateMessage NGSI-LD normalized Example      

Here is an example of a StateMessage in JSON-LD format as normalized. This is compatible with NGSI-LD when not using options and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
{
    "id": "urn:ngsi-ld:StateMessage:MiR:0001",
    "type": "StateMessage",
    "battery": {
        "type": "Property",
        "value": {
            "remainingPercentage": 80
        }
    },
    "destination": {
        "type": "Property",
        "value": {
            "mapId": "urn:ngsi-ld:Map:ue5_building_final",
            "nodeId": "P912",
            "orientation2D": {
                "theta": 0.0
            },
            "point2D": {
                "x": 172.1300048828125,
                "y": -109.61000061035156
            },
            "released": true,
            "sequenceId": 8.0
        }
    },
    "pose": {
        "type": "Property",
        "value": {
            "lastNodeSequenceId": 7.0,
            "mapId": "",
            "orientation2D": {
                "theta": 0.0
            },
            "point2D": {
                "x": 0.0,
                "y": 0.0
            }
        }
    },
    "velocity": {
        "type": "Property",
        "value": {
            "omega": 1.0,
            "vx": 1.0,
            "vy": 1.0
        }
    },
    "waypoints": {
        "type": "Property",
        "value": {
            "mapId": "urn:ngsi-ld:Map:ue5_building_final",
            "nodeId": "P912",
            "orientation2D": {
                "theta": 0.0
            },
            "point2D": {
                "x": 172.1300048828125,
                "y": -109.61000061035156
            },
            "released": true,
            "sequenceId": 8.0
        }
    },
    "status": {
        "type": "Property",
        "value": "ACTIVE"
    }
}
```  
</details><!-- /80-Examples -->
    
<!-- 90-FooterNotes -->
    
<!-- /90-FooterNotes -->
    
<!-- 95-Units -->
#### 
See [FAQ 10](https://smartdatamodels.org/index.php/faqs/) to get an answer on how to deal with magnitude units    
<!-- /95-Units -->
    
<!-- 97-LastFooter -->
---    
<!-- /97-LastFooter -->
    
