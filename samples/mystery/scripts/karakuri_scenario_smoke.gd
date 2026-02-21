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
	var dui := _dialogue_ui()
	dui.set("typing_speed", 0.0)

func _wait_for_base(name: String, max_frames: int = 2000) -> bool:
	print("[KARAKURI_SMOKE] waiting for base: ", name)
	return await _wait_until(
		func() -> bool:
			return _scene_container().get_child_count() > 0 and _scene_container().get_child(0).name == name,
		max_frames
	)

func _simulate_click() -> void:
	var event := InputEventMouseButton.new()
	event.button_index = MOUSE_BUTTON_LEFT
	event.pressed = true
	event.global_position = Vector2(640, 360) # screen center
	Input.parse_input_event(event)

	var event_up := InputEventMouseButton.new()
	event_up.button_index = MOUSE_BUTTON_LEFT
	event_up.pressed = false
	event_up.global_position = Vector2(640, 360)
	Input.parse_input_event(event_up)

func _wait_runner_idle(max_frames: int = 2000) -> bool:
	return await _wait_until(
		func() -> bool:
			var runner := _runner()
			return runner != null and not runner.call("is_running"),
		max_frames
	)

func _go_to_deduction(gs: Node) -> bool:
	var reached_warehouse := await _wait_for_base("WarehouseBase")
	if not reached_warehouse:
		return false
	await _wait_frames(60)

	# Prepare required inventory to avoid flaky hotspot timing in headless runs.
	gs.call("add_item", "ectoplasm")
	gs.call("add_item", "footprint")
	gs.call("add_item", "torn_memo")

	var wb := _scene_container().get_child(0)
	_interaction_manager().emit_signal("clicked_at", wb.get_node("hs_exit").global_position)
	
	# Skip the 2 exit dialogue lines
	for _i in range(5):
		await _wait_frames(60)
		_simulate_click()
		await _wait_frames(10)

	await _wait_runner_idle(2000)
	print("[KARAKURI_SMOKE] runner idle after clicking hs_exit")

	var reached_office := await _wait_for_base("OfficeBase", 2000)
	return reached_office

func _run() -> void:
	await _boot_shell()

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
	await _wait_frames(60)
	runner.call("on_choice_selected", 1, "")
	await _wait_frames(60)
	_assert(int(gs.call("get_health")) == hp_before_wrong - 1, "wrong deduction did not reduce HP")
	
	# Skip Ghost path dialogs: 'Boss: Response 1', 'Detective: Response 2'
	for _i in range(3):
		await _wait_frames(60)
		_simulate_click()

	await _wait_frames(120)

	runner.call("on_choice_selected", 0, "")

	var reached_confrontation := await _wait_for_base("WarehouseBase", 3000)
	_assert(reached_confrontation, "failed to transition to confrontation scene")
	
	# Skip Confrontation intro 'Detective: start1', 'Rat Witness: start2'
	for _i in range(4):
		await _wait_frames(60)
		_simulate_click()

	await _wait_runner_idle(2000)
	var present_button := current_scene.get_node("MainInfoUiLayer/TestimonySystem/VBoxContainer/ActionContainer/PresentButton")
	_set_locale("en")
	await _wait_frames(3)
	var en_present: String = str(present_button.text)
	_set_locale("ja")
	await _wait_frames(3)
	var ja_present: String = str(present_button.text)
	_assert(en_present != ja_present, "present button text did not switch locale during confrontation")

	# Wrong evidence once (after requesting present), then solve with correct evidence.
	var hp_before_testimony_wrong := int(gs.call("get_health"))
	runner.call("on_testimony_present_requested")
	await _wait_frames(5)
	runner.call("on_evidence_selected", "ectoplasm")
	await _wait_frames(20)
	_assert(int(gs.call("get_health")) == hp_before_testimony_wrong - 1, "wrong testimony evidence did not reduce HP")

	runner.call("on_testimony_next_requested")
	await _wait_frames(10)
	runner.call("on_testimony_present_requested")
	await _wait_frames(5)
	runner.call("on_evidence_selected", "footprint")
	await _wait_frames(15)
	runner.call("on_testimony_present_requested")
	await _wait_frames(5)
	runner.call("on_evidence_selected", "torn_memo")

	var reached_good_ending := await _wait_for_base("EndingBase", 2000)
	_assert(reached_good_ending, "good ending was not reached")

	# Failure branch: wrong deduction until HP reaches 0.
	gs.call("reset_game")
	await _boot_shell()
	ready_deduction = await _go_to_deduction(gs)
	_assert(ready_deduction, "failed to reach deduction on failure branch")
	await _wait_frames(60)
	for _i in range(3):
		runner = _runner()
		runner.call("on_choice_selected", 2, "")
		
		# Skip 'Boss: Alien Wrong 1', 'Detective: Retrying', 'Boss: Question'
		for _j in range(4):
			await _wait_frames(60)
			_simulate_click()
			
		await _wait_frames(120)
		await _wait_runner_idle(2000)

	var reached_bad_ending := await _wait_for_base("EndingBase", 3000)
	_assert(reached_bad_ending, "bad ending was not reached after HP depletion")

	if _failed:
		quit(1)
	else:
		print("[KARAKURI_SMOKE] passed")
		quit(0)
