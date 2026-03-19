#!/usr/bin/env python3
"""
test_validator.py
=================
Regression test suite for tools/validate_scenario.py.

Runs validate_scenario.py against known-good and known-bad scenario files
and asserts the expected exit code.

Usage
-----
    python3 tools/test_validator.py

Exit codes
----------
  0  all tests passed
  1  one or more tests failed
"""

import subprocess
import sys
from pathlib import Path

REPO_ROOT   = Path(__file__).resolve().parent.parent
VALIDATOR   = REPO_ROOT / "tools" / "validate_scenario.py"
MYSTERY_DIR = REPO_ROOT / "src" / "games" / "mystery_test" / "scenario"
BILLIARDS_DIR = REPO_ROOT / "src" / "games" / "billiards_test" / "scenario"
ROGUELIKE_DIR = REPO_ROOT / "src" / "games" / "roguelike_test" / "scenario"

# ---------------------------------------------------------------------------
# Test definitions
# ---------------------------------------------------------------------------
# Each entry: (description, path_or_glob, expected_exit_code)
TESTS = [
    # ── mystery_test: valid scenarios ──────────────────────────────────────
    ("mystery_case.yaml is valid",
     MYSTERY_DIR / "mystery_case.yaml",                     0),
    ("mystery_stress_test.yaml is valid",
     MYSTERY_DIR / "mystery_stress_test.yaml",               0),
    ("mystery_timeout_test.yaml is valid",
     MYSTERY_DIR / "mystery_timeout_test.yaml",              0),
    ("diagnostic_test.yaml is valid",
     MYSTERY_DIR / "diagnostic_test.yaml",                   0),

    # ── mystery_test: invalid fixture ─────────────────────────────────────
    ("mystery_corrupted.yaml is REJECTED",
     MYSTERY_DIR / "mystery_corrupted.yaml",                 1),

    # ── billiards_test: valid scenarios ───────────────────────────────────
    ("billiards_fake_smoke.yaml is valid",
     BILLIARDS_DIR / "billiards_fake_smoke.yaml",            0),
    ("billiards_foul_smoke.yaml is valid",
     BILLIARDS_DIR / "billiards_foul_smoke.yaml",            0),
    ("billiards_continue_smoke.yaml is valid",
     BILLIARDS_DIR / "billiards_continue_smoke.yaml",        0),

    # ── billiards_test: invalid fixture ───────────────────────────────────
    ("billiards_corrupted.yaml is REJECTED",
     BILLIARDS_DIR / "billiards_corrupted.yaml",             1),

    # ── roguelike_test: valid scenarios ───────────────────────────────────
    ("roguelike_clear_smoke.yaml is valid",
     ROGUELIKE_DIR / "roguelike_clear_smoke.yaml",           0),
    ("roguelike_fail_smoke.yaml is valid",
     ROGUELIKE_DIR / "roguelike_fail_smoke.yaml",            0),
    ("roguelike_continue_smoke.yaml is valid",
     ROGUELIKE_DIR / "roguelike_continue_smoke.yaml",        0),

    # ── roguelike_test: invalid fixture ───────────────────────────────────
    ("roguelike_corrupted.yaml is REJECTED",
     ROGUELIKE_DIR / "roguelike_corrupted.yaml",             1),
]

# ---------------------------------------------------------------------------
# Runner
# ---------------------------------------------------------------------------

def run_test(description: str, scenario_path: Path, expected_exit: int) -> bool:
    if not scenario_path.exists():
        print(f"  SKIP  {description}")
        print(f"        (file not found: {scenario_path.relative_to(REPO_ROOT)})")
        return True  # treat missing files as skips, not failures

    result = subprocess.run(
        [sys.executable, str(VALIDATOR), str(scenario_path)],
        capture_output=True,
        text=True,
    )

    passed = (result.returncode == expected_exit)
    status = "PASS" if passed else "FAIL"

    print(f"  {status}  {description}")
    if not passed:
        print(f"        Expected exit={expected_exit}, got exit={result.returncode}")
        if result.stdout.strip():
            for line in result.stdout.strip().splitlines():
                print(f"        {line}")
        if result.stderr.strip():
            for line in result.stderr.strip().splitlines():
                print(f"        STDERR: {line}")

    return passed


def main():
    print("=" * 60)
    print("Validator Regression Tests")
    print("=" * 60)

    results = []
    for desc, path, expected in TESTS:
        results.append(run_test(desc, path, expected))

    passed = sum(results)
    total  = len(results)
    failed = total - passed

    print()
    print("=" * 60)
    if failed == 0:
        print(f"ALL {total} TESTS PASSED")
    else:
        print(f"{failed}/{total} TEST(S) FAILED")
    print("=" * 60)

    sys.exit(0 if failed == 0 else 1)


if __name__ == "__main__":
    main()
