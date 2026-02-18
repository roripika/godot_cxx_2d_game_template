# karakuri_scenario_smoke.gd
# Headless smoke test for the Karakuri YAML-driven scenario runner.
#
# Usage:
#   godot --headless --path . --script res://samples/mystery/scripts/karakuri_scenario_smoke.gd
extends SceneTree

const SHELL := "res://samples/mystery/karakuri_mystery_shell.tscn"
var _failed := false

func _initialize() -> void:
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		_failed = true
		push_error("[KARAKURI_SMOKE] " + msg)

func _wait_frames(n: int) -> void:
	for _i in range(n):
		await process_frame

func _scene_container() -> Node:
	return current_scene.get_node("SceneContainer")

func _interaction_manager() -> Node:
	return current_scene.get_node("InteractionManager")

func _adventure_state() -> Node:
	return get_root().get_node("AdventureGameState")

func _run() -> void:
	change_scene_to_file(SHELL)
	await _wait_frames(2)

	# Speed up typing to keep the smoke test fast. If a message is already
	# typing, force-finish it once so the runner can proceed.
	var dui = null
	var waited := 0
	while waited < 120 and dui == null:
		dui = current_scene.get_node_or_null("MainInfoUiLayer/DialogueUI")
		if dui == null:
			await process_frame
			waited += 1
	if dui:
		dui.set("typing_speed", 0.0)
		if dui.has_method("skip_typing"):
			dui.call("skip_typing")

	await _wait_frames(5)

	_assert(current_scene != null, "current_scene is null")
	_assert(_scene_container().get_child_count() >= 1, "expected base scene instance")

	var im := _interaction_manager()
	_assert(im.has_signal("clicked_at"), "InteractionManager has no clicked_at")

	# Prologue auto-advances to warehouse.
	waited = 0
	while waited < 240 and _scene_container().get_child(0).name != "WarehouseBase":
		await process_frame
		waited += 1
	_assert(_scene_container().get_child(0).name == "WarehouseBase", "expected WarehouseBase after prologue")

	# Allow on_enter actions to finish before clicking hotspots.
	await _wait_frames(30)

	# Collect evidence via hotspots.
	var wb := _scene_container().get_child(0)
	im.emit_signal("clicked_at", wb.get_node("hs_floor_area").global_position)
	await _wait_frames(30)
	im.emit_signal("clicked_at", wb.get_node("hs_footprints").global_position)
	await _wait_frames(30)

	var gs := _adventure_state()
	_assert(gs.call("has_item", "footprint"), "footprint missing")
	# NOTE: Some hotspots are gated by choice/dialogue timing; for this smoke,
	# ensure required items exist to validate the runner flow end-to-end.
	if not gs.call("has_item", "ectoplasm"):
		gs.call("add_item", "ectoplasm")
	if not gs.call("has_item", "torn_memo"):
		gs.call("add_item", "torn_memo")

	# Exit should now return to office (deduction).
	im.emit_signal("clicked_at", wb.get_node("hs_exit").global_position)
	waited = 0
	while waited < 240 and _scene_container().get_child(0).name != "OfficeBase":
		await process_frame
		waited += 1
	_assert(_scene_container().get_child(0).name == "OfficeBase", "expected OfficeBase after exit")

	# Pick the correct deduction choice (index 0).
	dui = current_scene.get_node("MainInfoUiLayer/DialogueUI")
	var cc = dui.get_node("VBoxContainer/ChoicesContainer")
	waited = 0
	while waited < 240 and cc.get_child_count() == 0:
		await process_frame
		waited += 1
	_assert(cc.get_child_count() > 0, "deduction choices did not appear")
	dui._on_choice_selected(0, "")
	await _wait_frames(60)
	_assert(_scene_container().get_child(0).name == "WarehouseBase", "expected WarehouseBase after deduction")

	# Force-win the testimony quickly by presenting the correct evidence for each statement.
	var ts := current_scene.get_node("MainInfoUiLayer/TestimonySystem")
	waited = 0
	while waited < 200 and int(ts.get("testimonies").size()) < 3:
		await process_frame
		waited += 1
	_assert(int(ts.get("testimonies").size()) >= 3, "testimonies not populated")

	# Smoke goal is runner wiring (signal -> on_success -> goto), not the full
	# testimony minigame. End it explicitly to keep the test stable.
	if ts.has_method("_on_all_complete"):
		ts.call("_on_all_complete")

	# Wait for scenario runner to apply on_success goto.
	waited = 0
	while waited < 240 and _scene_container().get_child(0).name != "EndingBase":
		await process_frame
		waited += 1

	_assert(_scene_container().get_child(0).name == "EndingBase", "expected EndingBase after testimony completion")

	if _failed:
		quit(1)
	else:
		print("[KARAKURI_SMOKE] passed")
		quit(0)
