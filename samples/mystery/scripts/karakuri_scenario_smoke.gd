extends SceneTree

const SHELL := "res://samples/mystery/karakuri_mystery_shell.tscn"
const DIALOGUE_PLACEHOLDER := "ここに会話文が表示されます"
var _failed := false
var _seen_dialogue_signatures: Dictionary = {}

func _dialogue_text_label() -> Label:
	return _dialogue_ui().get_node_or_null("VBoxContainer/TextLabel")

func _collect_dialogue_snapshot() -> Dictionary:
	var ui := _dialogue_ui()
	var label := _dialogue_text_label()
	var label_text := ""
	if label:
		label_text = str(label.text).strip_edges()
	return {
		"message_key": str(ui.get("_message_text_key")).strip_edges(),
		"message_text": str(ui.get("_message_text")).strip_edges(),
		"current_text": str(ui.get("current_text")).strip_edges(),
		"label_text": label_text,
	}

func _best_dialogue_text(snapshot: Dictionary) -> String:
	var label_text := str(snapshot.get("label_text", ""))
	var current_text := str(snapshot.get("current_text", ""))
	var message_text := str(snapshot.get("message_text", ""))
	var message_key := str(snapshot.get("message_key", ""))
	if label_text != "":
		return label_text
	if current_text != "":
		return current_text
	if message_text != "":
		return message_text
	return message_key

func _track_dialogue_playback(context: String) -> bool:
	var snap := _collect_dialogue_snapshot()
	var best := _best_dialogue_text(snap).strip_edges()
	if best == "" or best == DIALOGUE_PLACEHOLDER:
		return false
	var sig := "%s|%s|%s|%s" % [
		str(snap.get("message_key", "")),
		str(snap.get("message_text", "")),
		str(snap.get("current_text", "")),
		str(snap.get("label_text", "")),
	]
	if not _seen_dialogue_signatures.has(sig):
		_seen_dialogue_signatures[sig] = context
		print("[KARAKURI_SMOKE] dialogue playback observed (", context, "): ", best)
	return true

func _wait_for_dialogue_playback(context: String, max_frames: int = 240) -> bool:
	for _i in range(max_frames):
		if _track_dialogue_playback(context):
			return true
		await process_frame
	return false

func _wait_for_dialogue_key(expected_key: String, max_frames: int = 240) -> bool:
	for _i in range(max_frames):
		var key := str(_dialogue_ui().get("_message_text_key")).strip_edges()
		if key == expected_key:
			_track_dialogue_playback("key:" + expected_key)
			return true
		await process_frame
	return false

func _initialize() -> void:
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		_failed = true
		push_error("[KARAKURI_SMOKE] " + msg)

func _wait_frames(n: int) -> void:
	for _i in range(n):
		await process_frame

func _wait_until(cond: Callable, max_frames: int) -> bool:
	for _i in range(max_frames):
		if cond.call():
			return true
		await process_frame
	return false

func _scene_container() -> Node:
	return current_scene.get_node("SceneContainer")

func _runner() -> Node:
	return current_scene.get_node("ScenarioRunner")

func _interaction_manager() -> Node:
	return current_scene.get_node("InteractionManager")

func _adventure_state() -> Node:
	return get_root().get_node("AdventureGameState")

func _dialogue_ui() -> Node:
	return current_scene.get_node("MainInfoUiLayer/DialogueUI")

func _set_locale(prefix: String) -> void:
	var service := get_root().get_node_or_null("KarakuriLocalization")
	if service and service.has_method("set_locale_prefix"):
		service.call("set_locale_prefix", prefix)
	else:
		TranslationServer.set_locale(prefix)

func _boot_shell() -> void:
	change_scene_to_file(SHELL)
	await _wait_frames(2)
	
	# Disable capture driver during smoke tests to avoid interference and headless errors
	var capture_driver = current_scene.get_node_or_null("UiCaptureDriver")
	if capture_driver:
		capture_driver.queue_free()
		await _wait_frames(1)

	var dui := _dialogue_ui()
	dui.set("typing_speed", 0.0)

func _inventory_ui() -> Node:
	return current_scene.get_node_or_null("InstantSubInfoUiLayer/InventoryUI")

