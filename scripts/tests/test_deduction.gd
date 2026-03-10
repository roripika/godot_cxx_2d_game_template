extends SceneTree

func _init():
	print("[TEST] Starting precise scenario test...")
	var shell = load("res://samples/mystery/karakuri_mystery_shell.tscn").instantiate()
	root.add_child(shell)
	await process_frame
	
	var runner = shell.get_node("ScenarioRunner")
	if runner:
		# Jump straight to deduction scene
		runner.call("load_scene_by_id", "deduction")
		print("[TEST] Jumped to deduction scene.")
	else:
		print("[TEST] No ScenarioRunner found!")
	
	# Wait a bit for the scene transition and text to play out
	var t = 0.0
	while t < 5.0:
		await process_frame
		t += root.get_process_delta_time()
		
		var dui = shell.find_child("DialogueUI", true, false)
		if dui and dui.is_typing:
			dui.call("skip_typing")
			
		# Automatically click to advance dialogue if waiting
		if dui and dui._waiting_for_click and not dui.is_typing:
			dui._waiting_for_click = false
			dui.dialogue_finished.emit()
			
		# Check if choices are shown
		var cc = dui._get_choices_container()
		if cc and cc.get_child_count() > 0:
			print("[TEST] Choices appeared at t = ", t, " seconds!")
			print("[TEST] Choice 0: ", cc.get_child(0).text)
			break

	var dui_final = shell.find_child("DialogueUI", true, false)
	var cc_final = dui_final._get_choices_container()
	print("[TEST] Final ChoicesContainer child count: ", cc_final.get_child_count())
	print("[TEST] Final DialogueUI visible: ", dui_final.visible)
	print("[TEST] Finished.")
	quit(0)
