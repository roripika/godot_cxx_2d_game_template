extends SceneTree

func _init():
	print("[TEST] Starting choice UI test...")
	var scene = load("res://samples/mystery/karakuri_mystery_shell.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	
	var dui = scene.find_child("DialogueUI", true, false)
	if not dui:
		print("[TEST] ERROR: DialogueUI not found")
		quit(1)
		return
		
	dui.visible = true
	var choices = [{"text": "Option A"}, {"text": "Option B"}]
	dui.call("show_choices_with_defs", choices)
	
	await process_frame
	await process_frame
	
	var cc = dui._get_choices_container()
	if not cc:
		print("[TEST] ERROR: ChoicesContainer not found")
		quit(1)
		return
		
	print("[TEST] ChoicesContainer child count: ", cc.get_child_count())
	for c in cc.get_children():
		print(" - ", c.name, " visible: ", c.visible, " text: ", c.text, " pos: ", c.position, " size: ", c.size, " global_pos: ", c.global_position)
		
	print("[TEST] DialogueUI global rect: ", dui.get_global_rect())
	print("[TEST] VBoxContainer global rect: ", dui.get_node("VBoxContainer").get_global_rect())
	print("[TEST] ChoicesContainer global rect: ", cc.get_global_rect())
	
	print("[TEST] Finished.")
	quit(0)
