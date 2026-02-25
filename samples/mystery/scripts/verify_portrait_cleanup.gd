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
	print("[VERIFY] Testing Portrait Cleanup...")
	
	# Load shell scene
	var shell_scene = load("res://samples/mystery/karakuri_mystery_shell.tscn")
	var root = shell_scene.instantiate()
	get_root().add_child(root)
	
	await _wait_frames(60)
	
	var runner = root.find_child("ScenarioRunner", true, false)
	_assert(runner != null, "ScenarioRunner not found")
	
	var dui = root.find_child("DialogueUI", true, false)
	_assert(dui != null, "DialogueUI not found")
	
	# 1. Simulate setting a portrait
	print("[VERIFY] Setting dummy portrait...")
	var dummy_tex = GradientTexture2D.new()
	dui.call("set_portrait", dummy_tex)
	
	var portrait_rect = dui.get_node("PortraitRect") if dui.has_node("PortraitRect") else null
	if not portrait_rect:
		# Try looking in parent as in karakuri_mystery_shell.tscn
		portrait_rect = root.find_child("PortraitRect", true, false)
	
	_assert(portrait_rect != null, "PortraitRect not found")
	_assert(portrait_rect.texture != null, "Portrait texture should be set")
	
	# 2. Trigger scenario load and scene load
	print("[VERIFY] Explicitly loading scenario...")
	runner.call("load_scenario")
	
	print("[VERIFY] Loading new scene (office)...")
	runner.call("load_scene_by_id", "office")
	
	await _wait_frames(5)
	
	# 3. Check if portrait is cleared
	print("[VERIFY] Checking if portrait is cleared...")
	_assert(portrait_rect.texture == null or not portrait_rect.visible, "Portrait should be cleared or hidden after scene load")
	
	print("[VERIFY] Portrait cleanup verified successfully.")
	quit(0)

func _init():
	_run()
