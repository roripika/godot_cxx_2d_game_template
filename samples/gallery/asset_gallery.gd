extends Node2D

@onready var grid_container_flat = $CanvasLayer/ScrollContainer/VBoxContainer/FlatGrid
@onready var grid_container_pixel = $CanvasLayer/ScrollContainer/VBoxContainer/PixelGrid

func _ready():
	load_assets("res://assets/zodiac_v2/128x128/", grid_container_flat)
	load_assets("res://assets/zodiac_pixel/128x128/", grid_container_pixel)

func load_assets(path: String, container: Control):
	var dir = DirAccess.open(path)
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if !dir.current_is_dir() and file_name.ends_with(".png"):
				var texture = load(path + file_name)
				if texture:
					var text_rect = TextureRect.new()
					text_rect.texture = texture
					text_rect.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
					text_rect.custom_minimum_size = Vector2(128, 128)
					
					var label = Label.new()
					label.text = file_name.trim_suffix(".png").replace("zodiac_", "").replace("pixel_", "")
					label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
					
					var vbox = VBoxContainer.new()
					vbox.add_child(text_rect)
					vbox.add_child(label)
					
					container.add_child(vbox)
			file_name = dir.get_next()
	else:
		print("An error occurred when trying to access the path: " + path)

func _input(event):
	if event.is_action_pressed("ui_cancel"):
		get_tree().quit() # Or go back to main menu
