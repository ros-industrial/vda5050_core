
Entity: CommandStatusMessage    
======================
<!-- /10-Header -->
    
<!-- 15-License -->
[License](../LICENSE.md)    
<!-- /15-License -->
    
<!-- 20-Description -->
Global description: **Command Status Message**    

version: 0.0.1    
<!-- /20-Description -->
    
<!-- 30-PropertiesList -->
    

## List of properties    

<sup><sub>[*] If there is not a type in an attribute is because it could have several types or different formats/patterns</sub></sup>    
- `robotId[string]`: ID of Robot Executing the Command
- `commandId[string]`: Unique ID of the Command assigned to the robot
- `commandUpdateId[string]`: Update count of the command assigned to the robot
- `commandStatusTime[date-time]`: Time of Command Status recorded
- `path[array]`: Path of Command to Robot
  - `waypoint[object]` : Waypoint in Path
    - `nodeId[string]` : Node ID of waypoint
    - `orientation2D[object]` : 2D Orientation of waypoint
      - `theta[number]` : Simple measurement of an angle
    - `point2D[object]` : 2D Position of waypoint
      - `x[number]` : Simple x coordinate of a point      
      - `y[number]` : Simple y coordinate of a point      
    - `released[bool]` : Determines if the waypoint is allowed to be traversed on yet
    - `sequenceId[number]`: Waypoint sequence relative to entire path 
  - `completed[bool]` : Determines if Waypoint has been traversed by the Robot
- `completed[bool]` : Determines if the Path has been completed
- `type[string]`: NGSI Entity type. It has to be CommandStatusMessage  
<!-- /30-PropertiesList -->
    
<!-- 35-RequiredProperties -->
    

Required properties
- `robotId`      
- `type`      
- `commandStatusTime`
- `commandId`
- `commandUpdateId`      
- `path` 
- `completed`  
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
CommandStatusMessage:
  description: Command Status message
  properties:
    robotId:
      description: ID of Robot Executing the Command
      type: string
      x-ngsi:
        type: Property
    commandId:
      description: Unique ID of the command assigned to the robot
      type: string
      x-ngsi:
        type: Property
    commandStatusTime:
      description: Time of Command Status recorded
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
      description: NGSI Entity type. It has to be CommandStatusMessage
      enum:
        - CommandMessage
      type: string
      x-ngsi:
        type: Property
    path:
      description: Path of Command to Robot
      items:
        properties:
          released:
            description: Determines if the waypoint is allowed to be traversed on yet
            type: bool
            x-ngsi:
              type: Property
          waypoint:
            description: Waypoint in Path
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
                    default: 0
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
                    default: 0
                    description: Simple coordinate of a point
                    type: number
                    x-ngsi:
                      type: Property
                  'y':
                    default: 0
                    description: Simple coordinate of a point
                    type: number
                    x-ngsi:
                      type: Property
                required:
                  - x
                  - 'y'
                type: object
                x-ngsi:
                  type: Property
              sequenceId:
                description: Waypoint sequence relative to entire path
                type: number
                x-ngsi:
                  type: Property
            required:
              - nodeId
              - point2D
              - released
              - sequenceId
            type: object
            x-ngsi:
              type: Property
        required:
          - waypoint
          - released
        type: object
      type: array
      x-ngsi:
        type: Property
  required:
    - robotId
    - type
    - commandStatusTime
    - commandId
    - commandUpdateId
    - path
  type: object
  x-derived-from: ''
  x-disclaimer: ''
  x-license-url: ''
  x-model-schema: ''
  x-model-tags: ''
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
  "id": "urn:ngsi-ld:Robot:MiR:9001:CommandStatusMessage",
  "type": "CommandStatusMessage",
  "commandId": {
    "type": "Property",
    "value": "001"
  },
  "commandStatusTime": {
    "type": "Property",
    "value": "07/10/2024, 08:00:42"
  },
  "commandUpdateId": {
    "type": "Property",
    "value": 2
  },
  "path": {
    "type": "Property",
    "value": [
      {
        "waypoint": {
          "nodeId": "P17",
          "point2D": {
            "x": -72,
            "y": -130
          },
          "orientation2D": {
            "theta": -72
          },
          "released": true,
          "sequenceId": 0
        },
        "completed": true
      },
      {
        "waypoint": {
          "nodeId": "P57",
          "point2D": {
            "x": -72,
            "y": -126
          },
          "orientation2D": {
            "theta": -71
          },
          "released": true,
          "sequenceId": 1
        },
        "completed": false
      }
    ]
  },
  "completed": true,
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
    
