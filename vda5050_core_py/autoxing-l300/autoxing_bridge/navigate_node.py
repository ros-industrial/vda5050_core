#!/usr/bin/env python3
"""POST /chassis/moves — GameTL diagram step ⑥ (spellbook navigate)."""

from __future__ import annotations

import argparse
import sys

from autoxing_bridge import CREATOR
from api_client import print_json
from navigate import monitor_move_after_dispatch, navigate


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Dispatch a standard move to X Y [ORI] via spellbook POST /chassis/moves.",
    )
    parser.add_argument(
        "rest",
        nargs="*",
        help="X Y [ORI radians CCW from East]",
    )
    parser.add_argument("--target-x", type=float, help="target_x (meters, map frame)")
    parser.add_argument("--target-y", type=float, help="target_y (meters, map frame)")
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
        "--no-monitor",
        action="store_true",
        help="POST only; skip /planning_state watch",
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

    print_json(move)
    if not args.no_monitor:
        monitor_move_after_dispatch()
    return 0


if __name__ == "__main__":
    sys.exit(main())
