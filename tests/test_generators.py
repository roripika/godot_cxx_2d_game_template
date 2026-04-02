#!/usr/bin/env python3
"""
tests/test_generators.py
========================
4 Generator の統合 smoke テスト（最小退行検知）。

パイプライン:
  spec fixture → generator 実行 → validate_scenario.py → expected output 照合

各テストは以下の 3 ステップを順に実行し、いずれか失敗した時点で FAIL とする:
  Step 1: Generator を spec fixture で呼び出し、exit 0 と YAML 出力を確認
  Step 2: validate_scenario.py で生成 YAML を検証し、exit 0 を確認
  Step 3: 生成 YAML を PyYAML でロードし、expected output と構造的に一致するか比較

Usage:
  python3 tests/test_generators.py
  python3 tests/test_generators.py --verbose
  python3 tests/test_generators.py --generator branching  # 1 件だけ実行
"""

import sys
import os
import glob
import tempfile
import subprocess
from pathlib import Path

try:
    import yaml
except ImportError:
    print("ERROR: PyYAML が見つかりません。pip install pyyaml を実行してください。")
    sys.exit(1)

# ── パス定義 ────────────────────────────────────────────────────────────────
WORKSPACE = Path(__file__).parent.parent
VENV_PYTHON = WORKSPACE / ".temp_venv/bin/python3"
PYTHON = str(VENV_PYTHON) if VENV_PYTHON.exists() else sys.executable

GENERATORS = {
    "branching":    WORKSPACE / "tools/gen_scenario_branching.py",
    "turn_grid":    WORKSPACE / "tools/gen_scenario_turn_grid.py",
    "time_clock":   WORKSPACE / "tools/gen_scenario_time_clock.py",
    "event_driven": WORKSPACE / "tools/gen_scenario_event_driven.py",
}
FIXTURES = {
    "branching":    WORKSPACE / "tests/fixtures/spec_branching.yaml",
    "turn_grid":    WORKSPACE / "tests/fixtures/spec_turn_grid.yaml",
    "time_clock":   WORKSPACE / "tests/fixtures/spec_time_clock.yaml",
    "event_driven": WORKSPACE / "tests/fixtures/spec_event_driven.yaml",
}
EXPECTED = {
    "branching":    WORKSPACE / "scenarios/generated/branching_basic_expected_output.yaml",
    "turn_grid":    WORKSPACE / "scenarios/generated/turn_grid_basic_expected_output.yaml",
    "time_clock":   WORKSPACE / "scenarios/generated/time_clock_basic_expected_output.yaml",
    "event_driven": WORKSPACE / "scenarios/generated/event_driven_basic_expected_output.yaml",
}
VALIDATE = WORKSPACE / "tools/validate_scenario.py"


# ── ユーティリティ ──────────────────────────────────────────────────────────

def load_yaml_safe(path: Path):
    """YAML ファイルを読み込み Python オブジェクトを返す。失敗時は None。"""
    try:
        with open(path) as f:
            return yaml.safe_load(f)
    except Exception as e:
        return None


def diff_data(actual, expected, path: str = "") -> list[str]:
    """actual と expected の構造的差分を文字列リストで返す（最大 20 件）。"""
    diffs: list[str] = []

    if type(actual) != type(expected):
        diffs.append(
            f"  {path or '(root)'}: "
            f"型が異なります actual={type(actual).__name__!r}  expected={type(expected).__name__!r}"
        )
        return diffs

    if isinstance(expected, dict):
        for k in sorted(set(list(actual.keys()) + list(expected.keys()))):
            child = f"{path}.{k}" if path else k
            if k not in actual:
                diffs.append(f"  {child}: expected に存在するが actual にない")
            elif k not in expected:
                diffs.append(f"  {child}: actual に存在するが expected にない")
            else:
                diffs.extend(diff_data(actual[k], expected[k], child))
            if len(diffs) >= 20:
                break

    elif isinstance(expected, list):
        if len(actual) != len(expected):
            diffs.append(
                f"  {path or '(root)'}: "
                f"リスト長が異なります actual={len(actual)}  expected={len(expected)}"
            )
        else:
            for i, (a, e) in enumerate(zip(actual, expected)):
                diffs.extend(diff_data(a, e, f"{path}[{i}]"))
                if len(diffs) >= 20:
                    break

    else:
        if actual != expected:
            diffs.append(f"  {path or '(root)'}: {actual!r}  !=  {expected!r}")

    return diffs[:20]


