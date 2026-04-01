#!/usr/bin/env python3
"""
gen_scenario_event_driven.py
============================
Phase 3-C T14 — Event-Driven Basic 専用 Scaffold Generator

Structured Spec YAML（generator_spec_schema.md T14-1〜T14-8 準拠）を受け取り、
scenarios/generated/ 配下に YAML 骨格と review.md を出力する。

Scope
-----
  - テンプレート: event_driven_basic のみ
  - 入力: 構造化 spec YAML（自由文入力は非対応）
  - 出力: シナリオ YAML 骨格 + review.md
  - boot(setup_round): pos-0 sacrifice 不要（_ready() 起動）
  - continue シーン:   pos-0 sacrifice 必要（wait_for_billiards_event × 2 自動配置）
  - terminal シーン:   pos-0 sacrifice 必要（end_game × 2 自動配置）
    → R-8/R-9/R-10: headless HG-4 smoke で確認済み（2026-04-01）
  - src/ への書き込みなし

Human Gate
----------
  HG-1: template 選択（spec 作成前。このスクリプトの外）
  HG-2: 生成 YAML レビュー → review.md の指示に従い実値確認
  HG-3: validate_scenario.py 実行 → exit 0 確認
  HG-4: Godot runtime 煙テスト（3 経路: clear / fail / continue loop）

Usage
-----
    python3 tools/gen_scenario_event_driven.py <spec.yaml>

    # 出力先を指定する場合
    python3 tools/gen_scenario_event_driven.py <spec.yaml> --out-dir scenarios/generated

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
except ImportError:  # pragma: no cover
    print("ERROR: PyYAML が必要です。pip install pyyaml を実行してください。")
    sys.exit(2)

# ──────────────────────────────────────────────────────────────────────────────
# 定数
# ──────────────────────────────────────────────────────────────────────────────

VALID_RESULTS = {"solved", "wrong", "failed", "lost", "timeout"}
VALID_EVENTS  = {"shot_committed", "ball_pocketed", "cue_ball_pocketed", "balls_stopped"}

# boot シーン名は常に固定
BOOT_SCENE_NAME = "setup_round"

# 予約済みシーン名（scenes.* に使用不可）
RESERVED_SCENE_NAMES = {BOOT_SCENE_NAME}

IDENTIFIER_RE = re.compile(r"^[a-z0-9_]+$")

# ──────────────────────────────────────────────────────────────────────────────
# バリデーション（Fail-Fast）
# ──────────────────────────────────────────────────────────────────────────────

def _err(rule_id: str, msg: str) -> None:
    print(f"SPEC ERROR: [{rule_id}] {msg}", file=sys.stderr)
    sys.exit(1)


def validate_spec(spec: dict) -> None:
    """V-ED-01〜V-ED-12 を全チェック（問題があれば即 exit(1)）。"""

    # V-ED-01: template
    if spec.get("template") != "event_driven_basic":
        _err("V-ED-01", "template は 'event_driven_basic' のみ対応しています。"
             f" 実際の値: {spec.get('template')!r}")

    # V-ED-02: scenario_name
    sname = spec.get("scenario_name", "")
    if not sname or not IDENTIFIER_RE.match(str(sname)):
        _err("V-ED-02", f"scenario_name '{sname}' は英数字・アンダースコアのみ・1文字以上にしてください。")
    if len(str(sname)) > 40:
        _err("V-ED-02", f"scenario_name '{sname}' は 40 文字以内にしてください。")

    # setup フィールド
    setup = spec.get("setup", {})
    if not isinstance(setup, dict):
        _err("V-ED-03", "'setup' は辞書型である必要があります。")

    # V-ED-03: setup.shot_limit
    shot_limit = setup.get("shot_limit")
    if shot_limit is None:
        _err("V-ED-03", "'setup.shot_limit' は必須です。")
    if not isinstance(shot_limit, int) or not (1 <= shot_limit <= 10):
        _err("V-ED-03", f"setup.shot_limit は 1〜10 の整数にしてください。実際の値: {shot_limit!r}")

    # V-ED-04: setup.target_count
    target_count = setup.get("target_count")
    if target_count is None:
        _err("V-ED-04", "'setup.target_count' は必須です。")
    if not isinstance(target_count, int) or not (1 <= target_count <= 2):
        _err("V-ED-04", f"setup.target_count は 1〜2 の整数にしてください。実際の値: {target_count!r}")

    # V-ED-05: wait_events
    wait_events = spec.get("wait_events", ["balls_stopped"])
    if not isinstance(wait_events, list) or len(wait_events) == 0:
        _err("V-ED-05", "'wait_events' は非空リストにしてください。")
    for ev in wait_events:
        if ev not in VALID_EVENTS:
            _err("V-ED-05", f"wait_events の値 '{ev}' は無効です。有効値: {sorted(VALID_EVENTS)}")

    # V-ED-06: wait_timeout
    wait_timeout = spec.get("wait_timeout", 0.1)
    try:
        wait_timeout_f = float(wait_timeout)
    except (TypeError, ValueError):
        _err("V-ED-06", f"wait_timeout は浮動小数 > 0.0 にしてください。実際の値: {wait_timeout!r}")
    if wait_timeout_f <= 0.0:
        _err("V-ED-06", f"wait_timeout は 0.0 より大きい値にしてください。実際の値: {wait_timeout!r}")

    # V-ED-07: boot_records
    boot_records = spec.get("boot_records", [])
    if not isinstance(boot_records, list):
        _err("V-ED-07", "'boot_records' はリストにしてください。")
    for ev in boot_records:
        if ev not in VALID_EVENTS:
            _err("V-ED-07", f"boot_records の値 '{ev}' は無効です。有効値: {sorted(VALID_EVENTS)}")

    # V-ED-08: continue_records
    continue_records = spec.get("continue_records", [])
    if not isinstance(continue_records, list):
        _err("V-ED-08", "'continue_records' はリストにしてください。")
    for ev in continue_records:
        if ev not in VALID_EVENTS:
            _err("V-ED-08", f"continue_records の値 '{ev}' は無効です。有効値: {sorted(VALID_EVENTS)}")

    # scenes フィールド
    scenes_cfg = spec.get("scenes", {})
    if not isinstance(scenes_cfg, dict):
        _err("V-ED-09", "'scenes' は辞書型である必要があります。")

    terminal_clear = scenes_cfg.get("terminal_clear", "")
    terminal_fail  = scenes_cfg.get("terminal_fail", "")

    # V-ED-09: terminal_clear / terminal_fail
    if not terminal_clear:
        _err("V-ED-09", "'scenes.terminal_clear' は必須です。")
    if not terminal_fail:
        _err("V-ED-09", "'scenes.terminal_fail' は必須です。")
    if terminal_clear == terminal_fail:
        _err("V-ED-09", f"scenes.terminal_clear と scenes.terminal_fail は異なる名前にしてください。"
             f" 両方 '{terminal_clear}' です。")

    # V-ED-10: 予約語チェック
    for field, value in [("terminal_clear", terminal_clear), ("terminal_fail", terminal_fail)]:
        if value in RESERVED_SCENE_NAMES:
            _err("V-ED-10", f"scenes.{field} に予約済みシーン名 '{value}' は使用できません。")

    # scenes.continue
    continue_scene = scenes_cfg.get("continue", "shoot_again")
    if not continue_scene or not IDENTIFIER_RE.match(str(continue_scene)):
        _err("V-ED-11", f"scenes.continue '{continue_scene}' は英数字・アンダースコアのみにしてください。")
    if continue_scene in RESERVED_SCENE_NAMES:
        _err("V-ED-10", f"scenes.continue に予約済みシーン名 '{continue_scene}' は使用できません。")

    # V-ED-11: continue と terminal が重複しないこと
    if continue_scene == terminal_clear:
        _err("V-ED-11", f"scenes.continue '{continue_scene}' と scenes.terminal_clear が同名です。")
    if continue_scene == terminal_fail:
        _err("V-ED-11", f"scenes.continue '{continue_scene}' と scenes.terminal_fail が同名です。")

    # V-ED-12: terminal_result
    tr = spec.get("terminal_result", {})
    if not isinstance(tr, dict):
        _err("V-ED-12", "'terminal_result' は辞書型である必要があります。")
    for key in ("clear", "fail"):
        val = tr.get(key)
        if val is not None and val not in VALID_RESULTS:
            _err("V-ED-12", f"terminal_result.{key} の値 '{val}' は無効です。"
                 f" 有効値: {sorted(VALID_RESULTS)}")


# ──────────────────────────────────────────────────────────────────────────────
# YAML 生成
# ──────────────────────────────────────────────────────────────────────────────

def _indent(text: str, level: int = 2) -> str:
    prefix = " " * level
    return "\n".join(prefix + line if line else line for line in text.splitlines())


def _build_wait_action(wait_events: list, wait_timeout: float, comment: str = "") -> str:
    events_str = "[" + ", ".join(wait_events) + "]"
    timeout_str = str(wait_timeout) if isinstance(wait_timeout, int) else f"{wait_timeout}"
    cmt = f"  # {comment}" if comment else ""
    return (
        f"      - action: wait_for_billiards_event{cmt}\n"
        f"        payload:\n"
        f"          events:  {events_str}\n"
        f"          timeout: {timeout_str}"
    )


def _build_record_actions(records: list) -> str:
    lines = []
    for ev in records:
        lines.append(
            f"      - action: record_billiards_event\n"
            f"        payload:\n"
            f"          event: {ev}"
        )
    return "\n\n".join(lines) if lines else ""


def _build_evaluate_action(terminal_clear: str, terminal_fail: str, if_continue: str) -> str:
    return (
        f"      - action: evaluate_billiards_round\n"
        f"        payload:\n"
        f"          if_clear:    {terminal_clear}\n"
        f"          if_fail:     {terminal_fail}\n"
        f"          if_continue: {if_continue}"
    )


def _build_end_game_action(result: str, comment: str = "") -> str:
    cmt = f"  # {comment}" if comment else ""
    return (
        f"      - action: end_game{cmt}\n"
        f"        payload: {{result: {result}}}"
    )


def generate_yaml(spec: dict) -> str:
    sname          = spec["scenario_name"]
    description    = spec.get("description", "")
    setup          = spec["setup"]
    shot_limit     = setup["shot_limit"]
    target_count   = setup["target_count"]
    wait_events    = spec.get("wait_events", ["balls_stopped"])
    wait_timeout   = spec.get("wait_timeout", 0.1)
    boot_records   = spec.get("boot_records", [])
    cont_records   = spec.get("continue_records", [])
    scenes_cfg     = spec.get("scenes", {})
    terminal_clear = scenes_cfg["terminal_clear"]
    terminal_fail  = scenes_cfg["terminal_fail"]
    continue_scene = scenes_cfg.get("continue", "shoot_again")
    tr             = spec.get("terminal_result", {})
    result_clear   = tr.get("clear", "solved")
    result_fail    = tr.get("fail", "failed")

    now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # ── ヘッダコメント ────────────────────────────────────────────────────────
    header_lines = [
        f"# Generated by gen_scenario_event_driven.py  [{now_str}]",
        f"# scenario_name: {sname}",
        f"# template:      event_driven_basic",
    ]
    if description:
        header_lines.append(f"# description:   {description}")
    header_lines += [
        "#",
        "# ── billiards_test Task データフロー ────────────────────────────────────",
        "# setup_billiards_round → round:shots_taken=0, round:shot_limit, round:target_count",
        "# wait_for_billiards_event  → TaskResult::Waiting ループ → timeout → event:last_name='balls_stopped'",
        "# record_billiards_event    → イベント名に応じて WorldState フラグ更新",
        "# evaluate_billiards_round  → cue_foul > pocketed >= target > shots >= limit の優先順で判定",
        "#",
        f"# setup.shot_limit:   {shot_limit}",
        f"# setup.target_count: {target_count}",
        f"# wait_events:        {wait_events}",
        f"# wait_timeout:       {wait_timeout}",
        f"# boot_records:       {boot_records if boot_records else '(なし)'}",
        f"# continue_records:   {cont_records if cont_records else '(なし)'}",
        f"# continue_scene:     {continue_scene}",
        f"# terminal_clear:     {terminal_clear}  (result: {result_clear})",
        f"# terminal_fail:      {terminal_fail}  (result: {result_fail})",
        "#",
        "# ── pos-0 sacrifice 適用ルール（R-8/R-9/R-10 確定） ────────────────────",
        "# setup_round (boot):   sacrifice 不要（_ready() 経由, index=0 から実行）",
        f"# {continue_scene}:  pos-0 sacrifice 必要（evaluate → load_scene → index++ → skip）",
        f"# {terminal_clear} / {terminal_fail}: pos-0 sacrifice 必要（同上）",
        "#",
        "# ── フロー概要 ──────────────────────────────────────────────────────────",
        "# setup_round",
        "#   └─ setup_billiards_round",
        "#   └─ wait_for_billiards_event(timeout) → balls_stopped",
    ]
    for ev in boot_records:
        header_lines.append(f"#   └─ record_billiards_event({ev})")
    header_lines += [
        "#   └─ evaluate_billiards_round",
        f"#        ├── if_clear    → {terminal_clear}",
        f"#        ├── if_fail     → {terminal_fail}",
        f"#        └── if_continue → {continue_scene}",
        f"# {continue_scene}  (pos-0 sacrificed)",
        "#   └─ [sacrifice] wait_for_billiards_event",
        "#   └─ [actual]    wait_for_billiards_event(timeout) → balls_stopped",
    ]
    for ev in cont_records:
        header_lines.append(f"#   └─ record_billiards_event({ev})")
    header_lines += [
        "#   └─ evaluate_billiards_round (同様の3分岐)",
        f"# {terminal_clear} / {terminal_fail}  (pos-0 sacrificed)",
        "#   └─ [sacrifice] end_game",
        "#   └─ [actual]    end_game",
    ]

    header = "\n".join(header_lines)

    # ── setup_round（boot） ───────────────────────────────────────────────────
    boot_parts = [
        f"  {BOOT_SCENE_NAME}:",
        f"    on_enter:",
        f"      - action: setup_billiards_round",
        f"        payload:",
        f"          shot_limit:   {shot_limit}",
        f"          target_count: {target_count}",
        "",
        _build_wait_action(wait_events, wait_timeout),
    ]
    if boot_records:
        boot_parts.append("")
        boot_parts.append(_build_record_actions(boot_records))
    boot_parts.append("")
    boot_parts.append(_build_evaluate_action(terminal_clear, terminal_fail, continue_scene))
    boot_scene = "\n".join(boot_parts)

    # ── continue シーン（pos-0 sacrifice 必要） ────────────────────────────────
    cont_parts = [
        f"  {continue_scene}:",
        f"    on_enter:",
        _build_wait_action(wait_events, wait_timeout, "pos-0 sacrifice（スキップされる）"),
        "",
        _build_wait_action(wait_events, wait_timeout, "実際に実行される"),
    ]
    if cont_records:
        cont_parts.append("")
        cont_parts.append(_build_record_actions(cont_records))
    cont_parts.append("")
    cont_parts.append(_build_evaluate_action(terminal_clear, terminal_fail, continue_scene))
    cont_scene = "\n".join(cont_parts)

    # ── terminal clear（pos-0 sacrifice 必要） ─────────────────────────────────
    clear_parts = [
        f"  {terminal_clear}:",
        f"    on_enter:",
        _build_end_game_action(result_clear, "pos-0 sacrifice（スキップされる）"),
        "",
        _build_end_game_action(result_clear, "実際に実行される"),
    ]
    clear_scene = "\n".join(clear_parts)

    # ── terminal fail（pos-0 sacrifice 必要） ──────────────────────────────────
    fail_parts = [
        f"  {terminal_fail}:",
        f"    on_enter:",
        _build_end_game_action(result_fail, "pos-0 sacrifice（スキップされる）"),
        "",
        _build_end_game_action(result_fail, "実際に実行される"),
    ]
    fail_scene = "\n".join(fail_parts)

    # ── 組み立て ──────────────────────────────────────────────────────────────
    lines = [
        header,
        f"start_scene: {BOOT_SCENE_NAME}",
        "",
        "scenes:",
        "",
        "  # 1. boot（_ready() 起動; pos-0 sacrifice 不要） ─────────────────────",
        boot_scene,
        "",
        f"  # 2. continue ループ（pos-0 sacrifice 適用） ─────────────────────────",
        cont_scene,
        "",
        f"  # 3. clear terminal（pos-0 sacrifice 適用） ─────────────────────────",
        clear_scene,
        "",
        f"  # 4. fail terminal（pos-0 sacrifice 適用） ──────────────────────────",
        fail_scene,
    ]
    return "\n".join(lines) + "\n"


# ──────────────────────────────────────────────────────────────────────────────
# review.md 生成
# ──────────────────────────────────────────────────────────────────────────────

def generate_review(spec: dict, yaml_path: Path) -> str:
    sname          = spec["scenario_name"]
    setup          = spec["setup"]
    shot_limit     = setup["shot_limit"]
    target_count   = setup["target_count"]
    wait_timeout   = spec.get("wait_timeout", 0.1)
    boot_records   = spec.get("boot_records", [])
    cont_records   = spec.get("continue_records", [])
    scenes_cfg     = spec.get("scenes", {})
    terminal_clear = scenes_cfg["terminal_clear"]
    terminal_fail  = scenes_cfg["terminal_fail"]
    continue_scene = scenes_cfg.get("continue", "shoot_again")
    tr             = spec.get("terminal_result", {})
    result_clear   = tr.get("clear", "solved")
    result_fail    = tr.get("fail", "failed")
    now_str        = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # イベント効果テーブル
    EVENT_EFFECTS = {
        "shot_committed":    "round:shots_taken +1",
        "ball_pocketed":     "round:target_1_pocketed → true (1球目) / target_2_pocketed → true (2球目)",
        "cue_ball_pocketed": "round:cue_ball_pocketed = true",
        "balls_stopped":     "WorldState 変更なし（サイクル区切り）",
    }

    def _event_table(records):
        if not records:
            return "_（なし）_"
        rows = ["| イベント名 | WorldState 効果 |", "|:---|:---|"]
        for ev in records:
            rows.append(f"| `{ev}` | {EVENT_EFFECTS.get(ev, '?')} |")
        return "\n".join(rows)

    # clear 条件の推定
    def _clear_condition(boot_recs, cont_recs):
        pocketed_boot = boot_recs.count("ball_pocketed")
        pocketed_cont = cont_recs.count("ball_pocketed")
        foul_boot = "cue_ball_pocketed" in boot_recs
        if foul_boot:
            return "boot シーンに cue_ball_pocketed が含まれるため、1周目は常に fail"
        if pocketed_boot >= target_count:
            return f"boot シーンで ball_pocketed が {pocketed_boot} 回 → pocketed({pocketed_boot}) >= target_count({target_count}) → 1周目で clear"
        if pocketed_cont >= target_count:
            return f"boot で pocketed < target → continue → shoot_again で ball_pocketed {pocketed_cont} 回 → clear"
        return f"clear 条件: 累積ポケット数 >= {target_count}（現在の記録設定では確認が必要）"

    review = f"""# {sname} — Generator Review
