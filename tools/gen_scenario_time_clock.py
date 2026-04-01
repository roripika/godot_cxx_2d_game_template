#!/usr/bin/env python3
"""
gen_scenario_time_clock.py
==========================
Phase 3-C T13 — Time/Clock Basic 専用 Scaffold Generator

Structured Spec YAML（generator_spec_schema.md T13-1〜T13-8 準拠）を受け取り、
scenarios/generated/ 配下に YAML 骨格と review.md を出力する。

Scope
-----
  - テンプレート: time_clock_basic のみ
  - 入力: 構造化 spec YAML（自由文入力は非対応）
  - 出力: シナリオ YAML 骨格 + review.md
  - boot: pos-0 sacrifice 不要（_ready() 起動 → setup_rhythm_round が pos-0 から実行）
  - 中間シーン(advance/judge/resolve/loop_gate): pos-0 sacrifice 必要（×2 自動配置）
  - terminal シーン: pos-0 sacrifice 必要（×2 自動配置）
    → R-5/R-6/R-7: headless HG-4 smoke で確認済み（2026-04-01）
  - __FILL_IN__ なし（全値が spec から決定可能）
  - 自動修正ループなし
  - src/ への書き込みなし

Human Gate
----------
  HG-1: template 選択（spec 作成前。このスクリプトの外）
  HG-2: 生成 YAML レビュー → review.md の指示に従い実値確認
  HG-3: validate_scenario.py 実行 → exit 0 確認
  HG-4: Godot runtime 煙テスト（3 経路: clear / fail / continue loop）

Usage
-----
    python3 tools/gen_scenario_time_clock.py <spec.yaml>

    # 出力先を指定する場合
    python3 tools/gen_scenario_time_clock.py <spec.yaml> --out-dir scenarios/generated

Exit codes
----------
  0  生成成功
  1  spec バリデーションエラー（YAML 未生成）
  2  入力ファイルエラー（存在しない等）
"""

import sys
import re
import argparse
from datetime import datetime
from pathlib import Path

try:
    import yaml
except ImportError:
    print("ERROR: PyYAML is required.  Run:  pip install pyyaml", file=sys.stderr)
    sys.exit(2)

# ---------------------------------------------------------------------------
# 定数（V-TC-01〜V-TC-12 に対応）
# ---------------------------------------------------------------------------

VALID_TEMPLATE = "time_clock_basic"
SCENARIO_NAME_RE = re.compile(r"^[a-z0-9_]{1,40}$")
SCENE_NAME_RE = re.compile(r"^[a-z0-9_]{1,40}$")

# 予約済み内部シーン名（T13-2 T13-2-3 制約）
RESERVED_SCENE_NAMES = {"boot", "advance", "judge", "resolve", "loop_gate"}

# 有効な end_game result 値
VALID_RESULTS = {"solved", "wrong", "failed", "lost", "timeout"}

# ノーツ数の制約
NOTES_MIN = 3
NOTES_MAX = 5


# ---------------------------------------------------------------------------
# バリデーション（V-TC-01〜V-TC-12 / generator_spec_schema.md T13-3 準拠）
# ---------------------------------------------------------------------------

class SpecError(Exception):
    """Structured Spec のバリデーションエラー"""
    pass


