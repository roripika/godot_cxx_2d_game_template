#!/bin/bash
# capture_rhythm_screenshots.command
#
# rhythm_test の clear / fail / continue の 3 経路を自動実行し、
# 各デバッグオーバーレイのスクリーンショットを docs/images/screenshots/ に保存する。
#
# 使い方:
#   ./capture_rhythm_screenshots.command          → 3 経路まとめて撮影
#   ./capture_rhythm_screenshots.command clear    → 指定した経路のみ
#   ./capture_rhythm_screenshots.command fail
#   ./capture_rhythm_screenshots.command continue

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")"; pwd)"
cd "$ROOT_DIR"

GODOT=/opt/homebrew/bin/godot
if [[ ! -x "$GODOT" ]]; then
  GODOT=/Applications/Godot.app/Contents/MacOS/Godot
fi
if [[ ! -x "$GODOT" ]]; then
  echo "ERROR: Godot が見つかりません。GODOT 変数を修正してください。" >&2
  exit 1
fi

SCRIPT_PATH="$ROOT_DIR/scripts/capture_rhythm_debug.gd"
OUT_DIR="$ROOT_DIR/docs/images/screenshots"

mkdir -p "$OUT_DIR"

# 引数がある場合は指定シーンのみ、なければ 3 経路すべて実行
if [[ $# -ge 1 ]]; then
  SCENES=("$@")
else
  SCENES=(clear fail continue)
fi

for KEY in "${SCENES[@]}"; do
  echo "=========================================="
  echo " Capturing: $KEY"
  echo "=========================================="
  "$GODOT" --path "$ROOT_DIR" --script "$SCRIPT_PATH" -- "$KEY"
  echo ""
  echo ">>> Saved: $OUT_DIR/rhythm_${KEY}.png"
  echo ""
done

echo "=========================================="
echo " All done!"
echo " Output: $OUT_DIR"
ls -lh "$OUT_DIR"/rhythm_*.png 2>/dev/null || true
echo "=========================================="
