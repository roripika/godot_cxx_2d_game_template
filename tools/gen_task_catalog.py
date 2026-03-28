#!/usr/bin/env python3
"""
gen_task_catalog.py — Task Catalog Auto-gen (Phase 3-B / T7)

src/games/*/tasks/*.cpp を走査し、各 Task の
  - クラス名 / モジュール
  - 必須 payload key / 省略可能 payload key
  - WorldState 書き込みキー / 読み込みキー
  - TaskResult::Waiting 使用有無

を抽出して docs/task_catalog.md に出力する。

Usage:
    python3 tools/gen_task_catalog.py [--out docs/task_catalog.md]

スコープ定義 (T7.1):
    対象: src/games/*/tasks/*.cpp  (ヘッダ .h は参照しない)
    除外: src/core/ 配下の TaskBase 等のカーネル Task (改変禁止領域)
    カウント根拠: billiards_test(4) + mystery_test(9) + rhythm_test(6) + roguelike_test(7) = 26
    ※ 開発ログに「30 tasks」と記載されている箇所があるが、これは 2026-03-28 の
       初期ナレーションにおける計算ミスであり正しい値は 26 です。

制約:
    - src/core/ および実装コードには一切変更を加えない (読み取り専用)
    - docs/ への Markdown 出力のみ
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
from datetime import date
from typing import NamedTuple

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
ROOT = pathlib.Path(__file__).resolve().parent.parent
GAMES_DIR = ROOT / "src" / "games"
DEFAULT_OUT = ROOT / "docs" / "task_catalog.md"

# ---------------------------------------------------------------------------
# Regex patterns
# ---------------------------------------------------------------------------
# Required payload keys:  !spec.payload.has("key")  (handles OR chains too)
# e.g. if (!has("speaker") || !has("text")) → both captured
_RE_PAYLOAD_REQUIRED = re.compile(r'!spec\.payload\.has\("([^"]+)"\)')

# All accessed payload keys: spec.payload.has("key") or spec.payload["key"]
_RE_PAYLOAD_HAS = re.compile(r'spec\.payload\.has\("([^"]+)"\)')
_RE_PAYLOAD_GET = re.compile(r'spec\.payload\["([^"]+)"\]')

# WorldState keys (must contain ':' to filter out enum/state strings)
# Matches the 3rd string argument in set_state / get_state calls.
# Pattern: ->set_state( "module" , SCOPE , "key:name"
_RE_WS_WRITE = re.compile(
    r'->set_state\(\s*"[^"]+"\s*,\s*[^,]+,\s*"([^"]+)"'
)
_RE_WS_READ = re.compile(
    r'->get_state\(\s*"[^"]+"\s*,\s*[^,]+,\s*"([^"]+)"'
)

_RE_WAITING = re.compile(r'\bTaskResult::Waiting\b')

# Module names to exclude from WS key lists
_MODULE_NAMES = {"mystery_test", "roguelike_test", "rhythm_test", "billiards_test"}


# ---------------------------------------------------------------------------
# Data
# ---------------------------------------------------------------------------
class TaskInfo(NamedTuple):
    module: str
    filename: str          # e.g. "add_evidence_task"
    class_name: str        # e.g. "AddEvidenceTask"
    display_name: str      # e.g. "Add Evidence"
    payload_required: list[str]
    payload_optional: list[str]
    ws_write: list[str]
    ws_read: list[str]
    uses_waiting: bool


# ---------------------------------------------------------------------------
# Extraction
# ---------------------------------------------------------------------------
def _snake_to_title(s: str) -> str:
    """add_evidence → Add Evidence"""
    return " ".join(w.capitalize() for w in s.split("_"))


def _snake_to_camel(s: str) -> str:
    """add_evidence_task → AddEvidenceTask"""
    return "".join(w.capitalize() for w in s.split("_"))


def _is_ws_key(s: str) -> bool:
    """WorldState の意味ある key は ':' を含む"""
    return ":" in s and s not in _MODULE_NAMES


def extract_task(cpp: pathlib.Path, module: str) -> TaskInfo:
    text = cpp.read_text(encoding="utf-8", errors="replace")
    stem = cpp.stem  # "add_evidence_task"

    # Payload keys
    required = sorted(set(_RE_PAYLOAD_REQUIRED.findall(text)))
    all_accessed = set(_RE_PAYLOAD_HAS.findall(text)) | set(_RE_PAYLOAD_GET.findall(text))
    optional = sorted(all_accessed - set(required))

    # WorldState keys
    ws_write = sorted({k for k in _RE_WS_WRITE.findall(text) if _is_ws_key(k)})
    ws_read  = sorted({k for k in _RE_WS_READ.findall(text)  if _is_ws_key(k)})

    # Display name: remove "_task" suffix, then title-case
    bare = stem[:-5] if stem.endswith("_task") else stem
    display = _snake_to_title(bare)
    class_name = _snake_to_camel(stem)
    uses_waiting = bool(_RE_WAITING.search(text))

    return TaskInfo(
        module=module,
        filename=stem,
        class_name=class_name,
        display_name=display,
        payload_required=required,
        payload_optional=optional,
        ws_write=ws_write,
        ws_read=ws_read,
        uses_waiting=uses_waiting,
    )


def collect_all(games_dir: pathlib.Path) -> dict[str, list[TaskInfo]]:
    """module → [TaskInfo, ...] を返す（モジュール名順）"""
    result: dict[str, list[TaskInfo]] = {}
    for module_dir in sorted(games_dir.iterdir()):
        if not module_dir.is_dir():
            continue
        tasks_dir = module_dir / "tasks"
        if not tasks_dir.exists():
            continue
        module = module_dir.name
        tasks: list[TaskInfo] = []
        for cpp in sorted(tasks_dir.glob("*.cpp")):
            tasks.append(extract_task(cpp, module))
        if tasks:
            result[module] = tasks
    return result


# ---------------------------------------------------------------------------
# Markdown rendering
# ---------------------------------------------------------------------------
def _fmt_keys(keys: list[str]) -> str:
    if not keys:
        return "—"
    return "<br>".join(f"`{k}`" for k in keys)


def _module_anchor(module: str) -> str:
    return module.replace("_", "-")


MODULE_LABELS = {
    "mystery_test":   "mystery_test — 分岐推理 (Branching / Evidence)",
    "roguelike_test": "roguelike_test — ターン制グリッド (Turn/Grid)",
    "rhythm_test":    "rhythm_test — 時刻駆動 (Time/Clock)",
    "billiards_test": "billiards_test — イベント駆動 (Event-Driven / Waiting)",
}

TEMPLATE_LINKS = {
    "mystery_test":   "../examples/templates/branching_basic/",
    "roguelike_test": "../examples/templates/turn_grid_basic/",
    "rhythm_test":    "../examples/templates/time_clock_basic/",
    "billiards_test": "../examples/templates/event_driven_basic/",
}


def render_markdown(catalog: dict[str, list[TaskInfo]], generated_date: str) -> str:
    lines: list[str] = []

    # Header
    lines += [
        "# Task Catalog",
        "",
        f"**Generated**: {generated_date}  ",
        f"**Source**: `src/games/*/tasks/*.cpp` (read-only scan)  ",
        "**Generator**: `tools/gen_task_catalog.py`  ",
        "",
        "> AI がシナリオを書く際の「どの Task を使うか」判断を支援するカタログ。  ",
        "> `src/core` には一切変更を加えていません。",
        "",
        "## スコープ定義 (T7.1)",
        "",
        "| 項目 | 値 |",
        "|:---|:---|",
        "| 走査対象 | `src/games/*/tasks/*.cpp`（`.h` は除外） |",
        "| 除外領域 | `src/core/`（Kernel = 改変禁止領域） |",
        "| カウント根拠 | billiards_test(4) + mystery_test(9) + rhythm_test(6) + roguelike_test(7) = **26** |",
        "| 注意 | 開発ログ内に「30 tasks」との記述があるが、これは 2026-03-28 の初期ナレーションにおける計算ミス。正しい値は **26**。 |",
        "",
        "---",
        "",
    ]

    # Summary table
    lines += [
        "## 概要",
        "",
        "| モジュール | Task 数 | Waiting 使用 | テンプレート |",
        "|:---|:---:|:---:|:---|",
    ]
    for module, tasks in catalog.items():
        waiting_count = sum(1 for t in tasks if t.uses_waiting)
        waiting_cell = f"✅ {waiting_count} 件" if waiting_count else "—"
        tmpl = TEMPLATE_LINKS.get(module, "")
        tmpl_cell = f"[link]({tmpl})" if tmpl else "—"
        label = MODULE_LABELS.get(module, module)
        lines.append(f"| [{module}](#{_module_anchor(module)}) | {len(tasks)} | {waiting_cell} | {tmpl_cell} |")
    lines += ["", "---", ""]

    # Per-module sections
    for module, tasks in catalog.items():
        label = MODULE_LABELS.get(module, module)
        tmpl = TEMPLATE_LINKS.get(module, "")
        lines += [
            f"## {label}",
            "",
        ]
        if tmpl:
            lines += [f"参照テンプレート: [{tmpl}]({tmpl})", ""]

        lines += [
            "| Task クラス | 責務 | 必須 payload key | 省略可能 payload key | WS 書き込みキー | WS 読み込みキー | Waiting |",
            "|:---|:---|:---|:---|:---|:---|:---:|",
        ]
        for t in tasks:
            waiting_mark = "✅" if t.uses_waiting else ""
            row = (
                f"| `{t.class_name}` "
                f"| {t.display_name} "
                f"| {_fmt_keys(t.payload_required)} "
                f"| {_fmt_keys(t.payload_optional)} "
                f"| {_fmt_keys(t.ws_write)} "
                f"| {_fmt_keys(t.ws_read)} "
                f"| {waiting_mark} |"
            )
            lines.append(row)
        lines += ["", "---", ""]

    # Footer
    total = sum(len(v) for v in catalog.values())
    lines += [
        "## 統計",
        "",
        f"- 総 Task 数: **{total}**",
        f"- モジュール数: **{len(catalog)}**",
        f"- `TaskResult::Waiting` 使用 Task: "
        + f"**{sum(1 for ts in catalog.values() for t in ts if t.uses_waiting)}**",
        "",
        "_このファイルは自動生成です。手動編集しないでください。_  ",
        f"_再生成: `python3 tools/gen_task_catalog.py`_",
    ]

    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--out",
        type=pathlib.Path,
        default=DEFAULT_OUT,
        help=f"出力先 Markdown ファイル (default: {DEFAULT_OUT})",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="ファイルに書かず stdout に出力",
    )
    args = parser.parse_args()

    if not GAMES_DIR.exists():
        print(f"ERROR: {GAMES_DIR} が存在しません", file=sys.stderr)
        sys.exit(1)

    catalog = collect_all(GAMES_DIR)
    if not catalog:
        print("ERROR: Task ファイルが見つかりませんでした", file=sys.stderr)
        sys.exit(1)

    md = render_markdown(catalog, generated_date=date.today().isoformat())

    if args.dry_run:
        print(md)
    else:
        out: pathlib.Path = args.out
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(md, encoding="utf-8")
        total = sum(len(v) for v in catalog.values())
        print(f"Generated: {out}  ({total} tasks across {len(catalog)} modules)")


if __name__ == "__main__":
    main()