func _test_inventory_ui(wb: Node) -> void:
	var inventory := _inventory_ui()
	if inventory == null:
		_assert(false, "InventoryUI node not found at InstantSubInfoUiLayer/InventoryUI")
		return

	# ── [TEST1] インベントリを開いて CloseButton で閉じられるか ──────────────
	print("[KARAKURI_SMOKE] testing inventory open/close via CloseButton...")
	inventory.call("show_inventory")
	await _wait_frames(3)
	_assert(inventory.visible == true, "inventory should be visible after show_inventory()")

	var close_btn_node := inventory.get_node_or_null("CloseButton")
	_assert(close_btn_node != null, "CloseButton not found inside InventoryUI")
	if close_btn_node:
		close_btn_node.emit_signal("pressed")
		await _wait_frames(3)
		_assert(inventory.visible == false, "inventory should be hidden after CloseButton pressed")

	# ── [TEST2] インベントリ表示中は NPC タップが無視されるか ──────────────
	print("[KARAKURI_SMOKE] testing NPC interaction blocked while inventory open...")
	inventory.call("show_inventory")
	await _wait_frames(3)
	_assert(inventory.visible == true, "inventory must be visible for NPC block test")

	# 表示中に Tanaka をクリック
	var tanaka_node := wb.get_node_or_null("hs_manager")
	if tanaka_node:
		_seen_dialogue_signatures.clear()  # 以前のログをリセット
		_interaction_manager().emit_signal("clicked_at", tanaka_node.global_position)
		await _wait_frames(20)
		# ランナーが動き出していない（被弾ダイアログが来ていない）ことを確認
		var runner := _runner()
		_assert(not runner.call("is_running"), "runner must not be running (NPC click should be blocked by inventory)")
		_assert(not _seen_dialogue_signatures.has("key:mystery.warehouse.tanaka.talk"),
			"Tanaka dialogue must NOT start while inventory is open")

	# テスト後は必ず閉じる
	inventory.call("hide_inventory")
	await _wait_frames(3)
	_assert(inventory.visible == false, "inventory should be hidden after hide_inventory()")

func _test_shell_inventory_button() -> void:
	print("[KARAKURI_SMOKE] testing shell inventory button toggle...")
	var shell = current_scene
	var inv_btn = shell.get_node_or_null("SystemUiLayer/InventoryButton")
	var inventory = _inventory_ui()
	
	_assert(inv_btn != null, "InventoryButton not found in SystemUiLayer")
	_assert(inventory != null, "InventoryUI not found")
	
	if inv_btn and inventory:
		inventory.call("hide_inventory")
		await _wait_frames(5)
		
		# Open via button
		inv_btn.emit_signal("pressed")
		await _wait_frames(5)
		_assert(inventory.visible == true, "inventory should be visible after shell button press")
		
		# Close via button
		inv_btn.emit_signal("pressed")
		await _wait_frames(5)
		_assert(inventory.visible == false, "inventory should be hidden after shell button press (toggle)")

func _test_mode_blocking(wb: Node) -> void:
	# ── [TEST1] 調査モード中に NPC (Tanaka) をクリックしても反応しないか ──────
	print("[KARAKURI_SMOKE] testing NPC (Tanaka) click blocked in Investigate Mode...")
	# 前提: 調査モードであること
	var npc_tanaka := wb.get_node_or_null("hs_manager")
	if npc_tanaka:
		_seen_dialogue_signatures.clear()
		_interaction_manager().emit_signal("clicked_at", npc_tanaka.global_position)
		await _wait_frames(20)
		_assert(not _seen_dialogue_signatures.has("key:mystery.warehouse.tanaka.talk"),
			"Tanaka dialogue must NOT start in Investigate Mode")

	# ── [TEST2] 会話モード中に 証拠品 (Floor) をクリックしても反応しないか ───────
	print("[KARAKURI_SMOKE] testing evidence (Floor) click blocked in Talk Mode...")
	# 会話モードへ切り替え
	var mode_btn := wb.get_node_or_null("ModeToggleButton")
	if mode_btn:
		mode_btn.emit_signal("pressed")
		await _wait_frames(10)
		_assert(wb.get_node("hs_manager").visible == true, "must be in Talk Mode")

		var hs_floor := wb.get_node_or_null("hs_floor_area")
		if hs_floor:
			_seen_dialogue_signatures.clear()
			_interaction_manager().emit_signal("clicked_at", hs_floor.global_position)
			await _wait_frames(20)
			# エクトプラズム取得メッセージが出ていないことを確認
			_assert(not _seen_dialogue_signatures.has("key:mystery.warehouse.intro"), # introは最初に出るので取得ダイアログ等のキーで判定すべきだが、Runnerが動かなければOK
				"Evidence interaction must NOT start in Talk Mode")
			var runner := _runner()
			_assert(not runner.call("is_running"), "runner must not be running from evidence click in Talk Mode")

		# 元の調査モードに戻す
		mode_btn.emit_signal("pressed")
		await _wait_frames(10)

