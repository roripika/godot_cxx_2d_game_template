extends Control

func _ready() -> void:
	# Load the othello scene
	var scene = load("res://samples/othello/othello_main.tscn").instantiate()
	add_child(scene)
	
	# Wait for rendering
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame
	
	var image = get_viewport().get_texture().get_image()
	image.save_png("res://othello_debug_capture.png")
	print("Screenshot saved to res://othello_debug_capture.png")
	get_tree().quit()
