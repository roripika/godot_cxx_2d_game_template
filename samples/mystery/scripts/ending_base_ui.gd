extends Node2D

@onready var background: TextureRect = $Background

# 準備
func _ready() -> void:
	var sr: Node = get_tree().root.find_child("ScenarioRunner", true, false)
	if sr:
		# ScenarioRunnerから現在のシーンIDを取得して、背景を切り替える
		var current_id = sr.get("current_scene_id")
		setup_ending(current_id == "ending_good")
	else:
		# テスト用：デフォルトはグッドエンド
		setup_ending(true)

# 事件の状態に合わせて背景を切り替える
func setup_ending(is_good: bool) -> void:
	# ヘッドレス（テスト）実行時はテクスチャをロードしない
	if DisplayServer.get_name() == "headless":
		return
	var path := "res://assets/mystery/backgrounds/"
	var tex_path := path + ("bg_ending_good.png" if is_good else "bg_ending_bad.png")
	background.texture = load(tex_path)
