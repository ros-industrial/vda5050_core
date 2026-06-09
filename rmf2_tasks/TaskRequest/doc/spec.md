
Entity: TaskRequest    
======================
<!-- /10-Header -->
    
<!-- 15-License -->
[License](../LICENSE.md)    
<!-- /15-License -->
    
<!-- 20-Description -->
Global description: **TaskRequest message**    

version: 0.0.1    
<!-- /20-Description -->
    
<!-- 30-PropertiesList -->
    

## List of properties    

<sup><sub>[*] If there is not a type in an attribute is because it could have several types or different formats/patterns</sub></sup>    
- `id[string]`: Task ID String
- `taskCommand[string]`: Command nature of the task Enum: START/PAUSE/RESUME/CANCEL
- `taskExpectedDuration[str]`: Expected total task duration, defined in the ISO 8601 standard
- `taskExpectedEnd[str]`: Expected End time of the task, defined in the ISO 8601 standard
- `taskExpectedStart[str]`: Expected Start time of the task, defined in the ISO 8601 standard
- `taskParams[array]`: List of Task Parameters represented as JSON Strings
- `taskType[string]`: Description of the Task Type
- `type[string]`: NGSI Entity type. It has to be TaskRequest  

<!-- /30-PropertiesList -->
    
<!-- 35-RequiredProperties -->

Required properties

- `id`
- `taskCommand`
- `taskExpectedDuration`
- `taskExpectedEnd`
- `taskExpectedStart`
- `taskParams`
- `taskType`
- `type` 

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
TaskRequest:
  description: TaskRequest message
  properties:
    id:
      description: Task ID String
      type: string
      x-ngsi:
        type: Property
    taskCommand:
      description: Description of the Task Type
      enum:
        - START
        - PAUSE
        - RESUME
        - CANCEL
      type: string
      x-ngsi:
        type: Property
    taskExpectedDuration:
      description: Expected duration of the task, defined in the ISO 8601 standard
      type: string
      x-ngsi:
        type: Property
    taskExpectedEnd:
      description: Expected End time of the task, defined in the ISO 8601 standard
      format: date-time
      type: string
      x-ngsi:
        type: Property
    taskExpectedStart:
      description: Expected Start time of the task, defined in the ISO 8601 standard
      format: date-time
      type: string
      x-ngsi:
        type: Property
    taskParams:
      description: List of Task Parameters represented as JSON Strings
      items:
        properties:
          content:
            description: Context of Parameter in JSON string
            type: string
            x-ngsi:
              type: Property
        required:
          - content
        type: object
      type: array
      x-ngsi:
        type: Property
    taskType:
      description: Description of the Task Type
      type: string
      x-ngsi:
        type: Property
    type:
      description: NGSI Entity type. It has to be TaskRequest
      enum:
        - TaskRequest
      type: string
      x-ngsi:
        type: Property
  required:
    - id
    - taskCommand
    - taskExpectedDuration
    - taskExpectedEnd
    - taskExpectedStart
    - taskParams
    - taskType
    - type
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

<!-- #### TaskRequest NGSI-LD key-values Example      

Here is an example of a CommandMessage in JSON-LD format as key-values. This is compatible with NGSI-LD when  using `options=keyValues` and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
To be populated
```  
</details>     -->

#### TaskRequest NGSI-LD normalized Example      

Here is an example of a Task in JSON-LD format as normalized. This is compatible with NGSI-LD when not using options and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json

TODO

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
    
