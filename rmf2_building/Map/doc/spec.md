
Entity: Map 
======================
<!-- /10-Header -->
    
<!-- 15-License -->
[License](../LICENSE.md)    
<!-- /15-License -->
    
<!-- 20-Description -->
Global description: **Map message**    

version: 0.0.1    
<!-- /20-Description -->
    
<!-- 30-PropertiesList -->
    

## List of properties    

<sup><sub>[*] If there is not a type in an attribute is because it could have several types or different formats/patterns</sub></sup>    

- `id[string]`: Unique Id assigned to Map
- `type[string]`: Property. NGSI Entity type. It has to be Map
- `updateTime[string]`: Last Update time of Map. date-time format
- `edges[array]`: Edges connecting Nodes in a Map
  - `edgeId[string]`: Edge ID
  - `connectedNodes[array]`: 
    - `nodeID[string]`: Node Id of connected node to this Edge
- `nodes[array]`: Edges connecting Nodes in a Map
  - `nodeId[string]`: Node Id of connected node to this Edge
  - `point2D[object]`: Point in 2D as a two simple coordinates x and y
- `raw[string]` : Raw, unparsed string representation of the building
<!-- /30-PropertiesList -->
    
<!-- 35-RequiredProperties -->

Required properties    
- `id`
- `type`
- `updateTime`
- `edges` 
- `nodes`
- `raw`
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
Map:
  description: Map message
  properties:
    id:
      description: Unique Id assigned to Map
      type: string
      x-ngsi:
        type: Property
    updateTime:
      description: Last Update for Map
      format: date-time
      type: string
      x-ngsi:
        type: Property
    type:
      description: NGSI Entity type. It has to be Map
      enum:
        - Map
      type: string
      x-ngsi:
        type: Property
    edges:
      description: Array of edges that connects nodes in the map
      items:
        properties:
          edgeId:
            description: Unique Id assigned to Edge
            type: string
            x-ngsi:
              type: Property
          connectedNodes:
            description: Array of Nodes this Edge connects
            items:
              properties:
                nodeId:
                  description: Node Id of connected node to this Edge
                  type: string
                  x-ngsi:
                    type: Property
              type: object
            type: array
            x-ngsi:
              type: Property
        type: object
      type: array
      x-ngsi:
        type: Property
    nodes:
      description: Array of Nodes in the Map
      items:
        properties:
          nodeId:
            description: Unique Id assigned to Node
            type: string
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
        type: object
      type: array
      x-ngsi:
        type: Property
    raw:
      description: 'Raw, unparsed string representation of the building'
      type: string
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

<!-- #### Map NGSI-LD key-values Example      

Here is an example of a Map in JSON-LD format as key-values. This is compatible with NGSI-LD when  using `options=keyValues` and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
To be populated
```  
</details>     -->

#### Map NGSI-LD normalized Example      

Here is an example of a Map in JSON-LD format as normalized. This is compatible with NGSI-LD when not using options and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
{
  "id": "urn:ngsi-ld:Map:TestMap",
  "type": "Map",
  "updateTime": {
      "type": "Property",
      "value": "07/10/2024, 08:00:42"
  },
  "edges": {
      "type": "Property",
      "value": [
          {
              "edgeId": "P0_P1",
              "connectedNodes": [
                  {
                      "nodeId" : "P0"
                  },
                  {
                      "nodeId" : "P1"
                  }
              ]
          },
          {
              "edgeId": "P1_P2",
              "connectedNodes": [
                  {
                      "nodeId" : "P1"
                  },
                  {
                      "nodeId" : "P2"
                  }
              ]
          }
      ]
  },
  "nodes": {
      "type": "Property",
      "value": [
          {
              "nodeId": "P0",
              "point2D": {
                  "x": -72.0,
                  "y": -130.0
              }
          },
          {
              "nodeId": "P1",
              "point2D": {
                  "x": -72.0,
                  "y": -126.0
              }
          },
          {
              "nodeId": "P2",
              "point2D": {
                  "x": -72.0,
                  "y": -122.0
              }
          }
      ]
  },
  "raw": {
      "type": "Property",
      "value": "Some Raw Map Data"
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
    
