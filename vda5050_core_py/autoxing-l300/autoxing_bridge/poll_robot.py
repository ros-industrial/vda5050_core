#!/usr/bin/env python3
"""Poll AXBot pose and planner state — GameTL diagram step ⑦."""

from __future__ import annotations

import argparse
import sys
from typing import Any

from autoxing_bridge import (
    DEFAULT_AT_GOAL_TOL_M,
    PLANNING_TOPIC,
    TRACKED_POSE_TOPIC,
    at_goal,
    distance_to_target,
    poll_pose_and_planning,
)
from api_client import print_json
from credentials import CONSTANTS as ROBOT
from get_move_status import get_move_status


def main() -> int:
    parser = argparse.ArgumentParser(
        description="One-shot poll of /tracked_pose and /planning_state.",
    )
    parser.add_argument("--move-id", type=int, help="optional GET /chassis/moves/{id}")
    parser.add_argument("--target-x", type=float, help="optional target X for at_goal check")
    parser.add_argument("--target-y", type=float, help="optional target Y for at_goal check")
    parser.add_argument(
        "--tol-m",
        type=float,
        default=DEFAULT_AT_GOAL_TOL_M,
        help=f"at_goal tolerance in meters (default: {DEFAULT_AT_GOAL_TOL_M})",
    )
    args = parser.parse_args()

    pose, planning = poll_pose_and_planning(ROBOT.ROBOT_IP)
    if pose is None:
        print("Failed to read /tracked_pose.", file=sys.stderr)
        return 1
    if planning is None:
        print("Failed to read /planning_state.", file=sys.stderr)
        return 1

    out: dict[str, Any] = {
        TRACKED_POSE_TOPIC: pose,
        PLANNING_TOPIC: planning,
    }

    if args.move_id is not None:
        move = get_move_status(args.move_id)
        if move is None:
            print(f"Failed to read move {args.move_id}.", file=sys.stderr)
            return 1
        out["move_status"] = move

    if args.target_x is not None and args.target_y is not None:
        out["distance_m"] = round(
            distance_to_target(pose, args.target_x, args.target_y) or 0.0,
            3,
        )
        out["at_goal"] = at_goal(
            pose,
            args.target_x,
            args.target_y,
            tol_m=args.tol_m,
        )

    print_json(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