def validate_spec(spec: dict) -> None:
    """
    Structured Spec を Fail-Fast でバリデーションする。
    エラーがあれば SpecError を送出。
    """
    # V-TC-01: template
    template = spec.get("template", "")
    if template != VALID_TEMPLATE:
        raise SpecError(
            f"[V-TC-01] template は '{VALID_TEMPLATE}' のみ対応しています。"
            f" 受け取った値: '{template}'"
        )

    # V-TC-02: scenario_name
    scenario_name = spec.get("scenario_name", "")
    if not SCENARIO_NAME_RE.match(str(scenario_name)):
        raise SpecError(
            f"[V-TC-02] scenario_name は [a-z0-9_] 1〜40 文字にしてください。"
            f" 受け取った値: '{scenario_name}'"
        )

    # V-TC-03: notes
    notes = spec.get("notes")
    if not isinstance(notes, list) or not (NOTES_MIN <= len(notes) <= NOTES_MAX):
        raise SpecError(
            f"[V-TC-03] notes は {NOTES_MIN}〜{NOTES_MAX} 要素のリストにしてください。"
            f" 現在: {len(notes) if isinstance(notes, list) else '未定義'}"
        )
    prev_time = -1
    for i, nt in enumerate(notes):
        try:
            nt_int = int(nt)
        except (TypeError, ValueError):
            raise SpecError(f"[V-TC-03] notes[{i}] は整数が必要です。受け取った値: '{nt}'")
        if nt_int <= 0:
            raise SpecError(
                f"[V-TC-03] notes[{i}] は正整数（> 0）にしてください。受け取った値: {nt_int}"
            )
        if nt_int <= prev_time:
            raise SpecError(
                f"[V-TC-03] notes は厳密昇順にしてください。"
                f" notes[{i-1}]={prev_time} >= notes[{i}]={nt_int}"
            )
        prev_time = nt_int

    # V-TC-04: taps
    taps = spec.get("taps")
    if not isinstance(taps, list):
        raise SpecError("[V-TC-04] taps はリストが必要です")
    if len(taps) != len(notes):
        raise SpecError(
            f"[V-TC-04] taps の要素数は notes と同数にしてください。"
            f" notes={len(notes)}, taps={len(taps)}"
        )
    for i, tap in enumerate(taps):
        try:
            tap_int = int(tap)
        except (TypeError, ValueError):
            raise SpecError(f"[V-TC-04] taps[{i}] は整数が必要です。受け取った値: '{tap}'")
        if tap_int < -1:
            raise SpecError(
                f"[V-TC-04] taps[{i}] は -1 以上にしてください（-1 = タップなし）。"
                f" 受け取った値: {tap_int}"
            )

    # V-TC-05: advance_ms（省略可）
    advance_ms = spec.get("advance_ms")
    if advance_ms is not None:
        try:
            advance_ms_int = int(advance_ms)
        except (TypeError, ValueError):
            raise SpecError(f"[V-TC-05] advance_ms は整数が必要です。受け取った値: '{advance_ms}'")
        if advance_ms_int <= 0:
            raise SpecError(
                f"[V-TC-05] advance_ms は正整数（> 0）にしてください。受け取った値: {advance_ms_int}"
            )

    # V-TC-06: clear_hit_count（省略可）
    clear_hit_count = spec.get("clear_hit_count")
    if clear_hit_count is not None:
        try:
            chc = int(clear_hit_count)
        except (TypeError, ValueError):
            raise SpecError(
                f"[V-TC-06] clear_hit_count は整数が必要です。受け取った値: '{clear_hit_count}'"
            )
        if not (1 <= chc <= len(notes)):
            raise SpecError(
                f"[V-TC-06] clear_hit_count は 1〜{len(notes)} の範囲にしてください。"
                f" 受け取った値: {chc}"
            )

    # V-TC-07: max_miss_count（省略可）
    max_miss_count = spec.get("max_miss_count")
    if max_miss_count is not None:
        try:
            mmc = int(max_miss_count)
        except (TypeError, ValueError):
            raise SpecError(
                f"[V-TC-07] max_miss_count は整数が必要です。受け取った値: '{max_miss_count}'"
            )
        if mmc < 0:
            raise SpecError(
                f"[V-TC-07] max_miss_count は 0 以上にしてください。受け取った値: {mmc}"
            )

    # V-TC-11 / V-TC-12: timing windows（省略可）
    perfect_ms = spec.get("perfect_window_ms")
    good_ms = spec.get("good_window_ms")
    if perfect_ms is not None:
        try:
            p = int(perfect_ms)
        except (TypeError, ValueError):
            raise SpecError(
                f"[V-TC-11] perfect_window_ms は整数が必要です。受け取った値: '{perfect_ms}'"
            )
        if p <= 0:
            raise SpecError(
                f"[V-TC-11] perfect_window_ms は正整数（> 0）にしてください。受け取った値: {p}"
            )
    if good_ms is not None:
        try:
            g = int(good_ms)
        except (TypeError, ValueError):
            raise SpecError(
                f"[V-TC-12] good_window_ms は整数が必要です。受け取った値: '{good_ms}'"
            )
        if perfect_ms is not None and g < int(perfect_ms):
            raise SpecError(
                f"[V-TC-12] good_window_ms は perfect_window_ms 以上にしてください。"
                f" perfect={int(perfect_ms)}, good={g}"
            )

    # V-TC-08 / V-TC-09 / V-TC-10: シーン名・result 値
    scenes = spec.get("scenes", {})
    if not isinstance(scenes, dict):
        raise SpecError("[V-TC-08] 'scenes' フィールド（マッピング）が必要です")

    terminal_clear = scenes.get("terminal_clear", "")
    terminal_fail = scenes.get("terminal_fail", "")

    if not terminal_clear or not SCENE_NAME_RE.match(str(terminal_clear)):
        raise SpecError(
            f"[V-TC-08] scenes.terminal_clear は [a-z0-9_] 1〜40 文字にしてください。"
            f" 受け取った値: '{terminal_clear}'"
        )
    if not terminal_fail or not SCENE_NAME_RE.match(str(terminal_fail)):
        raise SpecError(
            f"[V-TC-08] scenes.terminal_fail は [a-z0-9_] 1〜40 文字にしてください。"
            f" 受け取った値: '{terminal_fail}'"
        )
    if terminal_clear == terminal_fail:
        raise SpecError(
            f"[V-TC-08] terminal_clear と terminal_fail は異なる名前にしてください。"
            f" 両方: '{terminal_clear}'"
        )
    for name, field in ((terminal_clear, "terminal_clear"), (terminal_fail, "terminal_fail")):
        if name in RESERVED_SCENE_NAMES:
            raise SpecError(
                f"[V-TC-09] scenes.{field} に予約済みシーン名 '{name}' は使用できません。"
                f" 予約名: {sorted(RESERVED_SCENE_NAMES)}"
            )

    # V-TC-10: terminal_result
    terminal_result = spec.get("terminal_result", {})
    if not isinstance(terminal_result, dict):
        raise SpecError("[V-TC-10] 'terminal_result' フィールドはマッピングにしてください")
    for key in ("clear", "fail"):
        val = terminal_result.get(key)
        if val is not None and str(val) not in VALID_RESULTS:
            raise SpecError(
                f"[V-TC-10] terminal_result.{key} の有効値は {sorted(VALID_RESULTS)} です。"
                f" 受け取った値: '{val}'"
            )