func _wait_for_base(name: String, max_frames: int = 2000) -> bool:
	print("[KARAKURI_SMOKE] waiting for base: ", name)
	return await _wait_until(
		func() -> bool:
			return _scene_container().get_child_count() > 0 and _scene_container().get_child(0).name == name,
		max_frames
	)

func _wait_for_choices_with_clicks(max_frames: int = 4000) -> bool:
	print("[KARAKURI_SMOKE] waiting for choices...")
	var frames_since_click = 0
	for _i in range(max_frames):
		var choices = _dialogue_ui().get("_choice_defs")
		if choices != null and choices.size() > 0:
			return true
		frames_since_click += 1
		if frames_since_click >= 30:
			var event := InputEventMouseButton.new()
			event.button_index = MOUSE_BUTTON_LEFT
			event.pressed = true
			event.global_position = Vector2(100, 100)
			Input.parse_input_event(event)
			var event_up := InputEventMouseButton.new()
			event_up.button_index = MOUSE_BUTTON_LEFT
			event_up.pressed = false
			event_up.global_position = Vector2(100, 100)
			Input.parse_input_event(event_up)
			frames_since_click = 0
		await process_frame
	return false

func _wait_for_base_with_clicks(name: String, click_interval_frames: int = 30, max_frames: int = 4000) -> bool:
	print("[KARAKURI_SMOKE] waiting for base (with clicks): ", name)
	var frames_since_click = 0
	for _i in range(max_frames):
		if _scene_container().get_child_count() > 0:
			var cname = _scene_container().get_child(0).name
			if cname == name:
				return true
			if _i % 30 == 0:
				print("[KARAKURI_SMOKE] currently loaded child: ", cname)
		frames_since_click += 1
		if frames_since_click >= click_interval_frames:
			_safe_clear_dialogue()
			frames_since_click = 0
		await process_frame
	return false

func _safe_clear_dialogue() -> void:
	var ui := _dialogue_ui()
	_track_dialogue_playback("clear_cycle")
	if ui.get("is_typing"):
		ui.call("skip_typing")
	if ui.get("_waiting_for_click"):
		ui.set("_waiting_for_click", false)
		ui.emit_signal("dialogue_finished")

func _wait_runner_idle(max_frames: int = 2000) -> bool:
	return await _wait_until(
		func() -> bool:
			var runner := _runner()
			return runner != null and not runner.call("is_running"),
		max_frames
	)

