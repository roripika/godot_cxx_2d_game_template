extends SceneTree

func _init():
	print("[VERIFY] Starting Inventory YAML and Sort Test...")
	
	var inventory_ui = load("res://samples/mystery/ui/evidence_inventory_ui.tscn").instantiate()
	root.add_child(inventory_ui)
	
	# Wait for ready
	await process_frame
	await process_frame
	
	# Add evidence in unsorted order (30, 10, 40, 20)
	inventory_ui.add_evidence("torn_memo")
	inventory_ui.add_evidence("footprint")
	inventory_ui.add_evidence("delivery_log")
	inventory_ui.add_evidence("ectoplasm")
	
	# After adding, UI refreshes and should sort. Check the array order directly.
	# The expected order is footprint (10), ectoplasm (20), torn_memo (30), delivery_log (40)
	var expected_order = ["footprint", "ectoplasm", "torn_memo", "delivery_log"]
	var list = inventory_ui.evidence_list
	
	if list.size() != 4:
		print("[VERIFY] ERROR: Failed to load 4 items, got ", list.size())
		quit(1)
		return
		
	for i in range(4):
		if list[i].id != expected_order[i]:
			print("[VERIFY] ERROR: Item at index ", i, " is ", list[i].id, " but expected ", expected_order[i])
			quit(1)
			return
			
	print("[VERIFY] Success: Items correctly loaded from YAML and perfectly sorted by sort_order.")
	print("[VERIFY] ALL INVENTORY TESTS PASSED.")
	quit(0)
