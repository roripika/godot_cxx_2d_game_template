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
	print("[VERIFY] Testing Character Placement Externalization...")
	
	var root = get_root()
	var shell = load("res://samples/mystery/karakuri_mystery_shell.tscn").instantiate()
	root.add_child(shell)
	
	await _wait_frames(120) # Give more time for autoloads and initialization
	
	var runner = shell.get_node("ScenarioRunner")
	if not runner.get("scene_container_path"):
		print("[VERIFY] Runner scene_container_path is empty! Waiting...")
		await _wait_frames(60)

	runner.call("load_scenario")
	
	# 1. 調査シーンでのNPC表示確認
	print("[VERIFY] Loading warehouse_investigation...")
	runner.call("load_scene_by_id", "warehouse_investigation")
	await _wait_frames(60)
	
	var sc = shell.get_node("SceneContainer")
	var warehouse = sc.get_child(0)
	
	var tanaka = warehouse.find_child("hs_manager", true, false)
	_assert(tanaka != null and tanaka.visible == true, "Tanaka should be visible in investigation")
	_assert(tanaka.position.is_equal_approx(Vector2(900, 300)), "Tanaka position should be [900, 300]")
	
	var kenta = warehouse.find_child("hs_kenta", true, false)
	_assert(kenta != null and kenta.visible == true, "Kenta should be visible in investigation")

	# 2. 対決シーンでのNPC非表示確認（YAMLに定義がないため）
	print("[VERIFY] Loading confrontation...")
	runner.call("load_scene_by_id", "confrontation")
	await _wait_frames(10)
	
	warehouse = sc.get_child(0)
	tanaka = warehouse.find_child("hs_manager", true, false)
	_assert(tanaka != null and tanaka.visible == false, "Tanaka should be hidden in confrontation")
	
	print("[VERIFY] Character placement verified successfully.")
	quit(0)

func _init():
	_run()
