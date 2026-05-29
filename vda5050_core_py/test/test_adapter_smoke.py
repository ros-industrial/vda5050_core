"""Smoke tests that do not require a broker.

These verify the module imports cleanly, types construct, and the Adapter +
NavigationManager chain can be wired together. They do NOT call .start() —
that would try to connect MQTT.
"""

import vda5050_core_py as vda


def test_module_surface():
    assert hasattr(vda, "Adapter")
    assert hasattr(vda, "NavigationManager")
    assert hasattr(vda, "ProtocolAdapter")
    assert hasattr(vda, "Node")
    assert hasattr(vda, "NodePosition")
    assert hasattr(vda, "AGVPosition")
    assert hasattr(vda, "create_default_mqtt_client")


def test_types_construct_and_roundtrip():
    pos = vda.NodePosition()
    pos.x = 1.5
    pos.y = -2.0
    pos.theta = 0.5
    pos.map_id = "map1"

    node = vda.Node()
    node.node_id = "N1"
    node.sequence_id = 0
    node.released = True
    node.node_position = pos

    assert node.node_id == "N1"
    assert node.sequence_id == 0
    assert node.released is True
    assert node.node_position.x == 1.5
    assert node.node_position.theta == 0.5
    assert node.node_position.map_id == "map1"


def test_optional_fields_accept_none():
    pos = vda.NodePosition()
    pos.theta = None
    pos.allowed_deviation_x_y = None
    assert pos.theta is None
    assert pos.allowed_deviation_x_y is None


def test_agv_position_defaults():
    p = vda.AGVPosition()
    assert p.position_initialized is False
    assert p.x == 0.0
    assert p.y == 0.0


def test_adapter_construct_without_starting():
    # No broker contact: we don't call .start().
    mqtt = vda.create_default_mqtt_client("tcp://localhost:1883", "test_smoke")
    protocol = vda.ProtocolAdapter.make(
        mqtt, "uagv", "2.0.0", "Manufacturer", "S001",
    )
    adapter = vda.Adapter.make(protocol)
    nav = adapter.navigation_manager()
    assert nav is not None

    # Register a callback; just verify no exception. The callback is never
    # invoked because we never start the spin loop.
    adapter.on_navigate(lambda node: None)
