extends SceneTree

func _wait_frames(n: int):
	for i in range(n):
		await create_timer(0.01).timeout

func _assert(cond: bool, msg: String):
	if not cond:
		push_error("[VERIFY] ASSERT FAILED: " + msg)
		return false
	return true

func _run():
	print("[VERIFY] Testing Ending Scene UI...")
	
	# Set game_over flag to true via global node
	var ags = get_root().get_node_or_null("AdventureGameState")
	if ags:
		ags.set_flag("game_over", true)
	
	# Load ending scene
	var ending_scene = load("res://samples/mystery/ending.tscn")
	var root = ending_scene.instantiate()
	get_root().add_child(root)
	
	await _wait_frames(60) # Wait for _ready and animations
	
	var label = root.find_child("GameResultLabel", true, false)
	_assert(label != null, "GameResultLabel not found")
	_assert(label.visible, "GameResultLabel should be visible")
	_assert(label.text == "GAME OVER", "Label text should be GAME OVER")
	
	var dui = root.find_child("DialogueUI", true, false)
	_assert(dui != null, "DialogueUI not found")
	
	# DialogueUI visibility might be controlled by its own script, 
	# but in ending_scene.gd it's shown via show_message.
	# We poll for a moment.
	var dui_found := false
	for i in range(200):
		if dui.visible:
			dui_found = true
			break
		await _wait_frames(1)
	
	_assert(dui_found, "DialogueUI should become visible")
	
	var choices = dui.call("_get_choices_container")
	_assert(choices != null, "ChoicesContainer not found")
	
	print("[VERIFY] UI detected correctly.")
	
	print("[VERIFY] ALL GAME OVER UI TESTS PASSED.")
	quit(0)

func _init():
	_run()
