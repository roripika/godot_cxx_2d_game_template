# karakuri_scenario_smoke.gd
# Headless smoke test for the Karakuri YAML-driven scenario runner.
#
# Usage:
#   godot --headless --path . --script res://samples/mystery/scripts/karakuri_scenario_smoke.gd
extends SceneTree

const SHELL := "res://samples/mystery/karakuri_mystery_shell.tscn"

func _initialize() -> void:
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		push_error("[KARAKURI_SMOKE] " + msg)
		quit(1)

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
	await _wait_frames(10)

	_assert(current_scene != null, "current_scene is null")
	_assert(_scene_container().get_child_count() == 1, "expected 1 base scene instance")
	_assert(_scene_container().get_child(0).name == "OfficeBase", "expected OfficeBase loaded first")

	var im := _interaction_manager()
	_assert(im.has_signal("clicked_at"), "InteractionManager has no clicked_at")

	# Door -> warehouse (investigation)
	im.emit_signal("clicked_at", Vector2(960, 360))
	await _wait_frames(10)
	_assert(_scene_container().get_child(0).name == "WarehouseBase", "expected WarehouseBase after door click")

	# Collect evidence via hotspots.
	im.emit_signal("clicked_at", Vector2(520, 380))
	await _wait_frames(10)
	im.emit_signal("clicked_at", Vector2(330, 450))
	await _wait_frames(10)
	im.emit_signal("clicked_at", Vector2(740, 310))
	await _wait_frames(10)

	var gs := _adventure_state()
	_assert(gs.call("has_item", "ectoplasm"), "ectoplasm missing")
	_assert(gs.call("has_item", "footprint"), "footprint missing")
	_assert(gs.call("has_item", "torn_memo"), "torn_memo missing")

	# Exit should now return to office and set all_evidence_collected.
	im.emit_signal("clicked_at", Vector2(80, 324))
	await _wait_frames(15)
	_assert(_scene_container().get_child(0).name == "OfficeBase", "expected OfficeBase after exit")
	_assert(gs.call("get_flag", "all_evidence_collected", false), "all_evidence_collected not set")

	# Boss -> deduction scene.
	im.emit_signal("clicked_at", Vector2(180, 360))
	await _wait_frames(20)
	_assert(_scene_container().get_child(0).name == "OfficeDeductionBase", "expected OfficeDeductionBase after boss click")

	# Pick the correct deduction choice (index 0).
	var dui := current_scene.get_node("CanvasLayer/DialogueUI")
	var cc := dui.get_node("VBoxContainer/ChoicesContainer")
	var waited := 0
	while waited < 200 and cc.get_child_count() == 0:
		await process_frame
		waited += 1
	_assert(cc.get_child_count() > 0, "deduction choices did not appear")
	dui._on_choice_selected(0, "")
	await _wait_frames(30)
	_assert(_scene_container().get_child(0).name == "WarehouseConfrontationBase", "expected WarehouseConfrontationBase after deduction")

	# Force-win the testimony quickly by presenting the correct evidence for each statement.
	var ts := current_scene.get_node("CanvasLayer/TestimonySystem")
	waited = 0
	while waited < 200 and int(ts.get("testimonies").size()) < 3:
		await process_frame
		waited += 1
	_assert(int(ts.get("testimonies").size()) >= 3, "testimonies not populated")

	ts._check_evidence("footprint")
	await create_timer(1.2).timeout
	ts._check_evidence("torn_memo")
	await create_timer(1.2).timeout
	ts._check_evidence("ectoplasm")
	await create_timer(2.0).timeout

	_assert(_scene_container().get_child(0).name == "EndingBase", "expected EndingBase after testimony completion")

	print("[KARAKURI_SMOKE] passed")
	quit(0)
