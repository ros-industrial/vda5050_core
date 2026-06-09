# rmf2_agv

### List of data models

The following entity types are available:
- [StateMessage](./StateMessage/README.md). State message
- [CommandMessage](./CommandMessage/README.md). Command message
- [CommandStatusMessage](./CommandStatusMessage/README.md). Command Status Message

### Example API

An [example python package]() is included to be used externally

```python

from rmf2_agv.StateMessage import (
    StateMessage,
    Destination,
    Pose,
    Velocity,
    Battery,
    Orientation2D,
)

from rmf2_agv.CommandMessage import CommandMessage, Point2D, Waypoint

from rmf2_agv.CommandStatusMessage import CommandStatusMessage

```
### Contributors
[Link](./CONTRIBUTERS.yaml) to the current contributors of the data models of this Subject.


### Contribution
You can raise an [issue](https://gitlab.com/ROSI-AP/rmf2/rmf2-data-models/rmf2_agv/-/issues) or submit your [PR](https://gitlab.com/ROSI-AP/rmf2/rmf2-data-models/rmf2_agv/-/merge_requests) on existing data models