func _go_to_deduction(gs: Node) -> bool:
	var reached_warehouse := await _wait_for_base_with_clicks("WarehouseBase")
	if not reached_warehouse:
		return false

	await _wait_for_dialogue_key("mystery.warehouse.intro", 600)
	print("[KARAKURI_SMOKE] warehouse intro detected. clearing...")
	
	for _i in range(12):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if await _wait_runner_idle(20): break
	
	await _wait_frames(60)
	var wb := _scene_container().get_child(0)

	# --- [TEST] Mode Blocking (Investigate mode should block NPC, Talk mode should block Evidence) ---
	await _test_mode_blocking(wb)

	# --- [TEST] Mode Toggle ---
	print("[KARAKURI_SMOKE] testing mode toggle (Investigate -> Talk)...")
	var mode_btn := wb.get_node("ModeToggleButton")
	var btn_text_investigate: String = str(mode_btn.get("text"))
	mode_btn.emit_signal("pressed")
	await _wait_frames(10)
	_assert(wb.get_node("hs_manager").visible == true, "NPC (Tanaka) should be visible in Talk Mode")
	_assert(mode_btn.text != btn_text_investigate, "ModeToggleButton label should change when switching to Talk mode")

	# --- [TEST] NPC Interaction (Tanaka - Required for Footprints) ---
	print("[KARAKURI_SMOKE] testing NPC interaction (Tanaka)...")
	_interaction_manager().emit_signal("clicked_at", wb.get_node("hs_manager").global_position)
	_assert(await _wait_for_dialogue_key("mystery.warehouse.tanaka.talk", 300), "Tanaka dialogue did not start")
	
	for _i in range(15):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if await _wait_runner_idle(20): break
	
	_assert(gs.call("get_flag", "talked_to_tanaka"), "Tanaka flag was not set")

	# --- [TEST] NPC Interaction (Sato) ---
	print("[KARAKURI_SMOKE] testing NPC interaction (Sato)...")
	_interaction_manager().emit_signal("clicked_at", wb.get_node("hs_worker").global_position)
	_assert(await _wait_for_dialogue_key("mystery.warehouse.sato.talk", 300), "Sato dialogue did not start")
	
	for _i in range(20):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if await _wait_runner_idle(20): break
	
	_assert(gs.call("has_item", "ectoplasm"), "failed to get ectoplasm from Sato")

	# --- [TEST] Inventory: open/close & NPC blocking while open ---
	await _test_inventory_ui(wb)

	# --- [TEST] Mode Toggle Back ---
	print("[KARAKURI_SMOKE] testing mode toggle (Talk -> Investigate)...")
	var btn_text_talk: String = str(mode_btn.get("text"))
	mode_btn.emit_signal("pressed")
	await _wait_frames(10)
	_assert(wb.get_node("hs_worker").visible == false, "NPC should be hidden in Investigate Mode")
	_assert(mode_btn.text != btn_text_talk, "ModeToggleButton label should change when switching back to Investigate mode")

	# --- [TEST] Investigation Mode: Collecting Evidence ---
	print("[KARAKURI_SMOKE] testing investigation: collecting footprints...")
	_interaction_manager().emit_signal("clicked_at", wb.get_node("hs_footprints").global_position)
	_assert(await _wait_for_dialogue_key("mystery.warehouse.footprint.found", 300), "footprint dialogue did not start")
	
	for _i in range(10):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if await _wait_runner_idle(20): break
	
	_assert(gs.call("has_item", "footprint"), "failed to collect footprint via click")

	# Prepare remaining items
	gs.call("add_item", "torn_memo")
	gs.call("add_item", "delivery_log")
	gs.call("add_item", "witness_report")

	print("[KARAKURI_SMOKE] items ready. clicking exit...")
	# give_evidence auto-shows the inventory; close it before clicking the exit hotspot.
	var inv := _inventory_ui()
	if inv and inv.visible:
		inv.call("hide_inventory")
	await _wait_frames(100) # Give more time for idle
	_interaction_manager().emit_signal("clicked_at", wb.get_node("hs_exit").global_position)
	
	for _i in range(20):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if await _wait_runner_idle(20): break

	var reached_office := await _wait_for_base_with_clicks("OfficeBase", 30, 2000)
	return reached_office

