extends SceneTree

func _init() -> void:
	var root = get_root()
	var scene = load("res://samples/mystery/warehouse_base.tscn").instantiate()
	root.add_child(scene)
	call_deferred("_capture", root)

func _capture(root) -> void:
	await root.get_tree().create_timer(1.0).timeout
	var img = root.get_viewport().get_texture().get_image()
	img.save_png("res://warehouse_check.png")
	quit()
