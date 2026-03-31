#!/usr/bin/env python3
"""
gen_scenario_branching.py
=========================
Phase 3-C T11 — Branching Basic 専用 Scaffold Generator

Structured Spec YAML（generator_spec_schema.md v1.0 準拠）を受け取り、
scenarios/generated/ 配下に YAML 骨格と review.md を出力する。

Scope
-----
  - テンプレート: branching_basic のみ
  - 入力: 構造化 spec YAML（自由文入力は非対応）
  - 出力: シナリオ YAML 骨格 + review.md
  - pos-0 sacrifice を terminal シーン全てに自動配置
  - __FILL_IN__ プレースホルダーは現仕様では不要（全値がspecから決定可能）
  - 自動修正ループなし
  - src/ への書き込みなし

Human Gate
----------
  HG-1: template 選択（spec 作成前。このスクリプトの外）
  HG-2: 生成 YAML レビュー → review.md の指示に従い実値確認
  HG-3: validate_scenario.py 実行 → exit 0 確認
  HG-4: Godot runtime 煙テスト

Usage
-----
    python3 tools/gen_scenario_branching.py <spec.yaml>

    # 出力先を指定する場合
    python3 tools/gen_scenario_branching.py <spec.yaml> --out-dir scenarios/generated

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
# 定数
# ---------------------------------------------------------------------------

VALID_TEMPLATE = "branching_basic"
VALID_RESULT_VALUES = {"solved", "wrong", "failed", "lost", "timeout"}
SCENE_NAME_RE = re.compile(r"^[a-z0-9_]{1,40}$")
EVIDENCE_ID_RE = re.compile(r"^[a-z0-9_]{1,30}$")
LOCATION_RE = re.compile(r"^[a-z0-9_]{1,40}$")
SCENARIO_NAME_RE = re.compile(r"^[a-z0-9_]{1,40}$")

# ---------------------------------------------------------------------------
# バリデーション（V-01〜V-11 / generator_spec_schema.md セクション 3）
# ---------------------------------------------------------------------------

class SpecError(Exception):
    """Structured Spec のバリデーションエラー"""
    pass


def _require(spec: dict, key: str, rule_id: str) -> object:
    """ネストキー（ドット区切り）を必須チェックして返す"""
    keys = key.split(".")
    obj = spec
    for k in keys:
        if not isinstance(obj, dict) or k not in obj:
            raise SpecError(f"[{rule_id}] 必須フィールド '{key}' がありません")
        obj = obj[k]
    return obj


def validate_spec(spec: dict) -> None:
    """
    Structured Spec を Fail-Fast でバリデーションする。
    エラーがあれば SpecError を送出。
    """
    # V-01: template
    template = spec.get("template", "")
    if template != VALID_TEMPLATE:
        raise SpecError(
            f"[V-01] template は '{VALID_TEMPLATE}' のみ対応しています。"
            f" 受け取った値: '{template}'"
        )

    # V-02: scenario_name
    scenario_name = spec.get("scenario_name", "")
    if not SCENARIO_NAME_RE.match(str(scenario_name)):
        raise SpecError(
            f"[V-02] scenario_name は [a-z0-9_] 1〜40 文字にしてください。"
            f" 受け取った値: '{scenario_name}'"
        )

    # V-03: branches 長
    branches = spec.get("branches")
    if not isinstance(branches, list) or not (2 <= len(branches) <= 4):
        raise SpecError(
            f"[V-03] branches は 2〜4 要素のリストにしてください。"
            f" 現在: {len(branches) if isinstance(branches, list) else '未定義'}"
        )

    # V-04 / V-05: branches[].id
    ids_seen = []
    for i, branch in enumerate(branches):
        if not isinstance(branch, dict):
            raise SpecError(f"[V-04] branches[{i}] はマッピングである必要があります")
        bid = branch.get("id", "")
        if not EVIDENCE_ID_RE.match(str(bid)):
            raise SpecError(
                f"[V-04] branches[{i}].id は [a-z0-9_] 1〜30 文字にしてください。"
                f" 受け取った値: '{bid}'"
            )
        if bid in ids_seen:
            raise SpecError(f"[V-05] branches[].id '{bid}' が重複しています")
        ids_seen.append(bid)

    # V-06: check_type
    check_type = spec.get("check_type", "")
    if check_type not in ("all_of", "any_of"):
        raise SpecError(
            f"[V-06] check_type は 'all_of' または 'any_of' にしてください。"
            f" 受け取った値: '{check_type}'"
        )

    # V-07: scenes.terminal_clear / terminal_fail
    scenes = spec.get("scenes", {})
    if not isinstance(scenes, dict):
        raise SpecError("[V-07] scenes はマッピングである必要があります")
    terminal_clear = scenes.get("terminal_clear", "")
    terminal_fail = scenes.get("terminal_fail", "")
    if not terminal_clear:
        raise SpecError("[V-07] scenes.terminal_clear が指定されていません")
    if not terminal_fail:
        raise SpecError("[V-07] scenes.terminal_fail が指定されていません")

    # V-08: terminal_clear ≠ terminal_fail
    if terminal_clear == terminal_fail:
        raise SpecError(
            f"[V-08] scenes.terminal_clear と scenes.terminal_fail は異なる名前にしてください。"
            f" 両方: '{terminal_clear}'"
        )

    # V-09: start シーンが terminal と異なる
    start_scene = scenes.get("start", "investigation")
    if start_scene == terminal_clear or start_scene == terminal_fail:
        raise SpecError(
            f"[V-09] scenes.start '{start_scene}' を terminal シーンと同じにすることはできません"
        )

    # V-10: terminal_result
    terminal_result = spec.get("terminal_result", {})
    if isinstance(terminal_result, dict):
        for key in ("clear", "fail"):
            val = terminal_result.get(key)
            if val is not None and val not in VALID_RESULT_VALUES:
                raise SpecError(
                    f"[V-10] terminal_result.{key} の有効値: {sorted(VALID_RESULT_VALUES)}。"
                    f" 受け取った値: '{val}'"
                )

    # V-11: branches[].location の形式
    for i, branch in enumerate(branches):
        loc = branch.get("location")
        if loc is not None and not LOCATION_RE.match(str(loc)):
            raise SpecError(
                f"[V-11] branches[{i}].location は [a-z0-9_] 1〜40 文字にしてください。"
                f" 受け取った値: '{loc}'"
            )


# ---------------------------------------------------------------------------
# YAML 生成（セクション 4 マッピング規則に従う）
# ---------------------------------------------------------------------------

def _indent(text: str, spaces: int) -> str:
    pad = " " * spaces
    return "\n".join(pad + line if line.strip() else line for line in text.splitlines())


def generate_yaml(spec: dict) -> str:
    """
    Structured Spec から YAML 骨格文字列を生成する。
    ruamel / yaml.dump は使わず、テンプレート文字列で組み立てる。
    出力形式を期待出力サンプル（branching_basic_expected_output.yaml）と
    一致させるため、手動構築する。
    """
    scenes = spec.get("scenes", {})
    start_scene = scenes.get("start", "investigation")
    terminal_clear = scenes["terminal_clear"]
    terminal_fail = scenes["terminal_fail"]
    check_type = spec["check_type"]

    terminal_result = spec.get("terminal_result", {}) or {}
    result_clear = terminal_result.get("clear", "solved")
    result_fail = terminal_result.get("fail", "wrong")

    branches = spec["branches"]

    # --- goal コメント行 ---
    goal = spec.get("goal") or {}
    header_lines = []
    if goal.get("clear_condition"):
        for line in goal["clear_condition"].strip().splitlines():
            header_lines.append(f"# goal.clear_condition: {line}")
    if goal.get("fail_condition"):
        for line in goal["fail_condition"].strip().splitlines():
            header_lines.append(f"# goal.fail_condition: {line}")
    if header_lines:
        header_lines.append("#")
    header_lines.append("# Generated by tools/gen_scenario_branching.py")
    header_lines.append("# HG-2: シーン構成・Task 列を確認し review.md の指示に従ってください")
    header = "\n".join(header_lines)

    # --- discover_evidence 列 ---
    discover_lines = []
    for branch in branches:
        bid = branch["id"]
        location = branch.get("location") or f"{bid}_location"
        discover_lines.append(
            f"      - action: discover_evidence\n"
            f"        payload:\n"
            f"          evidence_id: {bid}\n"
            f"          location: {location}"
        )
    discover_block = "\n\n".join(discover_lines)

    # --- check_condition の条件リスト ---
    condition_items = "\n".join(
        f"            - evidence: {branch['id']}" for branch in branches
    )

    # --- terminal シーン ---
    def terminal_scene_block(scene_name: str, result: str) -> str:
        return (
            f"  {scene_name}:\n"
            f"    on_enter:\n"
            f"      - action: end_game          # pos-0 sacrifice\n"
            f"        payload: {{result: {result}}}\n"
            f"      - action: end_game\n"
            f"        payload: {{result: {result}}}"
        )

    clear_block = terminal_scene_block(terminal_clear, result_clear)
    fail_block = terminal_scene_block(terminal_fail, result_fail)

    yaml_text = f"""{header}

