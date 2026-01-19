# vda5050_execution

## 1. Communication Layer

The `ProtocolAdapter` handles the external communication by abstracting MQTT
topic structure and JSON serialization, allowing the users to work with C++
structs.

```cpp
  // Initialize the MQTT client with broker details and identity
  auto mqtt_client = vda5050_core::mqtt_client::create_default_client(
    "tcp://localhost:1883", "client");

  // Create the prototocl adapter to manage VDA5050 headers and topic naming
  auto protocol_adapter = vda5050_execution::ProtocolAdapter::make(
    mqtt_client, "uagv", "v2", "ROS-I", "S001");

  // Subscribe to topic just by specifying types. The protocol adapter handles
  // the JSON parsing internally
  protocol_adapter->subscribe<vda5050_types::Order>(
    [&](
      vda5050_types::Order order, std::optional<vda5050_types::Error> error) {
        if (error.has_value())
        {
          // Error handling if the incoming MQTT payload was malformed
        }
        else
        {
          // Application logic for a valid order
          VDA5050_INFO_STREAM("Received order with ID: " << order.order_id);
        }
      }
    },
    0);

  // Publish a connection request. The protocol adapter automatically fills in
  // the header along with a timestamp
  vda5050_types::Connection connection;
  connection.connection_state = vda5050_types::ConnectionState::ONLINE;
  c->protocol_adapter_->publish<vda5050_types::Connection>(connection, 1);
```

## 2. Execution Layer

The `ExecutionEngine` and `Provider` handle the internal working of the
execution system by decoupling the logic from network. This allows for an
asynchronous execution of the VDA5050 commands.

```cpp
  // Custom events can be defined for internal communication. Or existing
  // events can be used instead
  struct MyCustomEvent : public vda5050_execution::EventBase
  {
    std::type_index get_type() const override
    {
      return typeid(MyCustomEvent);
    }
  };

  // Custom updates can be defined for internal communication. Or existing
  // updates can be used instead
  struct MyCustomUpdate : public vda5050_execution::UpdateBase
  {
    std::type_index get_type() const override
    {
      return typeid(MyCustomUpdate);
    }
  };

  // Intialize the engine (for downstream communication) and provider (for
  // upstream communication)
  auto engine = std::make_shared<vda5050_execution::ExecutionEngine>();
  auto provider = std::make_shared<vda5050_execution::Provider>();

  // Register logic for specific events
  engine->on<MyCustomEvent>([&](std::shared_ptr<MyCustomEvent> event) {
      // This code runs when step() is called and MyCustomEvent is in the
      // event queue
    });

  // Register logic for the specific updates. Usually used by any entity that
  // sends messages over MQTT
  provider->on<MyCustomUpdate>([&](std::shared_ptr<MyCustomUpdate> update) {
      // Process the update (e.g., prepare to send it over MQTT)
    });

  // Trigger an event (simulate a command received over MQTT)
  engine->emit<MyCustomEvent>();

  // Step the engine to process the event
  engine->step();

  // Push an update (simulate reporting of some status)
  provider->push<MyCustomUpdate>();
```
