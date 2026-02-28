extends SceneTree

func _init():
	print("[VERIFY] Starting Boss Portrait Visibility Test...")
	
	# Load the office scene
	var scene = load("res://samples/mystery/office_base.tscn").instantiate()
	root.add_child(scene)
	
	# Load Dialogue UI
	var dialogue_ui = Control.new()
	dialogue_ui.set_script(load("res://samples/mystery/scripts/dialogue_ui_advanced.gd"))
	dialogue_ui.name = "DialogueUI"
	root.add_child(dialogue_ui)
	
	# Wait for a frame to let Godot call _ready() on added nodes
	await process_frame
	await process_frame
	
	var hs_boss = scene.get_node_or_null("hs_boss")
	if not hs_boss:
		print("[VERIFY] ERROR: hs_boss not found!")
		quit(1)
		return
		
	# Assume initially visible for the test
	hs_boss.visible = true
	print("[VERIFY] Initial hs_boss.visible = ", hs_boss.visible)
	
	# Emulate portrait shown
	print("[VERIFY] Emitting portrait_shown('boss')...")
	dialogue_ui.portrait_shown.emit("boss")
	await process_frame
	
	if hs_boss.visible:
		print("[VERIFY] ERROR: hs_boss is still visible after portrait_shown('boss')!")
		quit(1)
		return
	print("[VERIFY] Success: hs_boss is hidden.")
	
	# Emulate portrait hidden
	print("[VERIFY] Emitting portrait_hidden()...")
	dialogue_ui.portrait_hidden.emit()
	await process_frame
	
	if not hs_boss.visible:
		print("[VERIFY] ERROR: hs_boss is still hidden after portrait_hidden()!")
		quit(1)
		return
	print("[VERIFY] Success: hs_boss is visible again.")
	
	# Test a different character
	hs_boss.visible = true
	print("[VERIFY] Emitting portrait_shown('detective')...")
	dialogue_ui.portrait_shown.emit("detective")
	await process_frame
	
	if not hs_boss.visible:
		print("[VERIFY] ERROR: hs_boss got hidden for 'detective' portrait!")
		quit(1)
		return
	print("[VERIFY] Success: hs_boss remains visible for other characters.")
	
	print("[VERIFY] ALL BOSS PORTRAIT TESTS PASSED.")
	quit(0)
