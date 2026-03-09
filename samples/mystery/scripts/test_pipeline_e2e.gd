extends SceneTree

func _initialize():
	# Wait a few frames for Autoloads to enter the tree and call _ready()
	for i in range(5):
		await process_frame

	# ActionRegistry and MysteryGameState should be ready.
	var reg = Engine.get_singleton("ActionRegistry")
	if reg == null:
		printerr("[FAIL] ActionRegistry singleton not found")
		quit(1)
		return

	var mgs = MysteryGameState.get_singleton()
	if mgs == null:
		printerr("[FAIL] MysteryGameState not found")
		quit(1)
		return

	print("[PASS] ActionRegistry and MysteryGameState are ready")

	# Verify the action registration (done in MysteryGameState._ready)
	if not reg.has_action("add_evidence"):
		printerr("[FAIL] add_evidence action not registered")
		quit(1)
		return
	print("[PASS] add_evidence action is registered: ", reg.get_registered_actions())

	# --- UI Verification ---
	var test_scenario_path = "res://samples/mystery/scenario/test_ui_integration.json"
	
	print("--- E2E Test: UI Integration ---")
	
	# Signal capture flags
	var flags = {
		"portrait": false,
		"background": false
	}
	
	mgs.portrait_requested.connect(func(id, emo): 
		print("[Test] portrait_requested signal received: ", id)
		if id == "detective":
			flags["portrait"] = true
	)
	mgs.background_requested.connect(func(id): 
		print("[Test] background_requested signal received: ", id)
		if id == "office":
			flags["background"] = true
	)
	var inv_scene = load("res://samples/mystery/ui/evidence_inventory_ui.tscn")
	var inv_ui = inv_scene.instantiate()
	get_root().add_child(inv_ui)
	# Wait for UI _ready
	await process_frame
	
	print("[PASS] InventoryUI instanced and added to tree")

	# Now manually trigger the action via ScenarioRunner logic (replicated)
	var task = reg.create_task("add_evidence")
	if task == null:
		printerr("[FAIL] Failed to create TaskAddEvidence")
		quit(1)
		return
	
	task.set("evidence_id", "footprint")
	task.on_start()
	
	# Verify that MysteryGameState now has the evidence
	if mgs.has_evidence("footprint"):
		print("[PASS] Evidence 'footprint' was correctly added to MysteryGameState")
	else:
		printerr("[FAIL] Evidence 'footprint' not found in MysteryGameState")
		quit(1)
		return

	# Verify UI reflected the change
	if inv_ui.has_evidence("footprint"):
		print("[PASS] InventoryUI correctly reflected 'footprint'")
	else:
		printerr("[FAIL] InventoryUI did NOT reflect 'footprint'")
		quit(1)
		return

	# 4. Trigger Portrait and Background actions
	var portrait_task = reg.create_task("show_portrait")
	portrait_task.set("character_id", "detective")
	portrait_task.on_start()
	
	var bg_task = reg.create_task("change_background")
	bg_task.set("background_id", "office")
	bg_task.on_start()

	# 5. Verify Portrait and Background signals
	if not flags["portrait"]:
		printerr("[FAIL] portrait_requested signal not received or incorrect ID")
		quit(1)
		return
	if not flags["background"]:
		printerr("[FAIL] background_requested signal not received or incorrect ID")
		quit(1)
		return
	print("[PASS] Portrait and Background signals verified")

	print("\n--- ALL E2E TESTS PASSED ---")
	quit(0)
