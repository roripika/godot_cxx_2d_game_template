#!/usr/bin/env python3
"""
tests/test_generators.py
========================
4 Generator の統合 smoke テスト（Batch B 対応）。

パイプライン:
  spec fixture → generator 実行 → validate_scenario.py → expected output 照合

正常系（GTC-01）は以下の 3 ステップを順に実行し、
いずれか失敗した時点で FAIL とする:
  Step 1: Generator を spec fixture で呼び出し、exit 0 と YAML 出力を確認
  Step 2: validate_scenario.py で生成 YAML を検証し、exit 0 を確認
  Step 3: 生成 YAML を PyYAML でロードし、expected output と構造的に一致するか比較

負ケース（GTC-02 / GTC-03 / GTC-04 / GTC-05）は以下を確認する:
  - generator が non-zero exit を返す
  - YAML を生成しない
  - GTC-03 は失敗理由に主要キーワード（SPEC ERROR など）を含む

補助ケース:
  - GTC-06: 正常系実行時に *_review.md が生成される
  - GTC-07: --generator <name> で指定 1 件のみ実行される

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
SCRIPT_PATH = Path(__file__).resolve()
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
NEG_FIXTURES_TEMPLATE = {
    "branching":    WORKSPACE / "tests/fixtures/spec_branching_invalid_template.yaml",
    "turn_grid":    WORKSPACE / "tests/fixtures/spec_turn_grid_invalid_template.yaml",
    "time_clock":   WORKSPACE / "tests/fixtures/spec_time_clock_invalid_template.yaml",
    "event_driven": WORKSPACE / "tests/fixtures/spec_event_driven_invalid_template.yaml",
}
NEG_FIXTURES_MISSING = {
    "branching":    WORKSPACE / "tests/fixtures/spec_branching_missing_required.yaml",
    "turn_grid":    WORKSPACE / "tests/fixtures/spec_turn_grid_missing_required.yaml",
    "time_clock":   WORKSPACE / "tests/fixtures/spec_time_clock_missing_required.yaml",
    "event_driven": WORKSPACE / "tests/fixtures/spec_event_driven_missing_required.yaml",
}
NEG_FIXTURES_RANGE = {
    "turn_grid":    WORKSPACE / "tests/fixtures/spec_turn_grid_value_out_of_range.yaml",
    "time_clock":   WORKSPACE / "tests/fixtures/spec_time_clock_value_out_of_range.yaml",
    "event_driven": WORKSPACE / "tests/fixtures/spec_event_driven_value_out_of_range.yaml",
}
NEG_FIXTURES_SCENE_COLLISION = {
    "branching":    WORKSPACE / "tests/fixtures/spec_branching_scene_collision.yaml",
    "turn_grid":    WORKSPACE / "tests/fixtures/spec_turn_grid_scene_collision.yaml",
    "time_clock":   WORKSPACE / "tests/fixtures/spec_time_clock_scene_collision.yaml",
    "event_driven": WORKSPACE / "tests/fixtures/spec_event_driven_scene_collision.yaml",
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

def run_generator(name: str, spec_path: Path, out_dir: str) -> subprocess.CompletedProcess:
    """Generator を 1 回実行して subprocess.CompletedProcess を返す。"""
    gen_path = GENERATORS[name]
    return subprocess.run(
        [PYTHON, str(gen_path), str(spec_path), "--out-dir", out_dir],
        capture_output=True, text=True
    )


def step_generate(
    name: str,
    out_dir: str,
    verbose: bool,
    spec_path: Path | None = None
) -> tuple[bool, str]:
    """Step 1: Generator を spec fixture で呼び出す。
    成功時は (True, 生成 YAML パス) を返す。失敗時は (False, "") を返す。
    """
    if spec_path is None:
        spec_path = FIXTURES[name]

    result = run_generator(name, spec_path, out_dir)

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


def step_expect_generate_failure(
    name: str,
    spec_path: Path,
    out_dir: str,
    require_error_hint: bool,
    verbose: bool
) -> tuple[bool, str]:
    """負ケース: Generator が non-zero で失敗し、YAML を生成しないことを確認する。"""
    result = run_generator(name, spec_path, out_dir)

    if result.returncode == 0:
        return False, "Generator が成功終了した（失敗を期待）"

    yamls = [f for f in glob.glob(f"{out_dir}/*.yaml")]
    if yamls:
        return False, f"失敗ケースで YAML が生成された: {yamls}"

    combined = f"{result.stdout}\n{result.stderr}"
    if require_error_hint:
        has_hint = ("SPEC ERROR" in combined) or ("[V-" in combined) or ("rule" in combined.lower())
        if not has_hint:
            msg = "失敗理由に主要キーワード（SPEC ERROR / [V- / rule）が見つからない"
            if verbose:
                msg += f"\n  STDOUT: {result.stdout.strip()}\n  STDERR: {result.stderr.strip()}"
            return False, msg

    return True, ""


def step_check_review_generated(out_dir: str) -> tuple[bool, str]:
    """review.md 生成を確認する。"""
    reviews = [f for f in glob.glob(f"{out_dir}/*_review.md")]
    if not reviews:
        return False, "review.md が生成されていない"
    return True, ""


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


def run_negative_test(
    name: str,
    case_label: str,
    fixture_path: Path,
    require_error_hint: bool,
    verbose: bool
) -> bool:
    """1 Generator の負ケースを実行する。"""
    label = f"[{name}/{case_label}]"
    print(f"{label:<30}", end=" ", flush=True)

    with tempfile.TemporaryDirectory() as tmpdir:
        ok, info = step_expect_generate_failure(
            name=name,
            spec_path=fixture_path,
            out_dir=tmpdir,
            require_error_hint=require_error_hint,
            verbose=verbose,
        )
        if not ok:
            print(f"FAIL  {info}")
            return False

    print("PASS")
    return True


def run_review_generation_test(name: str, verbose: bool) -> bool:
    """正常系 fixture で review.md 生成を確認する。"""
    label = f"[{name}/review_generation]"
    print(f"{label:<30}", end=" ", flush=True)

    with tempfile.TemporaryDirectory() as tmpdir:
        ok, info = step_generate(name, tmpdir, verbose)
        if not ok:
            print(f"FAIL  step=generate  {info}")
            return False

        ok, info = step_check_review_generated(tmpdir)
        if not ok:
            print(f"FAIL  {info}")
            return False

    print("PASS")
    return True


def run_single_generator_cli_test(target: str, verbose: bool) -> bool:
    """--generator <name> 単体実行で対象 1 件のみ走ることを確認する。"""
    label = f"[cli/--generator={target}]"
    print(f"{label:<30}", end=" ", flush=True)

    result = subprocess.run(
        [PYTHON, str(SCRIPT_PATH), "--generator", target, "--skip-gtc07"],
        capture_output=True, text=True
    )

    if result.returncode != 0:
        msg = f"exit={result.returncode}"
        if verbose:
            msg += f"\n  STDOUT: {result.stdout.strip()}\n  STDERR: {result.stderr.strip()}"
        print(f"FAIL  {msg}")
        return False

    stdout = result.stdout
    if "Generator smoke test  (1 generators)" not in stdout:
        print("FAIL  単体実行の件数表示が 1 generators ではない")
        return False

    for other in GENERATORS.keys():
        if other == target:
            continue
        if f"[{other}]" in stdout or f"[{other}/" in stdout:
            print(f"FAIL  対象外 generator が実行された: {other}")
            return False

    print("PASS")
    return True


# ── エントリポイント ─────────────────────────────────────────────────────────

def main() -> None:
    args = sys.argv[1:]
    verbose = "--verbose" in args or "-v" in args
    skip_gtc07 = "--skip-gtc07" in args

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

    # GTC-01: 正常系
    for name in names:
        results[name] = run_test(name, verbose)

    # GTC-02: template 不一致（non-zero + YAML 未生成）
    negative_template: dict[str, bool] = {}
    for name in names:
        negative_template[name] = run_negative_test(
            name=name,
            case_label="template_mismatch",
            fixture_path=NEG_FIXTURES_TEMPLATE[name],
            require_error_hint=False,
            verbose=verbose,
        )

    # GTC-03: 必須欠落（non-zero + YAML 未生成 + 失敗理由キーワード）
    negative_missing: dict[str, bool] = {}
    for name in names:
        negative_missing[name] = run_negative_test(
            name=name,
            case_label="missing_required",
            fixture_path=NEG_FIXTURES_MISSING[name],
            require_error_hint=True,
            verbose=verbose,
        )

    # GTC-04: 値範囲違反（turn_grid / time_clock / event_driven）
    negative_range: dict[str, bool] = {}
    for name in names:
        fixture_path = NEG_FIXTURES_RANGE.get(name)
        if fixture_path is None:
            continue
        negative_range[name] = run_negative_test(
            name=name,
            case_label="value_out_of_range",
            fixture_path=fixture_path,
            require_error_hint=False,
            verbose=verbose,
        )

    # GTC-05: シーン名衝突（4 generator）
    negative_scene_collision: dict[str, bool] = {}
    for name in names:
        negative_scene_collision[name] = run_negative_test(
            name=name,
            case_label="scene_collision",
            fixture_path=NEG_FIXTURES_SCENE_COLLISION[name],
            require_error_hint=False,
            verbose=verbose,
        )

    # GTC-06: review.md 生成確認（4 generator）
    review_generation: dict[str, bool] = {}
    for name in names:
        review_generation[name] = run_review_generation_test(name, verbose)

    # GTC-07: --generator <name> 単体実行確認
    single_generator_cli: dict[str, bool] = {}
    if not skip_gtc07 and "--generator" not in args:
        for name in GENERATORS.keys():
            single_generator_cli[name] = run_single_generator_cli_test(name, verbose)

    print("-" * 50)
    total = (
        len(results)
        + len(negative_template)
        + len(negative_missing)
        + len(negative_range)
        + len(negative_scene_collision)
        + len(review_generation)
        + len(single_generator_cli)
    )
    passed = (
        sum(results.values())
        + sum(negative_template.values())
        + sum(negative_missing.values())
        + sum(negative_range.values())
        + sum(negative_scene_collision.values())
        + sum(review_generation.values())
        + sum(single_generator_cli.values())
    )
    for name in names:
        s1 = "PASS" if results[name] else "FAIL"
        s2 = "PASS" if negative_template[name] else "FAIL"
        s3 = "PASS" if negative_missing[name] else "FAIL"
        s4 = "PASS" if negative_range.get(name, True) else "FAIL"
        s4 = "N/A" if name not in NEG_FIXTURES_RANGE else s4
        s5 = "PASS" if negative_scene_collision[name] else "FAIL"
        s6 = "PASS" if review_generation[name] else "FAIL"
        s7 = "PASS" if single_generator_cli.get(name, True) else "FAIL"
        s7 = "N/A" if name not in single_generator_cli else s7
        print(
            f"  {name}: "
            f"GTC-01={s1}  GTC-02={s2}  GTC-03={s3}  "
            f"GTC-04={s4}  GTC-05={s5}  GTC-06={s6}  GTC-07={s7}"
        )
    print(f"\n{passed}/{total} checks passed")

    all_ok = (
        all(results.values())
        and all(negative_template.values())
        and all(negative_missing.values())
        and all(negative_range.values())
        and all(negative_scene_collision.values())
        and all(review_generation.values())
        and all(single_generator_cli.values())
    )
    sys.exit(0 if all_ok else 1)


if __name__ == "__main__":
    main()
