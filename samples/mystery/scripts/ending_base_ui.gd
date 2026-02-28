# 準備
func _ready() -> void:
	var sr := get_tree().root.find_child("ScenarioRunner", true, false)
	if sr:
		# ScenarioRunnerから現在のシーンIDを取得して、背景を切り替える
		var current_id = sr.get("current_scene_id")
		setup_ending(current_id == "ending_good")
	else:
		# テスト用：デフォルトはグッドエンド
		setup_ending(true)

# 事件の状態に合わせて背景を切り替える
func setup_ending(is_good: bool) -> void:
	var path := "res://assets/mystery/backgrounds/"
	if is_good:
		background.texture = load(path + "bg_ending_good.png")
	else:
		background.texture = load(path + "bg_ending_bad.png")
