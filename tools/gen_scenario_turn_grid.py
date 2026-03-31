#!/usr/bin/env python3
"""
gen_scenario_turn_grid.py
=========================
Phase 3-C T12 — Turn/Grid Basic 専用 Scaffold Generator

Structured Spec YAML（generator_spec_schema.md v1.1 T12-1〜T12-8 準拠）を受け取り、
scenarios/generated/ 配下に YAML 骨格と review.md を出力する。

Scope
-----
  - テンプレート: turn_grid_basic のみ
  - 入力: 構造化 spec YAML（自由文入力は非対応）
  - 出力: シナリオ YAML 骨格 + review.md
  - pos-0 sacrifice を terminal シーン（clear / fail）のみに自動配置
  - boot シーンには sacrifice 不要（R-1 HG-4 smoke で確認済み）
  - __FILL_IN__ なし（全値が spec から決定可能）
  - 自動修正ループなし
  - src/ への書き込みなし

Human Gate
----------
  HG-1: template 選択（spec 作成前。このスクリプトの外）
  HG-2: 生成 YAML レビュー → review.md の指示に従い実値確認
  HG-3: validate_scenario.py 実行 → exit 0 確認
  HG-4: Godot runtime 煙テスト（3 経路: clear / fail / continue）

Usage
-----
    python3 tools/gen_scenario_turn_grid.py <spec.yaml>

    # 出力先を指定する場合
    python3 tools/gen_scenario_turn_grid.py <spec.yaml> --out-dir scenarios/generated

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
# 定数（V-TG-01〜V-TG-09 に対応）
# ---------------------------------------------------------------------------

VALID_TEMPLATE = "turn_grid_basic"
VALID_COMMANDS = {"attack", "move_up", "move_down", "move_left", "move_right"}
ENEMY_ID_RE = re.compile(r"^enemy_[1-4]$")
SCENARIO_NAME_RE = re.compile(r"^[a-z0-9_]{1,40}$")
SCENE_NAME_RE = re.compile(r"^[a-z0-9_]{1,40}$")
COORD_RANGE = range(0, 10)  # 0〜9


# ---------------------------------------------------------------------------
# バリデーション（V-TG-01〜V-TG-09 / generator_spec_schema.md T12-3 準拠）
# ---------------------------------------------------------------------------

class SpecError(Exception):
    """Structured Spec のバリデーションエラー"""
    pass


def _get_int_opt(obj: dict, key: str, rule_id: str) -> "int | None":
    """省略可能な整数フィールドを取得する。存在すれば int に変換して返す"""
    val = obj.get(key)
    if val is None:
        return None
    try:
        return int(val)
    except (ValueError, TypeError):
        raise SpecError(f"[{rule_id}] '{key}' は整数である必要があります。受け取った値: '{val}'")


def validate_spec(spec: dict) -> None:
    """
    Structured Spec を Fail-Fast でバリデーションする。
    エラーがあれば SpecError を送出。
    """
    # V-TG-01: template
    template = spec.get("template", "")
    if template != VALID_TEMPLATE:
        raise SpecError(
            f"[V-TG-01] template は '{VALID_TEMPLATE}' のみ対応しています。"
            f" 受け取った値: '{template}'"
        )

    # V-TG-02: scenario_name
    scenario_name = spec.get("scenario_name", "")
    if not SCENARIO_NAME_RE.match(str(scenario_name)):
        raise SpecError(
            f"[V-TG-02] scenario_name は [a-z0-9_] 1〜40 文字にしてください"
            f"（英小文字・数字・アンダースコアのみ）。受け取った値: '{scenario_name}'"
        )

    # player フィールド検証
    player = spec.get("player")
    if not isinstance(player, dict):
        raise SpecError("[V-TG-03] 'player' フィールド（マッピング）が必要です")

    # V-TG-03: player.hp
    try:
        player_hp = int(player.get("hp", None))
    except (TypeError, ValueError):
        raise SpecError("[V-TG-03] player.hp は整数が必要です")
    if not (1 <= player_hp <= 10):
        raise SpecError(f"[V-TG-03] player.hp は 1〜10 の整数にしてください。受け取った値: {player_hp}")

    # player 座標（省略可）
    for coord_key in ("start_x", "start_y"):
        val = _get_int_opt(player, coord_key, "V-TG-09")
        if val is not None and val not in COORD_RANGE:
            raise SpecError(
                f"[V-TG-09] player.{coord_key} は 0〜9 の整数にしてください。受け取った値: {val}"
            )

    # V-TG-04: enemies
    enemies = spec.get("enemies")
    if not isinstance(enemies, list) or not (1 <= len(enemies) <= 4):
        raise SpecError(
            f"[V-TG-04] enemies は 1〜4 要素のリストにしてください。"
            f" 現在: {len(enemies) if isinstance(enemies, list) else '未定義'}"
        )

    # V-TG-05 / V-TG-06 / V-TG-09: enemies[] 各フィールド
    ids_seen = []
    for i, enemy in enumerate(enemies):
        if not isinstance(enemy, dict):
            raise SpecError(f"[V-TG-05] enemies[{i}] はマッピングである必要があります")

        # V-TG-05: id
        eid = enemy.get("id", "")
        if not ENEMY_ID_RE.match(str(eid)):
            raise SpecError(
                f"[V-TG-05] enemies[{i}].id は 'enemy_1'〜'enemy_4' 形式にしてください。"
                f" 受け取った値: '{eid}'"
            )
        if eid in ids_seen:
            raise SpecError(f"[V-TG-05] enemies[].id '{eid}' が重複しています")
        ids_seen.append(eid)

        # V-TG-06: hp
        try:
            enemy_hp = int(enemy.get("hp", None))
        except (TypeError, ValueError):
            raise SpecError(f"[V-TG-06] enemies[{i}].hp は整数が必要です")
        if not (1 <= enemy_hp <= 10):
            raise SpecError(
                f"[V-TG-06] enemies[{i}].hp は 1〜10 の整数にしてください。受け取った値: {enemy_hp}"
            )

        # V-TG-09: 敵座標（省略可）
        for coord_key in ("x", "y"):
            val = _get_int_opt(enemy, coord_key, "V-TG-09")
            if val is not None and val not in COORD_RANGE:
                raise SpecError(
                    f"[V-TG-09] enemies[{i}].{coord_key} は 0〜9 の整数にしてください。"
                    f" 受け取った値: {val}"
                )

    # V-TG-07: first_command
    first_command = spec.get("first_command", "")
    if first_command not in VALID_COMMANDS:
        raise SpecError(
            f"[V-TG-07] first_command は {sorted(VALID_COMMANDS)} のいずれかにしてください。"
            f" 受け取った値: '{first_command}'"
        )

    # V-TG-08: terminal_clear / terminal_fail
    terminal_clear = spec.get("terminal_clear", "")
    terminal_fail = spec.get("terminal_fail", "")
    if not terminal_clear or not SCENE_NAME_RE.match(str(terminal_clear)):
        raise SpecError(
            f"[V-TG-08] terminal_clear は [a-z0-9_] 1〜40 文字にしてください。"
            f" 受け取った値: '{terminal_clear}'"
        )
    if not terminal_fail or not SCENE_NAME_RE.match(str(terminal_fail)):
        raise SpecError(
            f"[V-TG-08] terminal_fail は [a-z0-9_] 1〜40 文字にしてください。"
            f" 受け取った値: '{terminal_fail}'"
        )
    if terminal_clear == terminal_fail:
        raise SpecError(
            f"[V-TG-08] terminal_clear と terminal_fail は異なる名前にしてください。"
            f" 両方: '{terminal_clear}'"
        )
    for name in (terminal_clear, terminal_fail):
        if name == "boot":
            raise SpecError(
                f"[V-TG-08] terminal シーン名に 'boot' は使用できません（ループシーン名と衝突）。"
                f" 受け取った値: '{name}'"
            )


# ---------------------------------------------------------------------------
# YAML 生成（T12-4 マッピング規則に従う）
# ---------------------------------------------------------------------------

def _build_setup_payload(spec: dict) -> list[str]:
    """setup_roguelike_round の payload 行リストを生成する"""
    player = spec["player"]
    enemies = spec["enemies"]  # v1.0: enemies[0] のみ
    enemy0 = enemies[0]

    lines = []
    lines.append(f"          player_hp: {int(player['hp'])}")
    lines.append(f"          enemy_hp:  {int(enemy0['hp'])}")

    # 省略可能フィールド（値が存在する場合のみ出力）
    if player.get("start_x") is not None:
        lines.append(f"          player_x:  {int(player['start_x'])}")
    if player.get("start_y") is not None:
        lines.append(f"          player_y:  {int(player['start_y'])}")
    if enemy0.get("x") is not None:
        lines.append(f"          enemy_1_x: {int(enemy0['x'])}")
    if enemy0.get("y") is not None:
        lines.append(f"          enemy_1_y: {int(enemy0['y'])}")

    return lines


def generate_yaml(spec: dict) -> str:
    """
    Structured Spec から YAML 骨格文字列を生成する。
    ruamel / yaml.dump は使わず、テンプレート文字列で直接組み立てる。
    """
    terminal_clear = spec["terminal_clear"]
    terminal_fail = spec["terminal_fail"]
    first_command = spec["first_command"]

    # --- ヘッダコメント ---
    description = spec.get("description", "")
    header_lines = []
    if description:
        for line in description.strip().splitlines():
            header_lines.append(f"# {line}")
        header_lines.append("#")
    header_lines.append("# Generated by tools/gen_scenario_turn_grid.py")
    header_lines.append("# HG-2: シーン構成・Task 列を確認し review.md の指示に従ってください")
    header = "\n".join(header_lines)

    # --- setup_roguelike_round payload ---
    setup_payload_lines = _build_setup_payload(spec)
    setup_payload_block = "\n".join(setup_payload_lines)

    # --- apply_player_attack または apply_player_move ---
    if first_command == "attack":
        action_block = (
            "      - action: apply_player_attack\n"
            "        payload:\n"
            "          target: enemy_1"
        )
    else:
        # move_* 系: apply_player_move には payload 不要（WorldState 経由）
        action_block = "      - action: apply_player_move"

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

    clear_block = terminal_block(terminal_clear, "solved")
    fail_block = terminal_block(terminal_fail, "failed")

    yaml_text = f"""{header}

