#!/bin/bash
# run_billiards.command
# 
# Billiards 3D テスト用の起動スクリプト。

ROOT_DIR=$(cd $(dirname $0); pwd)
cd $ROOT_DIR

# デフォルトで .tscn を起動。引数があればそれを使用。
if [ $# -eq 0 ]; then
    /opt/homebrew/bin/godot samples/billiards/billiards_test_main.tscn
else
    /opt/homebrew/bin/godot "$@"
fi
