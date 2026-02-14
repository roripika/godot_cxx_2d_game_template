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

	# Door -> warehouse
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

	print("[KARAKURI_SMOKE] passed")
	quit(0)

