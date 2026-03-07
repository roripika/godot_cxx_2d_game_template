#!/bin/bash
# Mystery Demo 起動スクリプト
cd "$(dirname "$0")"
/Applications/Godot.app/Contents/MacOS/Godot --path . res://scenes/demo/InvestigationStage.tscn
