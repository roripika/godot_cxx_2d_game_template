## invader_test_main.gd
## Invader デモのメインスクリプト。
##
## GUI 手作業（ノード配置・グループ追加・シグナル接続）は一切不要。
## _ready() の中ですべてのノード生成・シグナル結線を完結させる。
extends Node2D

# ────────────────────────────────────────────
# アセット参照（PackedScene / Texture）— コードでロード
# ────────────────────────────────────────────
const BULLET_SCENE    := preload("res://samples/invaders/invader_bullet_view.tscn")
const ENEMY_SCENE     := preload("res://samples/invaders/invader_enemy_view.tscn")
const PLAYER_TEXTURE  := preload("res://samples/invaders/assets_v3/pilot_ship.png")

# ────────────────────────────────────────────
# 内部参照（_ready で初期化）
# ────────────────────────────────────────────
var _manager:      InvaderManager
var _player:       InvaderPlayer
var _score_label:  Label
var _status_label: Label

# ════════════════════════════════════════════
# エントリポイント
# ════════════════════════════════════════════
func _ready() -> void:
	_setup_manager()
	_setup_ui()
	_setup_player()
	_setup_enemies()
	print("[InvaderMain] All nodes created and signals connected via code.")

# ════════════════════════════════════════════
# セットアップ: InvaderManager
# ════════════════════════════════════════════
func _setup_manager() -> void:
	_manager = InvaderManager.new()
	_manager.name       = "InvaderManager"
	_manager.game_over_y = 500.0
	add_child(_manager)

	# ── シグナル接続 ──
	_manager.score_changed.connect(_on_score_changed)
	_manager.game_over.connect(_on_game_over)
	_manager.game_clear.connect(_on_game_clear)

# ════════════════════════════════════════════
# セットアップ: UI（CanvasLayer + Label × 2）
# ════════════════════════════════════════════
func _setup_ui() -> void:
	var canvas := CanvasLayer.new()
	canvas.name = "UI"
	add_child(canvas)

	# スコアラベル
	_score_label = Label.new()
	_score_label.name     = "ScoreLabel"
	_score_label.text     = "SCORE: 0"
	_score_label.position = Vector2(20, 20)
	_score_label.add_theme_font_size_override("font_size", 24)
	canvas.add_child(_score_label)

	# ステータスラベル（GAME OVER / GAME CLEAR 表示用）
	_status_label = Label.new()
	_status_label.name                         = "StatusLabel"
	_status_label.text                         = ""
	_status_label.position                     = Vector2(20, 60)
	_status_label.add_theme_font_size_override("font_size", 36)
	canvas.add_child(_status_label)

# ════════════════════════════════════════════
# セットアップ: InvaderPlayer（コードで構築）
# ════════════════════════════════════════════
func _setup_player() -> void:
	_player = InvaderPlayer.new()
	_player.name     = "InvaderPlayer"
	_player.position = Vector2(576, 520)

	# Sprite2D — テクスチャをコードでセット
	var sprite := Sprite2D.new()
	sprite.texture = PLAYER_TEXTURE
	_player.add_child(sprite)

	# CollisionShape2D — 形状もコードで生成
	var shape := CollisionShape2D.new()
	var rect  := RectangleShape2D.new()
	rect.size   = Vector2(32, 32)
	shape.shape = rect
	_player.add_child(shape)

	add_child(_player)

	# ── シグナル接続 ──
	_player.bullet_fired.connect(_on_player_bullet_fired)
	# プレイヤー自身の Area2D に敵が重なったらゲームオーバー
	_player.area_entered.connect(_on_player_area_entered)

# ════════════════════════════════════════════
# セットアップ: InvaderEnemy グリッド
#   5列 × 2行 を ENEMY_SCENE から instantiate()
# ════════════════════════════════════════════
func _setup_enemies() -> void:
	const COLS    := 5
	const ROWS    := 2
	const SPACE_X := 75.0
	const SPACE_Y := 50.0

	var root := Node2D.new()
	root.name     = "Enemies"
	root.position = Vector2(576, 150)
	add_child(root)

	var offset_x := -(COLS - 1) * SPACE_X / 2.0

	for row in ROWS:
		for col in COLS:
			var enemy: InvaderEnemy = ENEMY_SCENE.instantiate()
			enemy.position = Vector2(offset_x + col * SPACE_X, row * SPACE_Y)
			root.add_child(enemy)

			# 撃破シグナル → InvaderManager.add_score に直結
			enemy.died.connect(_manager.add_score)

# ════════════════════════════════════════════
# シグナルハンドラ
# ════════════════════════════════════════════

## プレイヤーが弾を発射 → 弾ノードをコードで追加
func _on_player_bullet_fired(spawn_pos: Vector2) -> void:
	var bullet = BULLET_SCENE.instantiate()
	bullet.global_position = spawn_pos
	add_child(bullet)

## プレイヤーの当たり判定に Area2D が侵入
## → InvaderEnemy だったらプレイヤー被弾として通知
func _on_player_area_entered(area: Area2D) -> void:
	if area is InvaderEnemy:
		print("[InvaderMain] Player hit by enemy!")
		_manager.notify_player_hit()

## スコア変化 → Label 更新
func _on_score_changed(new_score: int) -> void:
	_score_label.text = "SCORE: %d" % new_score

## ゲームオーバー
func _on_game_over() -> void:
	_status_label.text     = "GAME OVER"
	_status_label.modulate = Color.RED
	print("[InvaderMain] GAME OVER")

## ゲームクリア
func _on_game_clear() -> void:
	_status_label.text     = "GAME CLEAR!"
	_status_label.modulate = Color.GREEN
	print("[InvaderMain] GAME CLEAR!")
