extends SceneTree

func _init():
	print("[VERIFY] Starting Warehouse Portrait Visibility Test...")
	
	# Load the warehouse scene
	var scene = load("res://samples/mystery/warehouse_base.tscn").instantiate()
	root.add_child(scene)
	
	# Load Dialogue UI
	var dialogue_ui = Control.new()
	dialogue_ui.set_script(load("res://samples/mystery/scripts/dialogue_ui_advanced.gd"))
	dialogue_ui.name = "DialogueUI"
	root.add_child(dialogue_ui)
	
	# Wait for a frame to let Godot call _ready() on added nodes
	await process_frame
	await process_frame
	
	var hs_manager = scene.get_node_or_null("hs_manager") # Tanaka
	if not hs_manager:
		print("[VERIFY] ERROR: hs_manager not found!")
		quit(1)
		return
		
	# Assume initially visible for the test
	hs_manager.visible = true
	print("[VERIFY] Initial hs_manager.visible = ", hs_manager.visible)
	
	# Emulate portrait shown for Tanaka
	print("[VERIFY] Emitting portrait_shown('tanaka')...")
	dialogue_ui.portrait_shown.emit("tanaka")
	await process_frame
	
	if hs_manager.visible:
		print("[VERIFY] ERROR: hs_manager is still visible after portrait_shown('tanaka')!")
		quit(1)
		return
	print("[VERIFY] Success: hs_manager (Tanaka) is hidden.")
	
	# Emulate portrait hidden
	print("[VERIFY] Emitting portrait_hidden()...")
	dialogue_ui.portrait_hidden.emit()
	await process_frame
	
	if not hs_manager.visible:
		print("[VERIFY] ERROR: hs_manager is still hidden after portrait_hidden()!")
		quit(1)
		return
	print("[VERIFY] Success: hs_manager (Tanaka) is visible again.")
	
	print("[VERIFY] ALL WAREHOUSE PORTRAIT TESTS PASSED.")
	quit(0)