**生成日**: {now_str}
**Generator**: gen_scenario_event_driven.py (T14)
**出力ファイル**: `{yaml_path.name}`

---

## 1. 生成パラメータ確認

| パラメータ | 値 |
|:---|:---|
| `setup.shot_limit` | {shot_limit} |
| `setup.target_count` | {target_count} |
| `wait_timeout` | {wait_timeout} s |
| `boot_records` | {boot_records if boot_records else "(なし)"} |
| `continue_records` | {cont_records if cont_records else "(なし)"} |
| `continue_scene` | `{continue_scene}` |
| `terminal_clear` | `{terminal_clear}` → `result: {result_clear}` |
| `terminal_fail` | `{terminal_fail}` → `result: {result_fail}` |

---

## 2. boot シーン イベント効果

{_event_table(boot_records)}

## 3. continue シーン イベント効果

{_event_table(cont_records)}

---

## 4. evaluate_billiards_round 判定ロジック（確認用）

```
判定優先順位（上から評価）:
  1. cue_ball_pocketed == true                    → if_fail（ファウル優先）
  2. (target_1_pocketed + target_2_pocketed)
       >= target_count({target_count})             → if_clear
  3. shots_taken >= shot_limit({shot_limit})       → if_fail（ショット上限）
  4. それ以外                                      → if_continue
```

