extends SceneTree

# Final UI Audit Script
# Run this: /Applications/Godot.app/Contents/MacOS/Godot --path . -s samples/mystery/scripts/ui_audit.gd --headless

func _init():
	call_deferred("_run_audit")

func _run_audit():
	print("\n--- [STARTING UI AUDIT] ---")
	
	# Load the shell scene
	var shell_scene = load("res://samples/mystery/karakuri_mystery_shell.tscn")
	if not shell_scene:
		print("[ERROR] Could not load karakuri_mystery_shell.tscn")
		quit(1)
		return
	
	var shell = shell_scene.instantiate()
	get_root().add_child(shell)
	
	# Wait a frame for layout
	await process_frame
	
	var window_size = Vector2(
		ProjectSettings.get_setting("display/window/size/viewport_width"),
		ProjectSettings.get_setting("display/window/size/viewport_height")
	)
	if window_size == Vector2.ZERO: window_size = Vector2(1152, 648) # Fallback
	print("Viewport Size: ", window_size)

	_audit_node(shell, window_size)
	
	# Also check Warehouse scene if instantiated
	var scene_container = shell.get_node_or_null("SceneContainer")
	if scene_container:
		# Manual load for audit
		var warehouse_scene = load("res://samples/mystery/warehouse_base.tscn")
		if warehouse_scene:
			var warehouse = warehouse_scene.instantiate()
			scene_container.add_child(warehouse)
			await process_frame
			print("\nAuditing Warehouse Scene inside Shell:")
			_audit_node(warehouse, window_size)
	
	print("\n--- [UI AUDIT COMPLETED] ---")
	quit()

func _audit_node(node: Node, screen_size: Vector2):
	if node is Control:
		# We check global rect. If parent is NOT a control, anchors might be broken.
		var parent = node.get_parent()
		if parent and not (parent is Control or parent is CanvasLayer):
			print("[ALERT] Node '%s' (type %s) is a Control under a non-UI parent (%s). Anchors will not work correctly." % [node.name, node.get_class(), parent.get_class()])
		
		if node.visible:
			var rect = node.get_global_rect()
			var off_left = rect.position.x < 0
			var off_top = rect.position.y < 0
			var off_right = rect.end.x > screen_size.x
			var off_bottom = rect.end.y > screen_size.y
			
			if off_left or off_top or off_right or off_bottom:
				var warnings = []
				if off_left: warnings.append("Left:%f" % rect.position.x)
				if off_right: warnings.append("Right:%f" % rect.end.x)
				if off_top: warnings.append("Top:%f" % rect.position.y)
				if off_bottom: warnings.append("Bottom:%f" % rect.end.y)
				print("[OFF-SCREEN] %s: %s (Screen: %s)" % [node.get_path(), ", ".join(warnings), screen_size])
			else:
				# print("[OK] %s: %s" % [node.name, rect])
				pass

	for child in node.get_children():
		_audit_node(child, screen_size)
