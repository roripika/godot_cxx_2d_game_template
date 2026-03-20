extends Control
## roguelike_test 専用デバッグオブザーバーオーバーレイ
##
## WorldState の SCOPE_SESSION から roguelike_test の全キーを読み取り、
## 5x5 ASCII グリッドと 13 項目の情報パネルを毎フレーム更新する。
## ゲームロジック・Tasks・YAML・src/core には一切変更を加えない。

const NS := "roguelike_test"
const S  := 1  # WorldState.SCOPE_SESSION

@onready var grid_label: Label = $GridLabel
@onready var info_label: Label = $InfoLabel

var ws: Node = null

func _ready() -> void:
	# 親チェーン: DebugOverlay → CanvasLayer → RoguelikeDebugScene → WorldState
	ws = get_parent().get_parent().get_node_or_null("WorldState")
	if ws == null:
		push_error("[RoguelikeDebugOverlay] WorldState ノードが見つかりません。シーンに WorldState を配置してください。")

func _process(_delta: float) -> void:
	if ws == null:
		return
	_refresh()

func _refresh() -> void:
	var px: int        = int(ws.get_state(NS, S, "player:x",          1))
	var py: int        = int(ws.get_state(NS, S, "player:y",          1))
	var ex: int        = int(ws.get_state(NS, S, "enemy_1:x",         3))
	var ey: int        = int(ws.get_state(NS, S, "enemy_1:y",         3))
	var gx: int        = int(ws.get_state(NS, S, "goal:x",            4))
	var gy: int        = int(ws.get_state(NS, S, "goal:y",            4))
	var turn: int      = int(ws.get_state(NS, S, "turn:index",        0))
	var phase: String  = str(ws.get_state(NS, S, "turn:phase",        "player"))
	var php: int       = int(ws.get_state(NS, S, "player:hp",         3))
	var ehp: int       = int(ws.get_state(NS, S, "enemy_1:hp",        2))
	var result: String = str(ws.get_state(NS, S, "round:result",      ""))
	var actor: String  = str(ws.get_state(NS, S, "last_action:actor", ""))
	var atype: String  = str(ws.get_state(NS, S, "last_action:type",  ""))

	# ── 5x5 ASCII グリッド ─────────────────────────────────────────────
	# 凡例: P=プレイヤー  E=敵  G=ゴール  .=床
	# 行 = y (0が上), 列 = x (0が左)
	var grid := "    0 1 2 3 4\n"
	for row in range(5):
		grid += _grid_row(row, px, py, ex, ey, gx, gy)
	grid_label.text = grid

	# ── 13キー 情報パネル ──────────────────────────────────────────────
	info_label.text = (
		"=== ROGUELIKE DEBUG ===\n"
		+ "turn:index        = %d\n"  % turn
		+ "turn:phase        = %s\n"  % phase
		+ "player:x          = %d\n"  % px
		+ "player:y          = %d\n"  % py
		+ "player:hp         = %d\n"  % php
		+ "enemy_1:x         = %d\n"  % ex
		+ "enemy_1:y         = %d\n"  % ey
		+ "enemy_1:hp        = %d\n"  % ehp
		+ "goal:x            = %d\n"  % gx
		+ "goal:y            = %d\n"  % gy
		+ "round:result      = %s\n"  % result
		+ "last_action:actor = %s\n"  % actor
		+ "last_action:type  = %s\n"  % atype
	)

func _grid_row(row: int, px: int, py: int, ex: int, ey: int, gx: int, gy: int) -> String:
	var line := "%d | " % row
	for col in range(5):
		if   col == px and row == py: line += "P "
		elif col == ex and row == ey: line += "E "
		elif col == gx and row == gy: line += "G "
		else:                         line += ". "
	return line + "\n"
