extends SceneTree

func _init():
	print("[VERIFY] Starting Warehouse Mode NPC Visibility Test...")
	
	var scene = load("res://samples/mystery/warehouse_base.tscn").instantiate()
	root.add_child(scene)
	
	# Load Dialogue UI to avoid errors during mode switch
	var dialogue_ui = Control.new()
	dialogue_ui.set_script(load("res://samples/mystery/scripts/dialogue_ui_advanced.gd"))
	dialogue_ui.name = "DialogueUI"
	root.add_child(dialogue_ui)
	
	await process_frame
	await process_frame
	
	var hs_manager = scene.get_node_or_null("hs_manager")
	if not hs_manager:
		print("[VERIFY] ERROR: hs_manager not found!")
		quit(1)
		return
		
	# Assume initially TALK mode, hs_manager is visible. Let's explicitly set it visible.
	hs_manager.visible = true
	print("[VERIFY] Initial hs_manager.visible = ", hs_manager.visible)
	
	# Switch to INVESTIGATE mode
	print("[VERIFY] Switching to INVESTIGATE mode...")
	scene.current_mode = 0 # WarehouseMode.INVESTIGATE
	scene._apply_mode()
	
	if hs_manager.visible:
		print("[VERIFY] ERROR: hs_manager is still visible in INVESTIGATE mode!")
		quit(1)
		return
	print("[VERIFY] Success: hs_manager is hidden in INVESTIGATE mode.")
	
	# Switch back to TALK mode
	print("[VERIFY] Switching back to TALK mode...")
	scene.current_mode = 1 # WarehouseMode.TALK
	scene._apply_mode()
	
	if not hs_manager.visible:
		print("[VERIFY] ERROR: hs_manager did not restore visibility in TALK mode!")
		quit(1)
		return
	print("[VERIFY] Success: hs_manager restored visibility in TALK mode.")
	
	print("[VERIFY] ALL WAREHOUSE MODE TESTS PASSED.")
	quit(0)
