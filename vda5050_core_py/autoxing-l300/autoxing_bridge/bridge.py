"""Shared Autoxing API layer for GameTL VDA5050 integration."""

from __future__ import annotations

import math
import time
from collections.abc import Callable
from typing import Any

from autoxing_bridge._spellbook_path import ensure_spellbook

ensure_spellbook()

from credentials import CONSTANTS as ROBOT  # noqa: E402
from credentials import timeout_seconds  # noqa: E402
from get_move_status import get_move_status  # noqa: E402
from navigate import navigate, planning_move_state  # noqa: E402
from ws_helper import ws_get_topics  # noqa: E402

import vda5050_core_py as vda

CREATOR = "vda5050_autoxing_l300"
TRACKED_POSE_TOPIC = "/tracked_pose"
PLANNING_TOPIC = "/planning_state"
TERMINAL_MOVE_STATES = frozenset({"succeeded", "failed", "cancelled"})
DEFAULT_POLL_INTERVAL_S = 0.5
DEFAULT_AT_GOAL_TOL_M = 0.15


def dispatch_move(node: vda.Node, *, target_accuracy: float | None = None) -> dict | None:
    """Map VDA5050 node → POST /chassis/moves (spellbook navigate)."""
    pos = node.node_position
    target_ori = pos.theta if pos.theta is not None else 0.0
    return navigate(
        pos.x,
        pos.y,
        target_ori,
        creator=CREATOR,
        target_accuracy=target_accuracy,
    )


def poll_pose_and_planning(
    robot_ip: str | None = None,
    *,
    timeout: float | None = None,
) -> tuple[dict[str, Any] | None, dict[str, Any] | None]:
    """One-shot WS poll of /tracked_pose and /planning_state."""
    ip = robot_ip or ROBOT.ROBOT_IP
    try:
        got = ws_get_topics(
            ip,
            [TRACKED_POSE_TOPIC, PLANNING_TOPIC],
            timeout=timeout,
        )
    except TypeError:
        return None, None
    return got.get(TRACKED_POSE_TOPIC), got.get(PLANNING_TOPIC)


def move_state_from_status(status: dict[str, Any] | None) -> str | None:
    """Extract terminal move state from GET /chassis/moves/{id} response."""
    if not isinstance(status, dict):
        return None
    for key in ("state", "move_state", "status"):
        val = status.get(key)
        if isinstance(val, str):
            return val
    return None


def distance_to_target(
    pose: dict[str, Any] | None,
    target_x: float,
    target_y: float,
) -> float | None:
    """Euclidean distance from tracked pose to target XY (meters)."""
    if not isinstance(pose, dict):
        return None
    pos = pose.get("pos")
    if not isinstance(pos, (list, tuple)) or len(pos) < 2:
        return None
    return math.hypot(float(pos[0]) - target_x, float(pos[1]) - target_y)


def at_goal(
    pose: dict[str, Any] | None,
    target_x: float,
    target_y: float,
    *,
    tol_m: float = DEFAULT_AT_GOAL_TOL_M,
) -> bool:
    """True when tracked pose is within ``tol_m`` of the target XY."""
    dist = distance_to_target(pose, target_x, target_y)
    return dist is not None and dist <= tol_m


def tracked_pose_to_agv_position(
    pose: dict[str, Any] | None,
    map_id: str = "",
) -> vda.AGVPosition:
    """Convert WS /tracked_pose payload to VDA5050 AGVPosition."""
    agv = vda.AGVPosition()
    agv.position_initialized = False
    agv.map_id = map_id
    if not isinstance(pose, dict):
        return agv

    pos = pose.get("pos")
    if isinstance(pos, (list, tuple)) and len(pos) >= 2:
        agv.x = float(pos[0])
        agv.y = float(pos[1])
        agv.position_initialized = True

    ori = pose.get("ori")
    if ori is not None:
        agv.theta = float(ori)

    return agv


def wait_for_arrival(
    target_x: float,
    target_y: float,
    *,
    move_id: int | None = None,
    robot_ip: str | None = None,
    map_id: str = "",
    timeout_s: float | None = None,
    poll_interval_s: float = DEFAULT_POLL_INTERVAL_S,
    on_poll: Callable[[vda.AGVPosition, str | None], None] | None = None,
) -> str:
    """
    Poll until terminal move_state (succeeded|failed|cancelled).

    Uses GET /chassis/moves/{id} when ``move_id`` is set (REST, no websockets).
    Also tries WS /tracked_pose for VDA5050 position mirroring when available.
    """
    timeout = timeout_s if timeout_s is not None else timeout_seconds()
    deadline = time.monotonic() + timeout
    last_state: str | None = None

    while time.monotonic() < deadline:
        move_state: str | None = None
        if move_id is not None:
            move_state = move_state_from_status(get_move_status(int(move_id)))

        pose_msg, planning_msg = poll_pose_and_planning(timeout=poll_interval_s + 1.0)
        if move_state is None and planning_msg is not None:
            move_state = planning_move_state(planning_msg)

        agv = tracked_pose_to_agv_position(pose_msg, map_id=map_id)
        if move_state is not None:
            last_state = move_state

        if on_poll is not None:
            on_poll(agv, move_state)

        if move_state in TERMINAL_MOVE_STATES:
            return move_state

        time.sleep(poll_interval_s)

    raise TimeoutError(
        f"Timed out after {timeout:.0f}s waiting for terminal move_state "
        f"(last={last_state!r}, target=({target_x}, {target_y}))."
    )
