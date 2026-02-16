@tool
extends SceneTree

func _init():
    print("Creating Testimony UI...")
    
    var script_path = "res://scripts/testimony_system.gd"
    if not FileAccess.file_exists(script_path):
        print("Script not found: " + script_path)
        quit(1)
        return

    var root = Control.new()
    root.name = "TestimonySystem"
    root.set_script(load(script_path))
    root.set_anchors_preset(Control.PRESET_FULL_RECT)
    root.visible = false # Hidden initially
    
    # Overlay background
    var background = ColorRect.new()
    background.name = "Overlay"
    background.color = Color(0, 0, 0, 0.4)
    background.set_anchors_preset(Control.PRESET_FULL_RECT)
    # Visible when testimony active
    root.add_child(background)
    background.set_owner(root)

    # Health Bar (Top Right)
    var bar = ProgressBar.new()
    bar.name = "HealthBar"
    bar.min_value = 0
    bar.max_value = 100
    bar.value = 100
    bar.show_percentage = false
    bar.position = Vector2(800, 20)
    bar.size = Vector2(180, 20)
    root.add_child(bar)
    bar.set_owner(root)

    # Mode Label (Top Left)
    var mode = Label.new()
    mode.name = "LabelMode"
    mode.text = "Witness Testimony"
    # Could use custom style or large font
    mode.position = Vector2(20, 20)
    mode.size = Vector2(200, 40)
    root.add_child(mode)
    mode.set_owner(root)

    # Press/Present Buttons (Middle Bottom, above DialogueUI)
    var btn_press = Button.new()
    btn_press.name = "ButtonPress"
    btn_press.text = "Press"
    btn_press.position = Vector2(300, 350)
    btn_press.size = Vector2(100, 40)
    root.add_child(btn_press)
    btn_press.set_owner(root)

    var btn_present = Button.new()
    btn_present.name = "ButtonPresent"
    btn_present.text = "Present"
    btn_present.position = Vector2(600, 350)
    btn_present.size = Vector2(100, 40)
    root.add_child(btn_present)
    btn_present.set_owner(root)

    # Save
    var scene = PackedScene.new()
    scene.pack(root)
    var dir = DirAccess.open("res://samples/mystery")
    if not dir.dir_exists("ui"):
        dir.make_dir("ui")
        
    var path = "res://samples/mystery/ui/testimony_ui.tscn"
    ResourceSaver.save(scene, path)
    print("Testimony UI saved to: " + path)
    quit()
