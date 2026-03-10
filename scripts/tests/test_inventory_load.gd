extends SceneTree

func _init():
	print("[VERIFY] Starting Inventory Load Test...")
	
	var scene = load("res://samples/mystery/ui/evidence_inventory_ui.tscn").instantiate()
	if not scene:
		print("[VERIFY] ERROR: Failed to instantiate inventory_ui.tscn!")
		quit(1)
		return
	
	root.add_child(scene)
	await process_frame
	
	print("[VERIFY] Loaded item cache keys: ", scene.evidence_resources.keys())
	if scene.evidence_resources.has("witness_report"):
		print("[VERIFY] Success: witness_report is properly loaded into inventory cache.")
		quit(0)
	else:
		print("[VERIFY] ERROR: witness_report is NOT in item cache!")
		quit(1)
