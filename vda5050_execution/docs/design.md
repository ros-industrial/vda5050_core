# Design Document for vda5050_execution

This document outlines the architectural design and key components of the
`vda5050_execution` package. The goal is to provide a decoupled, type-safe,
and thread-safe library for processing VDA5050 messages and executing robot
commands.

## Overall Architecture

The package is structured as an asynchronous event-driven system. It separates
communication from execution logic by using a producer-consumer model centered
around a synchronized event queue and a type-erased callback registry.

The high-level flow of message looks as follows,

1. Message Translation: The `ProtocolAdapter` receives an MQTT payload which is
deserialized into concrete `vda5050_types` structs.

2. Event Dispatch: The translated message is wrapped into an `Event` object and
passed to the `EventQueue` through an `ExecutionEngine`.

3. Buffering: The `EventQueue` holds events in a thread-safe FIFO buffer
allowing other threads to return to their specific tasks.

4. Logic Processing: The `ExecutionEngine` is stepped in a continuous loop in a
dedicated thread. It pops events from the queue and checks the
`CallbackRegistry` to find the associated logic.

5. Feedback: As the execution progresses, the state changes can be reported by
pushing `Update` objects back through the `Provider`.

## Core Components

### 1. `ProtocolAdapter`

This acts like a translation layer between the raw MQTT network and internal
execution logic.

- It manages VDA5050 headers by automatically handling increment of `headerId`
and timestamps.

- It uses C++ templates and static traits to ensure only valid VDA5050
compliant types are processed.

- It does not manage the MQTT connection itself, instead it accepts a pointer
to `MqttClientInterface`, allowing it to share a connection with other services.

### 2. `EventBase` and `UpdateBase`

These are the primary data carriers during execution. To avoid heavy
inheritance, lightweight classes are used as base for all messages travelling
through the execution system.

- `EventBase` is the base class for all downstream messages (e.g., new order,
instant action, order cancellation, etc.).

- `UpdateBase` is the base class for all upstream messages (e.g., position
update, battery status, order status, action status, etc.).

- Both use a `get_type()` virtual function to return a `std::type_index`
allowing for efficient routing without the need to perform a `dynamic_cast`.

### 3. `CallbackRegistry`

It holds type-erased callbacks and manages the lifecycle of all their function
pointers mapped to specific types.

- For events (objects derived from `EventBase`), the registry stores a
`std::vector` of callbacks allowing multiple listeners to react to the same
commands.

- For updates (objects derived from `UpdateBase`), the registry maps a
specific callback to a one update. Typically used to route robot state changes
back to the execution system.

- The registry uses an `std::unordered_map<std::type_index, ...>` to store
lambda wrappers that reconstruct the original type before calling the user-
defined callback.

### 4. `EventQueue`

A thread-safe queue object for storage and retrieval of events.

- It is thread-safe due to an internally managed mutex.

- It uses variadic templates to construct events in-place and minimizing large
copies of objects.

### 5. `Provider`

The public-facing API for application logic to report back states to the
execution layer.

- It allows asynchronous state updates and handles the internal construction
and routing to the registry.

- It can be used to register listeners in the form of function pointers, to
observe specific updates from the application logic.

### 6. `ExecutionEngine`

Acts as an orchestration layer to drive the execution system forward.

- Stepping the engine, causes it to fetch an event from the `EventQueue`

- That event is then used to find a registered callback in the
`CallbackRegistry`. The function pointer to the callback is extracted and
the function executed.