start_scene: boot

scenes:

  # boot: ターンループシーン
  # - 初回(_ready()経由): setup_roguelike_round が pos-0 から実行される
  # - ループ時: setup_roguelike_round(pos-0)はスキップ → 前ターンの状態が保持される
  # pos-0 sacrifice は不要（R-1 HG-4 smoke 確認済み）
  boot:
    on_enter:
      - action: setup_roguelike_round
        payload:
{setup_payload_block}

      - action: load_fake_player_command
        payload:
          command: {first_command}

{action_block}

      - action: apply_enemy_turn

      - action: resolve_roguelike_turn

      - action: evaluate_roguelike_round
        payload:
          if_clear:    {terminal_clear}
          if_fail:     {terminal_fail}
          if_continue: boot

{clear_block}

{fail_block}
"""
    return yaml_text


# ---------------------------------------------------------------------------
# review.md 生成（HG-2 用チェックリスト）
# ---------------------------------------------------------------------------

def generate_review(spec: dict, yaml_path: Path) -> str:
    """HG-2 向けの review.md を生成する"""
    terminal_clear = spec["terminal_clear"]
    terminal_fail = spec["terminal_fail"]
    first_command = spec["first_command"]
    player = spec["player"]
    enemies = spec["enemies"]

    # 座標サマリ
    def fmt_coord(obj: dict, x_key: str, y_key: str) -> str:
        x = obj.get(x_key)
        y = obj.get(y_key)
        if x is None and y is None:
            return "（デフォルト）"
        return f"({x if x is not None else '?'}, {y if y is not None else '?'})"

    player_pos = fmt_coord(player, "start_x", "start_y")
    enemy_rows = "\n".join(
        f"| `{e['id']}` | {e['hp']} | {fmt_coord(e, 'x', 'y')} |"
        for e in enemies
    )

    # attack / move 分岐説明
    if first_command == "attack":
        action_note = "`apply_player_attack {target: enemy_1}` が生成されます"
    else:
        action_note = f"`apply_player_move`（payload なし）が生成されます。方向は WorldState の `last_action:type={first_command}` 経由で渡されます"

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
        f"| template | `turn_grid_basic` |",
        f"| start_scene | `boot` （ループシーン） |",
        f"| terminal_clear | `{terminal_clear}` → result: `solved` |",
        f"| terminal_fail | `{terminal_fail}` → result: `failed` |",
        f"| first_command | `{first_command}` |",
        f"| player.hp | {int(player['hp'])} |",
        f"| player 初期位置 | {player_pos} |",
        f"| enemies 数 | {len(enemies)} |",
        "",
        "## Player 情報",
        "",
        f"- HP: **{int(player['hp'])}**",
        f"- 初期位置: **{player_pos}**",
        "",
        "## Enemy 一覧",
        "",
        "| id | hp | 初期位置 |",
        "|:---|:---:|:---|",
        enemy_rows,
        "",
        "## first_command の処理",
        "",
        f"- `first_command: {first_command}` →  {action_note}",
        "",
        "## ターンループ仕様",
        "",
        "```",
        "boot (初回): setup_roguelike_round[pos-0] が実行される → 状態初期化",
        "boot (ループ): setup_roguelike_round[pos-0] はスキップ → 状態保持",
        "   ↑ pos-0 skip は known issue の仕様内動作（R-1 HG-4 smoke 確認済み）",
        "```",
        "",
        "## HG-2 確認チェックリスト",
        "",
        "以下を目視確認してください。全て ✅ になったら `validate_scenario.py` へ進んでください。",
        "",
        "- [ ] `boot` シーンの Task 列が意図通りか（setup → load_cmd → action → enemy_turn → resolve → evaluate）",
        "- [ ] `first_command` が期待する行動に対応しているか",
        f"- [ ] terminal_clear `{terminal_clear}` が勝利シーンとして正しいか",
        f"- [ ] terminal_fail `{terminal_fail}` が敗北シーンとして正しいか",
        "- [ ] terminal 2 シーンに pos-0 sacrifice（同一 Task × 2）が配置されているか",
        "- [ ] `if_continue: boot` がループ設定として正しいか",
        "",
        "## ループ動作の注意点",
        "",
        "> **pos-0 skip の設計内動作**:  ",
        "> `evaluate_roguelike_round` が `if_continue: boot` でループするとき、  ",
        "> `setup_roguelike_round`（pos-0）はスキップされ WorldState の状態（player 座標・HP 等）が保持されます。  ",
        "> これは意図された動作です。`boot` に pos-0 sacrifice スロットを追加しないでください。",
        "",
        "## よくある間違い（参照: docs/few_shot_prompts.md Shot 2）",
        "",
        "- `apply_player_move` に payload を追加しない（方向は WorldState 経由）",
        "- `boot` シーンに pos-0 sacrifice を配置しない（初回の setup が壊れる）",
        "- `evaluate_roguelike_round` の `if_continue` を `boot` 以外に変更しない",
        "",
        "## 次ステップ",
        "",
        "```bash",
        f"python3 tools/validate_scenario.py {yaml_path}",
        "# exit 0 → HG-3 完了 → Godot で runtime 煙テスト（HG-4）: clear / fail / continue 3 経路",
        "```",
    ]
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# メインエントリ
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Turn/Grid Basic Scaffold Generator (Phase 3-C T12)"
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
    print("※ git add は HG-4（runtime 煙テスト: clear / fail / continue 3 経路）通過後に手動で行ってください。")

    return 0


if __name__ == "__main__":
    sys.exit(main())
