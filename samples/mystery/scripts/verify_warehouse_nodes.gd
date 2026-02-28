extends SceneTree

func _init():
	print("[VERIFY] Starting Warehouse Base Scene Load Test...")
	
	var scene = load("res://samples/mystery/warehouse_base.tscn").instantiate()
	if not scene:
		print("[VERIFY] ERROR: Failed to instantiate warehouse_base.tscn!")
		quit(1)
		return
	
	root.add_child(scene)
	await process_frame
	
	if scene.exit_label == null:
		print("[VERIFY] ERROR: exit_label is STILL NULL!")
		quit(1)
		return
		
	if scene.inv_border == null:
		print("[VERIFY] ERROR: inv_border is STILL NULL!")
		quit(1)
		return
		
	print("[VERIFY] Success: ExitVisual and InvestigationBorder successfully restored and linked.")
	quit(0)
