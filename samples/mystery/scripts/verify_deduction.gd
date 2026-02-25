# verify_deduction.gd
# Automated test for the deduction scene hang.
# Usage: godot --headless --path . -s samples/mystery/scripts/verify_deduction.gd
extends SceneTree

const SHELL := "res://samples/mystery/karakuri_mystery_shell.tscn"

func _initialize() -> void:
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		push_error("[VERIFY] ASSERT FAILED: " + msg)
		quit(1)

func _wait_frames(n: int) -> void:
	for _i in range(n):
		await process_frame

func _run() -> void:
	print("[VERIFY] Starting deduction verification test...")
	
	# Load shell
	change_scene_to_file(SHELL)
	await _wait_frames(10)
	
	var shell = root.get_node("KarakuriMysteryShell")
	var runner = shell.get_node("ScenarioRunner")
	var gs = root.get_node("AdventureGameState")
	var dui = shell.get_node("MainInfoUiLayer/DialogueUI")
	
	# Set up state
	gs.call("add_item", "ectoplasm")
	gs.call("add_item", "footprint")
	gs.call("add_item", "torn_memo")
	gs.call("add_item", "witness_report")
	gs.call("set_flag", "talked_to_tanaka", true)
	gs.call("set_flag", "all_evidence_collected", true)
	
	# Jump to deduction
	print("[VERIFY] Jumping to deduction scene...")
	runner.call("load_scene_by_id", "deduction")
	await _wait_frames(10)
	
	# Transition might take time, check if we are in deduction
	var current_scene_id = runner.call("get_current_scene_id")
	_assert(current_scene_id == "deduction", "Failed to jump to deduction scene (ID: " + str(current_scene_id) + ")")
	
	# Wait for first dialogue
	print("[VERIFY] Waiting for Boss question...")
	for _i in range(100):
		if dui.get("_message_text_key") == "mystery.deduction.question":
			break
		await _wait_frames(1)
	
	_assert(dui.get("_message_text_key") == "mystery.deduction.question", "Deduction question did not show up")
	
	# Click to finish dialogue
	print("[VERIFY] Clicking to finish dialogue...")
	await _wait_frames(10)
	# Force finish for headless test reliability
	if dui.get("is_typing"):
		dui.call("skip_typing")
	dui.emit_signal("dialogue_finished")
	await _wait_frames(10)
	
	# Check for choices
	print("[VERIFY] Checking if choice buttons appeared...")
	var cc = dui.call("_get_choices_container")
	_assert(cc != null, "ChoicesContainer not found")
	
	var choice_appeared := false
	for _i in range(100):
		if cc.get_child_count() > 0:
			choice_appeared = true
			break
		await _wait_frames(1)
	
	_assert(choice_appeared, "Choice buttons did not appear after dialogue finished")
	print("[VERIFY] Success: Choice buttons appeared. Count: ", cc.get_child_count())
	
	# Click a choice
	var btn = cc.get_child(0) as Button
	print("[VERIFY] Clicking choice: ", btn.text)
	btn.emit_signal("pressed")
	await _wait_frames(10)
	
	# Verify transition started
	print("[VERIFY] Verifying transition to confrontation...")
	for _i in range(200):
		if runner.call("get_current_scene_id") == "confrontation":
			break
		# If it's a dialogue action within the choice branch, we might need to click through it
		if dui.get("visible") and (dui.get("is_typing") or dui.get("_waiting_for_click")):
			if dui.get("is_typing"):
				dui.call("skip_typing")
			dui.emit_signal("dialogue_finished")
		await _wait_frames(5)
		
	_assert(runner.call("get_current_scene_id") == "confrontation", "Failed to transition to confrontation after choice")
	
	print("[VERIFY] ALL TESTS PASSED.")
	quit(0)