start_scene: {start_scene}

scenes:

  {start_scene}:
    on_enter:
{discover_block}

      - action: check_condition
        payload:
          {check_type}:
{condition_items}
          if_true:  {terminal_clear}
          if_false: {terminal_fail}

{clear_block}

{fail_block}
"""
    return yaml_text


# ---------------------------------------------------------------------------
# review.md 生成
# ---------------------------------------------------------------------------

def generate_review(spec: dict, yaml_path: Path) -> str:
    """HG-2 向けの review.md を生成する"""
    branches = spec["branches"]
    scenes = spec.get("scenes", {})
    terminal_clear = scenes["terminal_clear"]
    terminal_fail = scenes["terminal_fail"]
    start_scene = scenes.get("start", "investigation")
    check_type = spec["check_type"]
    terminal_result = spec.get("terminal_result", {}) or {}
    result_clear = terminal_result.get("clear", "solved")
    result_fail = terminal_result.get("fail", "wrong")

    branch_table = "\n".join(
        f"| `{b['id']}` | `{b.get('location') or b['id']+'_location'}` | {b.get('label', '（label 未設定）')} |"
        for b in branches
    )

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
        f"| 項目 | 値 |",
        f"|:---|:---|",
        f"| template | `branching_basic` |",
        f"| start_scene | `{start_scene}` |",
        f"| check_type | `{check_type}` |",
        f"| terminal_clear | `{terminal_clear}` → result: `{result_clear}` |",
        f"| terminal_fail | `{terminal_fail}` → result: `{result_fail}` |",
        f"| branches 数 | {len(branches)} |",
        "",
        "## 証拠フラグ一覧",
        "",
        "| evidence_id | location | label |",
        "|:---|:---|:---|",
        branch_table,
        "",
        "## HG-2 確認チェックリスト",
        "",
        "以下を目視確認してください。全て ✅ になったら `validate_scenario.py` へ進んでください。",
        "",
        "- [ ] シーン名 3 つ（start / terminal_clear / terminal_fail）が意図通りか",
        f"- [ ] `{check_type}` の判定方式が正しいか（all_of = 全証拠必須 / any_of = 1 つで OK）",
        "- [ ] 各 `discover_evidence` の `location` が正しいか（自動生成値は `{id}_location`）",
        f"- [ ] terminal_clear の result が `{result_clear}` で正しいか",
        f"- [ ] terminal_fail の result が `{result_fail}` で正しいか",
        "- [ ] terminal シーン 2 つに pos-0 sacrifice（同一 Task × 2）が配置されているか",
        "",
        "## よくある間違い（参照: docs/few_shot_prompts.md Shot 1）",
        "",
        "- `check_evidence` と `check_condition` を混同しない（2 フラグ結合は `check_condition`）",
        "- `all_of` と `any_of` を同時に書かない",
        "- terminal シーンの pos-0 sacrifice を削除しない",
        "",
        "## 次ステップ",
        "",
        "```bash",
        f"python3 tools/validate_scenario.py {yaml_path}",
        "# exit 0 → HG-3 完了 → Godot で runtime 煙テスト（HG-4）",
        "```",
    ]
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# メインエントリ
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Branching Basic Scaffold Generator (Phase 3-C T11)"
    )
    parser.add_argument("spec", help="Structured Spec YAML ファイルのパス")
    parser.add_argument(
        "--out-dir",
        default="scenarios/generated",
        help="出力ディレクトリ（デフォルト: scenarios/generated）",
    )
    args = parser.parse_args()

    spec_path = Path(args.spec)
    if not spec_path.exists():
        print(f"ERROR: spec ファイルが見つかりません: {spec_path}", file=sys.stderr)
        return 2

    # spec 読み込み
    try:
        with open(spec_path, encoding="utf-8") as f:
            spec = yaml.safe_load(f)
    except yaml.YAMLError as e:
        print(f"ERROR: spec の YAML パースに失敗しました:\n{e}", file=sys.stderr)
        return 2

    if not isinstance(spec, dict):
        print("ERROR: spec のトップレベルはマッピングである必要があります", file=sys.stderr)
        return 2

    # バリデーション（Fail-Fast）
    try:
        validate_spec(spec)
    except SpecError as e:
        print(f"SPEC ERROR: {e}", file=sys.stderr)
        print("YAML は生成されませんでした。spec を修正して再実行してください。", file=sys.stderr)
        return 1

    # 出力先ディレクトリ確保
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # タイムスタンプ付きファイル名
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    scenario_name = spec["scenario_name"]
    yaml_filename = f"{scenario_name}_{ts}.yaml"
    review_filename = f"{scenario_name}_{ts}_review.md"

    yaml_path = out_dir / yaml_filename
    review_path = out_dir / review_filename

    # YAML 生成 → 書き出し
    yaml_content = generate_yaml(spec)
    yaml_path.write_text(yaml_content, encoding="utf-8")

    # review.md 生成 → 書き出し
    review_content = generate_review(spec, yaml_path)
    review_path.write_text(review_content, encoding="utf-8")

    # 完了メッセージ
    print(f"Generated: {yaml_path}")
    print(f"Review:    {review_path}")
    print()
    print("--- HG-2 ---")
    print(f"  {review_path} を開いてチェックリストを確認してください。")
    print()
    print("--- HG-3 ---")
    print(f"  python3 tools/validate_scenario.py {yaml_path}")
    print()
    print("※ git add は HG-4（runtime 煙テスト）通過後に手動で行ってください。")

    return 0


if __name__ == "__main__":
    sys.exit(main())
