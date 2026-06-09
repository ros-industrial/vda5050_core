
Entity: TaskStatus   
======================
<!-- /10-Header -->
    
<!-- 15-License -->
[License](../LICENSE.md)    
<!-- /15-License -->
    
<!-- 20-Description -->
Global description: **Task Status Message**    

version: 0.0.1    
<!-- /20-Description -->
    
<!-- 30-PropertiesList -->
    

## List of properties    

<sup><sub>[*] If there is not a type in an attribute is because it could have several types or different formats/patterns</sub></sup>    
- `id[string]`: Task ID String
- `status[string]`: Status of the task.  Enum: IN_PROGRESS/COMPLETED/CANCELLED/PAUSED/ERROR
- `taskElapsedDuration[string]`: Total Elapsed time of the task from start time, defined in the ISO 8601 standard
- `taskEnd[date-time]`: End time of the task, defined in the ISO 8601 standard
- `taskStart[date-time]`: Start time of the task, defined in the ISO 8601 standard
- `taskType[string]`: Description of the Task Type
- `type[string]`: NGSI Entity type. It has to be TaskStatus  

<!-- /30-PropertiesList -->
    
<!-- 35-RequiredProperties -->

Required properties

- `id`
- `status`
- `taskElapsedDuration`
- `taskEnd`
- `taskStart`
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
TaskStatus:
  description: Task Status Message
  properties:
    id:
      description: Task ID String
      type: string
      x-ngsi:
        type: Property
    status:
      description: Status of the task.
      enum:
        - IN_PROGRESS
        - COMPLETED
        - CANCELLED
        - PAUSED
        - ERROR
      type: string
      x-ngsi:
        type: Property
    taskElapsedDuration:
      description: Total Elapsed time of the task from start time, defined in the ISO 8601 standard
      type: string
      x-ngsi:
        type: Property
    taskEnd:
      description: End time of the task, defined in the ISO 8601 standard
      format: date-time
      type: string
      x-ngsi:
        type: Property
    taskStart:
      description: Start Time of the task, defined in the ISO 8601 standard
      format: date-time
      type: string
      x-ngsi:
        type: Property
    taskType:
      description: Description of the Task Type
      type: string
      x-ngsi:
        type: Property
    type:
      description: NGSI Entity type. It has to be TaskStatus
      enum:
        - TaskStatus
      type: string
      x-ngsi:
        type: Property
  required:
    - id
    - status
    - taskElapsedDuration
    - taskEnd
    - taskStart
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

<!-- #### CommandMessage NGSI-LD key-values Example      

Here is an example of a CommandMessage in JSON-LD format as key-values. This is compatible with NGSI-LD when  using `options=keyValues` and returns the context data of an individual entity.    
<details><summary><strong>show/hide example</strong></summary>      

```json  
To be populated
```  
</details>     -->

#### TaskStatus NGSI-LD normalized Example      

Here is an example of a TaskStatus in JSON-LD format as normalized. This is compatible with NGSI-LD when not using options and returns the context data of an individual entity.    
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
    
