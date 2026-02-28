extends SceneTree

func _init():
	var file = FileAccess.open("res://samples/mystery/scenario/mystery.yaml", FileAccess.READ)
	var content = file.get_as_text()
	var lines = content.split("\n")
	
	# We simulates the C++ parsing or just manually print to see the actual contents of the array
	print("Checking mystery yaml for choices in deduction...")
	quit()