# ---------------------------------------------------------------------------
# YAML 生成（T13-4 マッピング規則に従う）
# ---------------------------------------------------------------------------

def _format_list_inline(values: list) -> str:
    """Python list を YAML インライン形式 [a, b, c] に変換する"""
    return "[" + ", ".join(str(v) for v in values) + "]"


def _build_setup_payload(spec: dict) -> list[str]:
    """setup_rhythm_round の payload 行リストを生成する（省略可能フィールドは値あり時のみ）"""
    notes = spec["notes"]
    taps = spec["taps"]
    lines = []
    lines.append(f"          notes: {_format_list_inline(notes)}")
    lines.append(f"          taps:  {_format_list_inline(taps)}")

    for key, label in (
        ("advance_ms",       "advance_ms"),
        ("perfect_window_ms","perfect_window_ms"),
        ("good_window_ms",   "good_window_ms"),
        ("clear_hit_count",  "clear_hit_count"),
        ("max_miss_count",   "max_miss_count"),
    ):
        val = spec.get(key)
        if val is not None:
            lines.append(f"          {label}: {int(val)}")
    return lines


def generate_yaml(spec: dict) -> str:
    """
    Structured Spec から YAML 骨格文字列を生成する。
    ruamel / yaml.dump は使わず、テンプレート文字列で直接組み立てる。
    """
    scenes = spec.get("scenes", {})
    terminal_clear = scenes["terminal_clear"]
    terminal_fail = scenes["terminal_fail"]

    terminal_result = spec.get("terminal_result", {})
    result_clear = terminal_result.get("clear", "solved")
    result_fail = terminal_result.get("fail", "failed")

    # --- ヘッダコメント ---
    description = spec.get("description", "")
    header_lines = []
    if description:
        for line in description.strip().splitlines():
            header_lines.append(f"# {line}")
        header_lines.append("#")
    header_lines.append("# Generated by tools/gen_scenario_time_clock.py")
    header_lines.append("# HG-2: シーン構成・Task 列を確認し review.md の指示に従ってください")
    header = "\n".join(header_lines)

    # --- setup_rhythm_round payload ---
    setup_payload_lines = _build_setup_payload(spec)
    setup_payload_block = "\n".join(setup_payload_lines)

    # --- terminal シーン（pos-0 sacrifice あり）---
    def terminal_block(scene_name: str, result: str) -> str:
        return (
            f"  {scene_name}:\n"
            f"    on_enter:\n"
            f"      - action: end_game          # pos-0 sacrifice\n"
            f"        payload: {{result: {result}}}\n"
            f"      - action: end_game\n"
            f"        payload: {{result: {result}}}"
        )

    clear_block = terminal_block(terminal_clear, result_clear)
    fail_block = terminal_block(terminal_fail, result_fail)

    yaml_text = f"""{header}

start_scene: boot

scenes:

  # boot: 初期化シーン
  # - _ready() 経由: setup_rhythm_round が pos-0 から実行される（sacrifice 不要）
  # - setup_rhythm_round が clock を 0 にリセットし、全 WorldState を初期化する
  boot:
    on_enter:
      - action: setup_rhythm_round
        payload:
{setup_payload_block}
      - action: evaluate_rhythm_round
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: advance

  # advance: クロック進行シーン（pos-0 sacrifice 必要）
  advance:
    on_enter:
      - action: advance_rhythm_clock  # pos-0 sacrifice（スキップされる）
      - action: advance_rhythm_clock  # 実際に実行される
      - action: evaluate_rhythm_round
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: judge

  # judge: タップ入力読み取り + ノーツ判定シーン（pos-0 sacrifice 必要）
  judge:
    on_enter:
      - action: load_fake_tap        # pos-0 sacrifice
      - action: load_fake_tap        # 実際に実行される
      - action: judge_rhythm_note
      - action: evaluate_rhythm_round
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: resolve

  # resolve: 判定結果を集計し chart:index を進めるシーン（pos-0 sacrifice 必要）
  resolve:
    on_enter:
      - action: resolve_rhythm_progress  # pos-0 sacrifice
      - action: resolve_rhythm_progress  # 実際に実行される
      - action: evaluate_rhythm_round
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: loop_gate

  # loop_gate: ノーツ間の継続判定ゲート（pos-0 sacrifice 必要）
  loop_gate:
    on_enter:
      - action: evaluate_rhythm_round  # pos-0 sacrifice
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: advance
      - action: evaluate_rhythm_round  # 実際に実行される
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: advance

{clear_block}

{fail_block}
"""
    return yaml_text