func _run() -> void:
	await _boot_shell()
	var has_prologue_dialogue := await _wait_for_dialogue_key("mystery.prologue.system", 600)
	_assert(has_prologue_dialogue, "prologue dialogue was not rendered")

	# Skip prologue dialogue (Ren, Yui, Boss)
	for _i in range(15):
		await _wait_frames(20)
		_safe_clear_dialogue()

	# Locale switch sanity: EN/JA translation must differ.
	_set_locale("en")
	var en_title: String = str(TranslationServer.translate("office_title"))
	_set_locale("ja")
	var ja_title: String = str(TranslationServer.translate("office_title"))
	_assert(en_title != ja_title, "office_title translation does not change by locale")

	var gs := _adventure_state()
	_assert(gs != null, "AdventureGameState autoload missing")

	var ready_deduction := await _go_to_deduction(gs)
	_assert(ready_deduction, "failed to reach deduction")

	var runner := _runner()
	var hp_before_wrong := int(gs.call("get_health"))
	
	await _wait_for_choices_with_clicks(4000)
	
	# Give 5 frames for the choice UI to fully show
	await _wait_frames(5)
	runner.call("on_choice_selected", 1, "")
	
	# The wrong choice has dialogue, takes damage, then loops back to the choice menu.
	# So we just wait for the choice menu to reappear! It will click through the dialogues automatically.
	await _wait_for_choices_with_clicks(4000)
	
	# After returning to the choice menu, damage must have been taken!
	_assert(int(gs.call("get_health")) == hp_before_wrong - 1, "wrong deduction did not reduce HP")

	await _wait_frames(5)
	runner.call("on_choice_selected", 0, "")

	var reached_confrontation := await _wait_for_base_with_clicks("WarehouseBase", 30, 3000)
	_assert(reached_confrontation, "failed to transition to confrontation scene")

	var has_confrontation_intro := await _wait_for_dialogue_key("mystery.confrontation.start1", 600)
	_assert(has_confrontation_intro, "confrontation intro dialogue was not rendered")
	
	# Skip Confrontation intro 'Detective: start1', 'Rat Witness: start2'
	print("[KARAKURI_SMOKE] clearing confrontation intro...")
	for _i in range(10):
		await _wait_frames(30)
		_safe_clear_dialogue()
	await _wait_frames(30)
	var present_button := current_scene.get_node("MainInfoUiLayer/TestimonySystem/VBoxContainer/ActionContainer/PresentButton")
	_set_locale("en")
	await _wait_frames(10)
	var en_present: String = str(present_button.text)
	_set_locale("ja")
	await _wait_frames(10)
	var ja_present: String = str(present_button.text)
	_assert(en_present != ja_present, "present button text did not switch locale during confrontation")

	# Wrong evidence once (after requesting present), then solve with correct evidence.
	var hp_before_testimony_wrong := int(gs.call("get_health"))
	runner.call("on_testimony_present_requested")
	await _wait_frames(15)
	runner.call("on_evidence_selected", "ectoplasm")
	
	# The wrong evidence has a dialogue and then takes damage. We click through it.
	print("[KARAKURI_SMOKE] clicking through wrong testimony...")
	for _i in range(10):
		await _wait_frames(30)
		_safe_clear_dialogue()
		
	_assert(int(gs.call("get_health")) == hp_before_testimony_wrong - 1, "wrong testimony evidence did not reduce HP")

	runner.call("on_testimony_next_requested")
	print("[KARAKURI_SMOKE] clicking through round 1 statement...")
	for _i in range(5):
		await _wait_frames(30)
		_safe_clear_dialogue()
	
	# Present footprint for round 1 (line 2)
	runner.call("on_testimony_present_requested")
	await _wait_frames(15)
	runner.call("on_evidence_selected", "footprint")
	
	for _i in range(10):
		await _wait_frames(30)
		_safe_clear_dialogue()
		
	# The engine automatically advanced to round 2 (line 3) after correct evidence.
	# Present witness_report for round 2 (line 3)
	runner.call("on_testimony_present_requested")
	await _wait_frames(15)
	
	runner.call("on_evidence_selected", "witness_report")
	
	# Click through the 'correct_evidence' and success dialogues until EndingBase is loaded
	print("[KARAKURI_SMOKE] clicking through correct testimony (and on_success dialogue)...")
	for _i in range(500):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if _scene_container().get_child_count() > 0:
			if _scene_container().get_child(0).name == "EndingBase":
				break

	print("[KARAKURI_SMOKE] waiting for good ending check...")
	var reached_good_ending := _scene_container().get_child_count() > 0 and _scene_container().get_child(0).name == "EndingBase"
	_assert(reached_good_ending, "good ending was not reached")

	# Failure branch: pass deduction, then fail testimony
	gs.call("reset_game")
	await _boot_shell()
	ready_deduction = await _go_to_deduction(gs)
	_assert(ready_deduction, "failed to reach deduction on failure branch")
	
	await _wait_for_choices_with_clicks(4000)
	runner = _runner()
	runner.call("on_choice_selected", 0, "") # Correct
	
	# Wait for confrontation
	print("[KARAKURI_SMOKE] moving to confrontation for failure branch...")
	for _i in range(100):
		await _wait_frames(30)
		_safe_clear_dialogue()
		if _scene_container().get_child_count() > 0:
			if _scene_container().get_child(0).name == "WarehouseBase":
				break
	
	var has_failure_confrontation_intro := await _wait_for_dialogue_key("mystery.confrontation.start1", 600)
	_assert(has_failure_confrontation_intro, "failure branch confrontation intro was not rendered")

	# Clicking through intro
	for _i in range(8):
		await _wait_frames(30)
		_safe_clear_dialogue()

	# Wrong evidence until death
	print("[KARAKURI_SMOKE] providing wrong evidence until death...")
	for _i in range(12):
		runner.call("on_testimony_present_requested")
		await _wait_frames(15)
		runner.call("on_evidence_selected", "ectoplasm") # Wrong
		for _j in range(8):
			await _wait_frames(10)
			_safe_clear_dialogue()
		if _scene_container().get_child_count() > 0:
			if _scene_container().get_child(0).name == "EndingBase":
				break

	for _i in range(120):
		if _scene_container().get_child_count() > 0 and _scene_container().get_child(0).name == "EndingBase":
			break
		await _wait_frames(5)
		_safe_clear_dialogue()

	print("[KARAKURI_SMOKE] waiting for bad ending check...")
	var reached_bad_ending := _scene_container().get_child_count() > 0 and _scene_container().get_child(0).name == "EndingBase"
	_assert(reached_bad_ending, "bad ending was not reached after HP depletion")
	_assert(_seen_dialogue_signatures.size() >= 3, "dialogue playback observations were too few; dialogue may not be rendering")

	if _failed:
		quit(1)
	else:
		print("[KARAKURI_SMOKE] passed")
		quit(0)