**このシナリオの clear 経路推定**:
{_clear_condition(boot_records, cont_records)}

---

## 5. フロー図

```
setup_round (boot)
  setup_billiards_round (shot_limit={shot_limit}, target_count={target_count})
  wait_for_billiards_event (timeout={wait_timeout})
  → [timeout] event:last_name = "balls_stopped"
"""
    for ev in boot_records:
        review += f"  record_billiards_event({ev})\n"
    review += f"""  evaluate_billiards_round
    ├── if_clear    → {terminal_clear}   (result: {result_clear})
    ├── if_fail     → {terminal_fail}    (result: {result_fail})
    └── if_continue → {continue_scene}

{continue_scene} (pos-0 sacrificed)
  [skip] wait_for_billiards_event
  [run]  wait_for_billiards_event (timeout={wait_timeout})
"""
    for ev in cont_records:
        review += f"  record_billiards_event({ev})\n"
    review += f"""  evaluate_billiards_round (同様)
    └── if_continue → {continue_scene}  (自己ループ)

{terminal_clear} (pos-0 sacrificed)  →  end_game(result: {result_clear})
{terminal_fail}  (pos-0 sacrificed)  →  end_game(result: {result_fail})
```

---

## 6. HG チェックリスト

### HG-2: 生成 YAML レビュー

