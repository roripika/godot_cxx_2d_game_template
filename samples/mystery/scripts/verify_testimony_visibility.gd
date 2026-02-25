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
	print("[VERIFY] Loading shell scene...")
	var shell_scene = load("res://samples/mystery/karakuri_mystery_shell.tscn")
	var root = shell_scene.instantiate()
	get_root().add_child(root)
	
	await _wait_frames(60) # Wait for initial transitions
	
	var runner = root.find_child("ScenarioRunner", true, false)
	_assert(runner != null, "Could not find ScenarioRunner")
	
	print("[VERIFY] Explicitly loading scenario...")
	runner.call("load_scenario")
	
	var testimony_ui = root.find_child("TestimonySystem", true, false)
	_assert(testimony_ui != null, "Could not find TestimonySystem")
	
	var dui = root.find_child("DialogueUI", true, false)
	_assert(dui != null, "Could not find DialogueUI")
	
	# Jump to confrontation
	print("[VERIFY] Jumping to confrontation scene...")
	runner.call("load_scene_by_id", "confrontation")
	await _wait_frames(40)
	
	var cur_id = runner.call("get_current_scene_id")
	print("[VERIFY] Current scene ID: ", cur_id)
	_assert(cur_id == "confrontation", "Runner did not jump to confrontation scene (current: " + str(cur_id) + ")")
	
	# Wait for intro dialogue to be visible
	print("[VERIFY] Waiting for intro dialogue...")
	for i in range(100):
		if dui.visible: break
		await _wait_frames(1)
	
	_assert(dui.visible, "Intro dialogue did not appear")
	print("[VERIFY] Dialogue text: ", dui.get("_message_text_key"))
	
	# Initial state: should be in confrontation mode, but testimony action NOT yet hit
	_assert(cur_id == "confrontation", "Not in confrontation scene")
	
	print("[VERIFY] Checking initial visibility during intro dialogue...")
	_assert(not testimony_ui.visible, "Testimony UI should be hidden during intro dialogue")
	print("[VERIFY] Success: Testimony UI is hidden.")
	
	# Progress through intro dialogues
	# 1st dialogue
	print("[VERIFY] Progressing dialogue 1...")
	if dui.get("is_typing"): dui.call("skip_typing")
	dui.emit_signal("dialogue_finished")
	await _wait_frames(60) # Wait for transition_object
	
	_assert(not testimony_ui.visible, "Testimony UI should still be hidden after 1st dialogue")
	
	# 2nd dialogue
	print("[VERIFY] Progressing dialogue 2...")
	for i in range(100):
		if dui.visible and (dui.get("is_typing") or dui.get("_waiting_for_click")): break
		await _wait_frames(1)
	
	if dui.get("is_typing"): dui.call("skip_typing")
	dui.emit_signal("dialogue_finished")
	
	# Now the 'testimony' action should start
	print("[VERIFY] Polling for Testimony UI visibility...")
	var visible_found := false
	for i in range(100):
		if testimony_ui.visible:
			visible_found = true
			break
		await _wait_frames(1)
	
	if not _assert(visible_found, "Testimony UI should be visible now"): return
	print("[VERIFY] Success: Testimony UI is now visible.")
	
	print("[VERIFY] ALL VISIBILITY TESTS PASSED.")
	quit(0)

func _init():
	_run()
