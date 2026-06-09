#!/usr/bin/env python3
"""
Run All VDA5050 Tests

This script runs all VDA5050 tests and provides a summary.

Tests included:
1. test_vda5050_basic.py - Basic VDA5050 functionality
2. test_stitch_guard_unit.py - Stitch guard basic tests
3. test_stitch_guard_orderupdateid.py - orderUpdateId fix tests

Run: python3 run_all_tests.py
"""

import subprocess
import sys
import os

def run_test(test_file: str) -> tuple:
    """Run a test file and return (success, output)."""
    try:
        result = subprocess.run(
            [sys.executable, test_file],
            capture_output=True,
            text=True,
            cwd=os.path.dirname(os.path.abspath(__file__))
        )
        return result.returncode == 0, result.stdout + result.stderr
    except Exception as e:
        return False, str(e)


def main():
    print("=" * 70)
    print("VDA5050 TEST SUITE")
    print("=" * 70)

    tests = [
        ("Basic VDA5050 Functionality", "test_vda5050_basic.py"),
        ("Stitch Guard Unit Tests", "test_stitch_guard_unit.py"),
        ("Stitch Guard orderUpdateId Fix", "test_stitch_guard_orderupdateid.py"),
    ]

    results = []

    for name, test_file in tests:
        print(f"\n{'=' * 70}")
        print(f"Running: {name}")
        print(f"File: {test_file}")
        print("=" * 70)

        success, output = run_test(test_file)
        results.append((name, success))

        # Print last few lines of output (summary)
        lines = output.strip().split('\n')
        summary_start = None
        for i, line in enumerate(lines):
            if "TEST SUMMARY" in line or "ALL TESTS PASSED" in line:
                summary_start = max(0, i - 1)
                break

        if summary_start is not None:
            print('\n'.join(lines[summary_start:]))
        else:
            # Print last 10 lines if no summary found
            print('\n'.join(lines[-10:]))

    # Final summary
    print("\n" + "=" * 70)
    print("FINAL SUMMARY")
    print("=" * 70)

    all_passed = True
    for name, success in results:
        status = "PASS" if success else "FAIL"
        if not success:
            all_passed = False
        print(f"  [{status}] {name}")

    print()
    if all_passed:
        print("=" * 70)
        print("ALL TEST SUITES PASSED!")
        print("=" * 70)
        return 0
    else:
        print("=" * 70)
        print("SOME TEST SUITES FAILED!")
        print("=" * 70)
        return 1


if __name__ == "__main__":
    sys.exit(main())
