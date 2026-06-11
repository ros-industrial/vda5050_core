#!/usr/bin/env python3
"""Navigate to X Y then poll until terminal move_state (no VDA5050 yet)."""

from __future__ import annotations

import argparse
import sys

from autoxing_bridge import (
    CREATOR,
    DEFAULT_POLL_INTERVAL_S,
    distance_to_target,
    poll_pose_and_planning,
    wait_for_arrival,
)
from credentials import timeout_seconds
from navigate import navigate


def main() -> int:
    parser = argparse.ArgumentParser(
        description="POST /chassis/moves then poll until succeeded|failed|cancelled.",
    )
    parser.add_argument(
        "rest",
        nargs="*",
        help="X Y [ORI radians CCW from East]",
    )
    parser.add_argument("--target-x", type=float, help="target_x (meters)")
    parser.add_argument("--target-y", type=float, help="target_y (meters)")
    parser.add_argument(
        "--target-ori",
        type=float,
        default=0.0,
        help="target_ori radians CCW from East (default: 0.0)",
    )
    parser.add_argument(
        "--target-accuracy",
        type=float,
        help="optional target_accuracy (meters)",
    )
    parser.add_argument(
        "--poll-interval",
        type=float,
        default=DEFAULT_POLL_INTERVAL_S,
        help=f"seconds between polls (default: {DEFAULT_POLL_INTERVAL_S})",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=None,
        help="overall wait timeout in seconds (default: spellbook timeout_ms)",
    )
    args = parser.parse_args()

    ax = args.target_x
    ay = args.target_y
    ao = args.target_ori

    if args.rest:
        if len(args.rest) == 3:
            ax, ay, ao = float(args.rest[0]), float(args.rest[1]), float(args.rest[2])
        elif len(args.rest) == 2:
            ax, ay = float(args.rest[0]), float(args.rest[1])
        else:
            print("Provide X Y [ORI] or --target-x/--target-y.", file=sys.stderr)
            return 1
    elif ax is None or ay is None:
        print("Provide X Y [ORI] or --target-x/--target-y.", file=sys.stderr)
        return 1

    move = navigate(
        ax,
        ay,
        ao,
        creator=CREATOR,
        target_accuracy=args.target_accuracy,
    )
    if move is None:
        print("Navigate request failed.", file=sys.stderr)
        return 1

    move_id = move.get("id")
    print(f"Move dispatched: id={move_id} target=({ax}, {ay}, {ao})", file=sys.stderr)

    timeout_s = args.timeout if args.timeout is not None else timeout_seconds()

    def _on_poll(_agv, move_state: str | None) -> None:
        pose, _planning = poll_pose_and_planning()
        dist = distance_to_target(pose, ax, ay)
        dist_str = f"{dist:.2f}m" if dist is not None else "n/a"
        state_str = move_state or "unknown"
        print(f"  move_state={state_str} distance={dist_str}", file=sys.stderr)

    try:
        terminal = wait_for_arrival(
            ax,
            ay,
            move_id=move_id,
            timeout_s=timeout_s,
            poll_interval_s=args.poll_interval,
            on_poll=_on_poll,
        )
    except TimeoutError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    if terminal == "succeeded":
        print("Arrived.", file=sys.stderr)
        return 0

    print(f"Move finished: {terminal}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
