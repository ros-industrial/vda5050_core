"""Smoke tests that do not require a broker.

These verify the module imports cleanly, types construct, and the RobotRuntime +
Reporter chain can be wired together. They do NOT call .start() — that would try
to connect MQTT.
"""

import vda5050_core_py as vda


def test_module_surface():
    assert hasattr(vda, "RobotRuntime")
    assert hasattr(vda, "Reporter")
    assert hasattr(vda, "Node")
    assert hasattr(vda, "Edge")
    assert hasattr(vda, "Order")
    assert hasattr(vda, "NodePosition")
    assert hasattr(vda, "AGVPosition")
    # Removed building blocks are no longer exposed to Python.
    assert not hasattr(vda, "ProtocolAdapter")
    assert not hasattr(vda, "Adapter")
    assert not hasattr(vda, "create_default_mqtt_client")


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


def test_edge_and_order_construct():
    edge = vda.Edge()
    edge.edge_id = "E1"
    edge.sequence_id = 1
    edge.start_node_id = "N0"
    edge.end_node_id = "N1"

    order = vda.Order()
    order.order_id = "O1"
    order.nodes = [vda.Node()]
    order.edges = [edge]

    assert order.order_id == "O1"
    assert len(order.nodes) == 1
    assert order.edges[0].end_node_id == "N1"


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


def test_runtime_construct_without_starting():
    # No broker contact: we don't call .start().
    runtime = vda.RobotRuntime(
        broker="tcp://localhost:1883",
        client_id="test_smoke",
        manufacturer="Manufacturer",
        serial_number="S001",
    )
    rep = runtime.reporter()
    assert rep is not None

    # Register callbacks; just verify no exception. They never fire because we
    # never start the loop.
    runtime.on_navigate(lambda node, edge: None)
    runtime.on_base(lambda order: None)