# ---------------------------------------------------------------------------
# review.md 生成（HG-2 用チェックリスト）
# ---------------------------------------------------------------------------

def generate_review(spec: dict, yaml_path: Path) -> str:
    """HG-2 向けの review.md を生成する"""
    scenes = spec.get("scenes", {})
    terminal_clear = scenes["terminal_clear"]
    terminal_fail = scenes["terminal_fail"]
    terminal_result = spec.get("terminal_result", {})
    result_clear = terminal_result.get("clear", "solved")
    result_fail = terminal_result.get("fail", "failed")

    notes = spec["notes"]
    taps = spec["taps"]
    advance_ms = spec.get("advance_ms", 1000)
    clear_hit_count = spec.get("clear_hit_count", len(notes))
    max_miss_count = spec.get("max_miss_count", 1)
    perfect_window_ms = spec.get("perfect_window_ms", 50)
    good_window_ms = spec.get("good_window_ms", 150)

    # 各ノーツのタイミング分析
    note_rows = []
    for i, (nt, tap) in enumerate(zip(notes, taps)):
        tap_str = str(tap) if tap != -1 else "-1（タップなし）"
        diff = abs(tap - nt) if tap != -1 else "—"
        if tap == -1:
            judge_pred = "miss（タップなし → good_window 超過後 miss）"
        elif isinstance(diff, int) and diff <= int(perfect_window_ms):
            judge_pred = f"perfect（差分 {diff}ms ≤ {perfect_window_ms}ms）"
        elif isinstance(diff, int) and diff <= int(good_window_ms):
            judge_pred = f"good（差分 {diff}ms ≤ {good_window_ms}ms）"
        else:
            judge_pred = f"miss（差分 {diff}ms > {good_window_ms}ms）"
        note_rows.append(f"| {i} | {nt} | {tap_str} | {judge_pred} |")

    note_table = "\n".join(note_rows)

    lines = [
        f"# HG-2 Review — {spec['scenario_name']}",
        "",
        f"**生成日時**: {datetime.now().strftime('%Y-%m-%d %H:%M')}  ",
        f"**生成元**: `{yaml_path.name}`  ",
        f"**次ステップ**: `python3 tools/validate_scenario.py {yaml_path}` を実行して exit 0 を確認してください",
        "",
        "---",
        "",
        "## 生成内容サマリ",
        "",
        "| 項目 | 値 |",
        "|:---|:---|",
        f"| template | `time_clock_basic` |",
        f"| start_scene | `boot` |",
        f"| ノーツ数 | {len(notes)} |",
        f"| advance_ms | {advance_ms} ms |",
        f"| perfect_window_ms | {perfect_window_ms} ms |",
        f"| good_window_ms | {good_window_ms} ms |",
        f"| clear_hit_count | {clear_hit_count} |",
        f"| max_miss_count | {max_miss_count} |",
        f"| terminal_clear | `{terminal_clear}` → result: `{result_clear}` |",
        f"| terminal_fail | `{terminal_fail}` → result: `{result_fail}` |",
        "",
        "## ノーツ・タップ タイミング解析",
        "",
        "⚠️ HG-2 確認事項: 各ノーツが `advance_ms` の倍数に合致しているか確認してください",
        f"（advance_ms={advance_ms}, ノーツが `advance_ms * n` に近いほど判定が安定します）",
        "",
        "| # | note_time (ms) | tap_time (ms) | 予想判定 |",
        "|:---:|:---:|:---:|:---|",
        note_table,
        "",
        "## シーン構成（T13-4 マッピング準拠）",
        "",
        "```",
        "boot           : setup_rhythm_round + evaluate_rhythm_round",
        "                 ↑ sacrifice 不要（_ready() 起動）",
        "advance        : advance_rhythm_clock ×2（sacrifice + real）",
        "judge          : load_fake_tap ×2（sacrifice + real）+ judge_rhythm_note",
        "resolve        : resolve_rhythm_progress ×2（sacrifice + real）",
        "loop_gate      : evaluate_rhythm_round ×2（sacrifice + real）",
        f"{terminal_clear:<15}: end_game ×2（sacrifice + real）→ result: {result_clear}",
        f"{terminal_fail:<15}: end_game ×2（sacrifice + real）→ result: {result_fail}",
        "```",
        "",
        "## ループフロー",
        "",
        "```",
        "boot → advance → judge → resolve → loop_gate ─┐",
        "                                               │（if_continue → advance）",
        "               ↑───────────────────────────────┘",
        "     ↓ if_clear（いずれかのシーンから）",
        f"     {terminal_clear}",
        "     ↓ if_fail（いずれかのシーンから）",
        f"     {terminal_fail}",
        "```",
        "",
        "## HG-4 実行用コマンド（validate 後に実行）",
        "",
        "```bash",
        "# 1. validate",
        f"python3 tools/validate_scenario.py {yaml_path}",
        "",
        "# 2. headless smoke（Godot シーンファイルが必要。samples/rhythm_test/ 配下を参照）",
        "# /Applications/Godot.app/Contents/MacOS/Godot --headless --path . \\",
        "#   samples/rhythm_test/rhythm_debug_r5_clear.tscn",
        "```",
        "",
        "## 既知制限（v1.0 スコープ外）",
        "",
        "- 複数コマンドシーケンス（ノーツごとに異なるタップタイミング）は自動生成非対応",
        "  → HG-2 で手動編集して `chart:tap_N:time_ms` を設定してください",
        "- `perfect_window_ms` / `good_window_ms` の動的変更は非対応",
        "- ノーツ数 > 5、または曲の途中でシーン遷移するケースは非対応",
    ]
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# エントリポイント
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Time/Clock Basic Scenario Generator (Phase 3-C T13)"
    )
    parser.add_argument("spec", help="入力 Structured Spec YAML ファイルのパス")
    parser.add_argument(
        "--out-dir",
        default="scenarios/generated",
        help="出力先ディレクトリ（デフォルト: scenarios/generated）",
    )
    args = parser.parse_args()

    spec_path = Path(args.spec)
    if not spec_path.exists():
        print(f"SPEC ERROR: ファイルが見つかりません: {spec_path}", file=sys.stderr)
        sys.exit(2)

    try:
        with spec_path.open("r", encoding="utf-8") as f:
            spec = yaml.safe_load(f)
    except yaml.YAMLError as e:
        print(f"SPEC ERROR: YAML パースエラー: {e}", file=sys.stderr)
        sys.exit(2)

    if not isinstance(spec, dict):
        print("SPEC ERROR: spec は YAML マッピング（辞書）である必要があります", file=sys.stderr)
        sys.exit(1)

    # --- バリデーション ---
    try:
        validate_spec(spec)
    except SpecError as e:
        print(f"SPEC ERROR: {e}", file=sys.stderr)
        sys.exit(1)

    # --- 出力先ディレクトリ作成 ---
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # --- ファイル名生成 ---
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    scenario_name = spec["scenario_name"]
    yaml_filename = f"{scenario_name}_{timestamp}.yaml"
    review_filename = f"{scenario_name}_{timestamp}_review.md"

    yaml_path = out_dir / yaml_filename
    review_path = out_dir / review_filename

    # --- YAML 生成・書き込み ---
    yaml_content = generate_yaml(spec)
    yaml_path.write_text(yaml_content, encoding="utf-8")

    # --- review.md 生成・書き込み ---
    review_content = generate_review(spec, yaml_path)
    review_path.write_text(review_content, encoding="utf-8")

    print(f"Generated: {yaml_path}")
    print(f"Review:    {review_path}")
    print(f"Next: python3 tools/validate_scenario.py {yaml_path}")


if __name__ == "__main__":
    main()
