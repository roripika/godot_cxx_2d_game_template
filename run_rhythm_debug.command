#!/bin/bash
# run_rhythm_debug.command
#
# rhythm_test デバッグオブザーバー起動スクリプト。
#
# 使い方:
#   引数なし  → rhythm_debug_clear.tscn を起動 (デフォルト)
#   clear     → rhythm_debug_clear.tscn
#   fail      → rhythm_debug_fail.tscn
#   continue  → rhythm_debug_continue.tscn

ROOT_DIR=$(cd $(dirname $0); pwd)
cd $ROOT_DIR

case "${1}" in
    fail)
        /opt/homebrew/bin/godot samples/rhythm_test/rhythm_debug_fail.tscn
        ;;
    continue)
        /opt/homebrew/bin/godot samples/rhythm_test/rhythm_debug_continue.tscn
        ;;
    clear|"")
        /opt/homebrew/bin/godot samples/rhythm_test/rhythm_debug_clear.tscn
        ;;
    *)
        # フルパスなど任意の引数をそのまま渡す
        /opt/homebrew/bin/godot "$@"
        ;;
esac
