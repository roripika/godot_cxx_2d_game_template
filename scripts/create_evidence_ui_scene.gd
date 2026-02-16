@tool
extends SceneTree

func _init():
    print("Creating Evidence Inventory UI...")
    
    var script_path = "res://scripts/evidence_inventory_ui.gd"
    if not FileAccess.file_exists(script_path):
        print("Script not found: " + script_path)
        quit(1)
        return

    var root = Control.new()
    root.name = "EvidenceInventoryUI"
    root.set_script(load(script_path))
    root.set_anchors_preset(Control.PRESET_FULL_RECT)
    
    # Overlay background
    var background = ColorRect.new()
    background.name = "Overlay"
    background.color = Color(0, 0, 0, 0.7)
    background.set_anchors_preset(Control.PRESET_FULL_RECT)
    root.add_child(background)
    background.set_owner(root)

    var panel = Panel.new()
    panel.name = "Panel"
    panel.custom_minimum_size = Vector2(800, 500)
    panel.set_anchors_preset(Control.PRESET_CENTER)
    # Manual centering offset (roughly) if preset doesn't fully work with size
    # But PRESET_CENTER should work with custom_minimum_size and grow direction.
    panel.set_anchors_preset(Control.PRESET_CENTER)
    panel.set_h_grow_direction(Control.GROW_DIRECTION_BOTH)
    panel.set_v_grow_direction(Control.GROW_DIRECTION_BOTH)
    root.add_child(panel)
    panel.set_owner(root)

    # Title
    var title = Label.new()
    title.name = "TitleLabel"
    title.text = "Evidence List"
    title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    title.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
    title.position = Vector2(0, 10)
    title.size = Vector2(800, 40)
    panel.add_child(title)
    title.set_owner(root)

    # Grid for items (Left side)
    var grid = GridContainer.new()
    grid.name = "GridContainer"
    grid.columns = 4
    grid.position = Vector2(20, 60)
    grid.size = Vector2(380, 400)
    panel.add_child(grid)
    grid.set_owner(root)

    # Details area (Right side)
    var large_icon = TextureRect.new()
    large_icon.name = "LargeIcon"
    large_icon.position = Vector2(420, 60)
    large_icon.size = Vector2(128, 128)
    large_icon.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
    large_icon.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
    panel.add_child(large_icon)
    large_icon.set_owner(root)

    var name_label = Label.new()
    name_label.name = "NameLabel"
    name_label.text = ""
    name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    name_label.position = Vector2(420, 200)
    name_label.size = Vector2(360, 30)
    panel.add_child(name_label)
    name_label.set_owner(root)

    var desc_label = Label.new()
    desc_label.name = "DescriptionLabel"
    desc_label.text = "Select an item to see details."
    desc_label.autowrap_mode = TextServer.AUTOWRAP_WORD
    desc_label.position = Vector2(420, 240)
    desc_label.size = Vector2(360, 150)
    panel.add_child(desc_label)
    desc_label.set_owner(root)

    var present_btn = Button.new()
    present_btn.name = "PresentButton"
    present_btn.text = "Present!"
    present_btn.position = Vector2(500, 420)
    present_btn.size = Vector2(200, 50)
    present_btn.disabled = true
    panel.add_child(present_btn)
    present_btn.set_owner(root)

    # Save
    var scene = PackedScene.new()
    scene.pack(root)
    var dir = DirAccess.open("res://samples/mystery")
    if not dir.dir_exists("ui"):
        dir.make_dir("ui")
        
    var path = "res://samples/mystery/ui/evidence_inventory_ui.tscn"
    ResourceSaver.save(scene, path)
    print("Scene saved to: " + path)
    quit()
