extends SceneTree

func _init():
    print("Creating Global Main Menu...")
    
    var root = Control.new()
    root.name = "GlobalMainMenu"
    root.set_anchors_preset(Control.PRESET_FULL_RECT)
    
    # Background
    var color_rect = ColorRect.new()
    color_rect.color = Color(0.1, 0.1, 0.15)
    color_rect.set_anchors_preset(Control.PRESET_FULL_RECT)
    root.add_child(color_rect)
    color_rect.set_owner(root)

    var center_container = CenterContainer.new()
    center_container.name = "CenterContainer"
    center_container.set_anchors_preset(Control.PRESET_FULL_RECT)
    root.add_child(center_container)
    center_container.set_owner(root)

    var vbox = VBoxContainer.new()
    vbox.name = "VBoxContainer"
    center_container.add_child(vbox)
    vbox.set_owner(root)

    var label = Label.new()
    label.name = "Title"
    label.text = "Universal Game Template Demos"
    label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    label.modulate = Color(1, 1, 0)
    vbox.add_child(label)
    label.set_owner(root)
    
    # Buttons
    var demos = [
        {"name": "Rhythm Game", "path": "res://samples/rhythm/rhythm_demo.tscn"},
        {"name": "Fighting Game", "path": "res://samples/fighting/fighting_demo.tscn"},
        {"name": "Sandbox (Mining/Building)", "path": "res://samples/sandbox/sandbox_demo.tscn"},
        {"name": "Mystery Adventure", "path": "res://samples/mystery/demo_adv.tscn"},
        {"name": "Pixel Art Gallery", "path": "res://samples/gallery/gallery_demo.tscn"}
    ]
    
    for demo in demos:
        var btn = Button.new()
        btn.name = "Btn" + demo.name.replace(" ", "").replace("/", "")
        btn.text = demo.name
        btn.set_meta("scene_path", demo.path)
        btn.custom_minimum_size = Vector2(300, 50)
        vbox.add_child(btn)
        btn.set_owner(root)

    var quit_btn = Button.new()
    quit_btn.name = "BtnQuit"
    quit_btn.text = "Quit Game"
    quit_btn.custom_minimum_size = Vector2(300, 50)
    vbox.add_child(quit_btn)
    quit_btn.set_owner(root)

    # --- Script ---
    var script = GDScript.new()
    script.source_code = """
extends Control

func _ready():
    var vbox = $CenterContainer/VBoxContainer
    for child in vbox.get_children():
        if child is Button:
            if child.name == "BtnQuit":
                child.pressed.connect(_on_quit_pressed)
            elif child.has_meta("scene_path"):
                child.pressed.connect(_on_demo_pressed.bind(child.get_meta("scene_path")))

func _on_demo_pressed(path):
    if FileAccess.file_exists(path):
        get_tree().change_scene_to_file(path)
    else:
        print("Demo scene not found: " + path)

func _on_quit_pressed():
    get_tree().quit()
"""
    if script.reload() != OK:
        print("Error reloading script")
        quit(1)
        return
        
    root.set_script(script)

    var scene = PackedScene.new()
    scene.pack(root)
    ResourceSaver.save(scene, "res://samples/main_menu.tscn")
    
    print("Scene saved to res://samples/main_menu.tscn")
    quit(0)