- [ ] `setup_round` に sacrifice がない（boot は不要）
- [ ] `{continue_scene}` の on_enter が `wait_for_billiards_event` × 2 で始まる（sacrifice + actual）
- [ ] `{terminal_clear}` / `{terminal_fail}` が `end_game` × 2 で始まる（sacrifice + actual）
- [ ] `evaluate_billiards_round` の `if_clear/if_fail/if_continue` が意図通り（各シーン 3 箇所確認）
- [ ] `boot_records` / `continue_records` のイベント名が意図通り

### HG-3: validate_scenario.py

```bash
python3 tools/validate_scenario.py {yaml_path}
# → exit 0 を確認
```

### HG-4: Godot headless smoke テスト

3 経路を各1回実行:

1. **clear 経路**: `boot_records` に `ball_pocketed` を含む場合 → `result: {result_clear}`
2. **fail 経路**: `boot_records` に `cue_ball_pocketed` を含む場合 → `result: {result_fail}`
3. **continue 経路**: boot で clear/fail 条件未達 → `{continue_scene}` でループ

期待ログ:
```
[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.
[EvaluateBilliardsRoundTask] Clear → {terminal_clear}
[EndGameTask] Game ended with result: {result_clear}
```
"""
    return review


# ──────────────────────────────────────────────────────────────────────────────
# メイン
# ──────────────────────────────────────────────────────────────────────────────

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Event-Driven Basic Scaffold Generator (Phase 3-C T14)"
    )
    parser.add_argument("spec", help="Structured Spec YAML ファイルパス")
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

    try:
        with open(spec_path, encoding="utf-8") as f:
            spec = yaml.safe_load(f)
    except yaml.YAMLError as e:
        print(f"ERROR: YAML 解析エラー: {e}", file=sys.stderr)
        return 2

    if not isinstance(spec, dict):
        print("ERROR: spec の最上位は dict である必要があります。", file=sys.stderr)
        return 2

    # Fail-Fast バリデーション
    validate_spec(spec)

    # 出力先
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    sname = spec["scenario_name"]
    yaml_path   = out_dir / f"{sname}_{timestamp}.yaml"
    review_path = out_dir / f"{sname}_{timestamp}_review.md"

    # 生成
    yaml_content   = generate_yaml(spec)
    review_content = generate_review(spec, yaml_path)

    yaml_path.write_text(yaml_content, encoding="utf-8")
    review_path.write_text(review_content, encoding="utf-8")

    print(f"Generated: {yaml_path}")
    print(f"Review:    {review_path}")
    print("次のステップ: HG-2 → tools/validate_scenario.py → HG-3 → Godot headless HG-4")
    return 0


if __name__ == "__main__":
    sys.exit(main())