# ── 各ステップ実行 ───────────────────────────────────────────────────────────

def step_generate(name: str, out_dir: str, verbose: bool) -> tuple[bool, str]:
    """Step 1: Generator を spec fixture で呼び出す。
    成功時は (True, 生成 YAML パス) を返す。失敗時は (False, "") を返す。
    """
    gen_path = GENERATORS[name]
    spec_path = FIXTURES[name]

    result = subprocess.run(
        [PYTHON, str(gen_path), str(spec_path), "--out-dir", out_dir],
        capture_output=True, text=True
    )

    if result.returncode != 0:
        msg = f"Generator exit {result.returncode}"
        if verbose:
            msg += f"\n  STDOUT: {result.stdout.strip()}"
            msg += f"\n  STDERR: {result.stderr.strip()}"
        return False, msg

    # タイムスタンプ付きファイル名を glob で取得（review.md を除外）
    yamls = [f for f in glob.glob(f"{out_dir}/*.yaml")]
    if not yamls:
        return False, "Generator が YAML を出力しなかった"

    return True, yamls[0]


def step_validate(yaml_path: str, verbose: bool) -> tuple[bool, str]:
    """Step 2: validate_scenario.py で生成 YAML を検証する。"""
    result = subprocess.run(
        [PYTHON, str(VALIDATE), yaml_path],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        msg = f"validate exit {result.returncode}"
        if verbose:
            msg += f"\n  STDOUT: {result.stdout.strip()}"
        return False, msg
    return True, ""


def step_compare(actual_path: str, expected_path: Path, verbose: bool) -> tuple[bool, str]:
    """Step 3: PyYAML でロードして構造的に比較する。"""
    actual = load_yaml_safe(Path(actual_path))
    expected = load_yaml_safe(expected_path)

    if actual is None:
        return False, f"actual YAML のロードに失敗: {actual_path}"
    if expected is None:
        return False, f"expected YAML のロードに失敗: {expected_path}"

    if actual == expected:
        return True, ""

    diffs = diff_data(actual, expected)
    msg = "データ構造が expected output と一致しない\n" + "\n".join(diffs)
    return False, msg


# ── テスト実行 ───────────────────────────────────────────────────────────────

def run_test(name: str, verbose: bool) -> bool:
    """1 Generator のテストを実行し、PASS/FAIL を返す。"""
    label = f"[{name}]"
    print(f"{label:<18}", end=" ", flush=True)

    with tempfile.TemporaryDirectory() as tmpdir:
        # Step 1: generate
        ok, info = step_generate(name, tmpdir, verbose)
        if not ok:
            print(f"FAIL  step=generate  {info}")
            return False
        yaml_path = info  # 生成 YAML のパス

        # Step 2: validate
        ok, info = step_validate(yaml_path, verbose)
        if not ok:
            print(f"FAIL  step=validate  {info}")
            return False

        # Step 3: compare
        ok, info = step_compare(yaml_path, EXPECTED[name], verbose)
        if not ok:
            print(f"FAIL  step=compare\n{info}")
            return False

    print("PASS")
    return True


# ── エントリポイント ─────────────────────────────────────────────────────────

def main() -> None:
    args = sys.argv[1:]
    verbose = "--verbose" in args or "-v" in args

    # --generator <name> で単体実行
    if "--generator" in args:
        idx = args.index("--generator")
        if idx + 1 >= len(args):
            print("ERROR: --generator の後にテンプレート名を指定してください。")
            sys.exit(1)
        target = args[idx + 1]
        if target not in GENERATORS:
            print(f"ERROR: 不明なテンプレート '{target}'。選択肢: {list(GENERATORS.keys())}")
            sys.exit(1)
        names = [target]
    else:
        names = list(GENERATORS.keys())

    print(f"Generator smoke test  ({len(names)} generators)")
    print("-" * 50)

    results: dict[str, bool] = {}
    for name in names:
        results[name] = run_test(name, verbose)

    print("-" * 50)
    passed = sum(results.values())
    total = len(results)
    for name, ok in results.items():
        status = "PASS" if ok else "FAIL"
        print(f"  {status}  {name}")
    print(f"\n{passed}/{total} passed")

    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
