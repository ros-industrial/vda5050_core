
Entity: CommandMessage    
======================
<!-- /10-Header -->
    
<!-- 15-License -->
[License](../LICENSE.md)    
<!-- /15-License -->
    
<!-- 20-Description -->
Global description: **Command message**    

version: 0.0.1    
<!-- /20-Description -->
    
<!-- 30-PropertiesList -->
    

## List of properties    

<sup><sub>[*] If there is not a type in an attribute is because it could have several types or different formats/patterns</sub></sup>    
- `command[string]`: Command sent to the robot
- `commandId[string]`: Unique ID of the command assigned to the robot
- `commandTime[date-time]`: Sent time to the robot  
- `commandUpdateId[number]`: Update count of the command assigned to the robot
- `type[string]`: NGSI Entity type. It has to be CommandMessage  
- `waypoints[array]`: List of waypoints  
<!-- /30-PropertiesList -->
    
<!-- 35-RequiredProperties -->
    

Required properties    
- `command`
- `commandId`
- `commandTime` 
- `commandUpdateId`
- `id`  
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
CommandMessage:      
  description: Command message      
  properties:      
    command:      
      description: Command sent to the robot      
      type: string      
      x-ngsi:      
        type: Property
    commandId:      
      description: Unique ID of the command assigned to the robot
      type: string      
      x-ngsi:      
        type: Property        
    commandTime:      
      description: Sent time to the robot      
      format: date-time      
      type: string      
      x-ngsi:      
        type: Property
    commandUpdateId:      
      description: Update count of the command assigned to the robot
      type: number      
      x-ngsi:      
        type: Property      
    type:      
      description: NGSI Entity type. It has to be CommandMessage      
      enum:      
        - CommandMessage      
      type: string      
      x-ngsi:      
        type: Property      
    waypoints:      
      description: List of waypoints      
      items:      
        properties:  
          nodeId:      
            description: ID of the corresponding mpa node of a waypoint     
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
    - id      
    - type      
    - commandTime
    - commandId
    - commandUpdateId      
    - command      
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

#### CommandMessage NGSI-LD normalized Example      

Here is an example of a CommandMessage in JSON-LD format as normalized. This is compatible with NGSI-LD when not using options and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
{
    "id": "urn:ngsi-ld:Robot:MiR:9001:CommandMessage",
    "type": "CommandMessage",
    "command": {
        "type": "Property",
        "value": "move_to"
    },
    "commandId": {
        "type": "Property",
        "value": "001"
    },
    "commandTime": {
        "type": "Property",
        "value": "07/10/2024, 08:00:42"
    },
    "commandUpdateId": {
        "type": "Property",
        "value": 2
    },
    "waypoints": {
        "type": "Property",
        "value": [
            {
                "nodeId": "P17",
                "point2D": {
                    "x": -72.0,
                    "y": -130.0
                },
                "released": true,
                "sequenceId": 0
            },
            {
                "nodeId": "P57",
                "point2D": {
                    "x": -72.0,
                    "y": -126.0
                },
                "released": true,
                "sequenceId": 1
            },
            {
                "nodeId": "P97",
                "point2D": {
                    "x": -72.0,
                    "y": -122.0
                },
                "released": true,
                "sequenceId": 2
            },
            {
                "nodeId": "P137",
                "point2D": {
                    "x": -72.0,
                    "y": -118.0
                },
                "released": true,
                "sequenceId": 3
            }
        ]
    },
    "@context": [
        "https://uri.etsi.org/ngsi-ld/v1/ngsi-ld-core-context-v1.7.jsonld"
    ]
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
    
