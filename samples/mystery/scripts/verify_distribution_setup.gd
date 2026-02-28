extends SceneTree

func _wait_frames(n: int):
	for i in range(n):
		await create_timer(0.01).timeout

func _assert(cond: bool, msg: String):
	if not cond:
		push_error("[VERIFY] ASSERT FAILED: " + msg)
		return false
	return true

func _run():
	print("[VERIFY] Testing Distribution Setup...")
	
	# 1. 起動シーンの設定を確認
	var config = ConfigFile.new()
	var err = config.load("res://project.godot")
	if err == OK:
		var main_scene = config.get_value("application", "run/main_scene")
		_assert(main_scene == "res://samples/mystery/mystery_title.tscn", "Main scene should be mystery_title.tscn")
		var proj_name = config.get_value("application", "config/name")
		_assert(proj_name == "Mystery Demo: Apparition at the Warehouse", "Project name should be updated")
	else:
		_assert(false, "Failed to load project.godot")

	# 2. タイトルシーンのロード検証
	var title_scene = load("res://samples/mystery/mystery_title.tscn")
	var root = title_scene.instantiate()
	get_root().add_child(root)
	
	await _wait_frames(10)
	
	var start_btn = root.find_child("StartButton", true, false)
	_assert(start_btn != null, "StartButton not found")
	_assert(start_btn.text == "調査開始", "StartButton text should be 調査開始")
	
	var quit_btn = root.find_child("QuitButton", true, false)
	_assert(quit_btn != null, "QuitButton not found")
	
	print("[VERIFY] Distribution setup verified successfully.")
	quit(0)

func _init():
	_run()
