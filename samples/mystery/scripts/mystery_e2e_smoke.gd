# mystery_e2e_smoke.gd
# Headless end-to-end smoke test for the Mystery demo flow.
#
# Usage:
#   godot --headless --path . --script res://samples/mystery/scripts/mystery_e2e_smoke.gd
extends SceneTree

const OFFICE_SCENE := "res://samples/mystery/office_scene.tscn"
const WAREHOUSE_INVESTIGATION_SCENE := "res://samples/mystery/warehouse_investigation.tscn"
const OFFICE_DEDUCTION_SCENE := "res://samples/mystery/office_deduction.tscn"
const WAREHOUSE_CONFRONTATION_SCENE := "res://samples/mystery/warehouse_confrontation.tscn"
const ENDING_SCENE := "res://samples/mystery/ending.tscn"

func _initialize() -> void:
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		push_error("[SMOKE] ASSERT FAILED: " + msg)
		quit(1)

func _wait_frames(n: int) -> void:
	for _i in range(n):
		await process_frame

func _run() -> void:
	# Locale smoke (this is the simplest way to confirm toggle works in CLI/headless).
	TranslationServer.set_locale("en")
	var en_office := TranslationServer.translate("office_title")
	TranslationServer.set_locale("ja")
	var ja_office := TranslationServer.translate("office_title")
	_assert(en_office != ja_office, "Translation does not change across locales for key office_title")

	await _wait_frames(1)
	var game_state := root.get_node_or_null("AdventureGameState")
	_assert(game_state != null, "Autoload /root/AdventureGameState not found")

	# Start at office.
	change_scene_to_file(OFFICE_SCENE)
	await _wait_frames(5)
	_assert(current_scene != null, "current_scene is null after loading office")

	# Go to warehouse investigation via Office door click.
	if current_scene.has_method("_on_clicked_at"):
		current_scene._on_clicked_at(Vector2(910, 210))
	await create_timer(1.3).timeout
	_assert(current_scene != null and current_scene.scene_file_path == WAREHOUSE_INVESTIGATION_SCENE, "Did not transition to warehouse investigation")

	# Collect all evidence by directly triggering hotspot manager hit tests.
	await _wait_frames(10) # allow _ready() to build hotspots
	var hm = current_scene.get_node_or_null("HotspotManager")
	_assert(hm != null, "HotspotManager not found in warehouse investigation")
	var hs_arr = hm.get("hotspots")
	_assert(hs_arr != null and hs_arr is Array and hs_arr.size() >= 3, "HotspotManager.hotspots is missing or empty")
	for hs in hs_arr:
		if hs == null:
			continue
		var hid = str(hs.get("id"))
		if hid == "floor_area" or hid == "footprints" or hid == "memo":
			hm._trigger_hotspot(hs)
	await _wait_frames(10)
	_assert(bool(game_state.has_item("ectoplasm")), "ectoplasm not in inventory after hotspot click")
	_assert(bool(game_state.has_item("footprint")), "footprint not in inventory after hotspot click")
	_assert(bool(game_state.has_item("torn_memo")), "torn_memo not in inventory after hotspot click")
	_assert(bool(game_state.get_flag("all_evidence_collected")), "all_evidence_collected was not set after collecting evidence")

	# Exit back to office.
	if current_scene.has_method("_exit_warehouse"):
		current_scene._exit_warehouse()
	await create_timer(1.3).timeout
	_assert(current_scene != null and current_scene.scene_file_path == OFFICE_SCENE, "Did not return to office")

	# Talk to boss to go to deduction.
	if current_scene.has_method("_on_clicked_at"):
		current_scene._on_clicked_at(Vector2(120, 240))
	await create_timer(1.1).timeout
	_assert(current_scene != null and current_scene.scene_file_path == OFFICE_DEDUCTION_SCENE, "Did not transition to office deduction")

	# Wait for choices to show, then select the correct one (index 0).
	var dui = current_scene.get_node_or_null("CanvasLayer/DialogueUI")
	_assert(dui != null, "DialogueUI not found in office deduction")
	_assert(dui.has_method("show_choices"), "DialogueUIAdvanced.show_choices is missing on deduction DialogueUI")
	# show_choices is triggered after a couple of timers; wait until buttons exist.
	var cc = dui.get_node_or_null("VBoxContainer/ChoicesContainer")
	var waited_s := 0.0
	while waited_s < 6.0:
		await create_timer(0.1).timeout
		waited_s += 0.1
		if cc != null and cc.get_child_count() > 0:
			break
	_assert(cc != null and cc.get_child_count() > 0, "Choice buttons did not appear in deduction")
	if dui.has_method("_on_choice_selected"):
		dui._on_choice_selected(0, "")
	await create_timer(2.8).timeout
	_assert(current_scene != null and current_scene.scene_file_path == WAREHOUSE_CONFRONTATION_SCENE, "Did not transition to warehouse confrontation")

	# Solve all contradictions (3 statements) directly.
	var ts = current_scene.get_node_or_null("CanvasLayer/TestimonySystem")
	_assert(ts != null, "TestimonySystem not found in confrontation")
	# Wait until testimonies are populated by the scene controller.
	var waited_ts := 0.0
	while waited_ts < 6.0:
		await create_timer(0.1).timeout
		waited_ts += 0.1
		var arr = ts.get("testimonies")
		if arr != null and arr is Array and arr.size() >= 3:
			break
	var arr2 = ts.get("testimonies")
	_assert(arr2 != null and arr2 is Array and arr2.size() >= 3, "Testimonies were not populated")

	ts._check_evidence("footprint")
	await create_timer(1.2).timeout
	ts._check_evidence("torn_memo")
	await create_timer(1.2).timeout
	ts._check_evidence("ectoplasm")
	await create_timer(2.5).timeout
	_assert(current_scene != null and current_scene.scene_file_path == ENDING_SCENE, "Did not transition to ending after victory")

	# Ending: pick "Back to Menu" (index 1) so we don't loop.
	await create_timer(3.0).timeout
	var end_ui = current_scene.get_node_or_null("CanvasLayer/DialogueUI")
	_assert(end_ui != null, "DialogueUI not found in ending")
	if end_ui.has_method("_on_choice_selected"):
		end_ui._on_choice_selected(1, "")
	await create_timer(0.8).timeout

	print("[SMOKE] Mystery e2e smoke test passed.")
	quit(0)
