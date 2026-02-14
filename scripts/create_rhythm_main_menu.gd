extends SceneTree

func _init():
    print("Creating Rhythm Game Demo Main Menu...")
    
    var root = Control.new()
    root.name = "RhythmGameDemo"
    root.set_anchors_preset(Control.PRESET_FULL_RECT)

    var vbox = VBoxContainer.new()
    vbox.name = "VBoxContainer"
    vbox.set_anchors_preset(Control.PRESET_CENTER)
    root.add_child(vbox)
    vbox.set_owner(root)

    var label = Label.new()
    label.name = "Title"
    label.text = "Rhythm Game Tech Demo"
    label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    vbox.add_child(label)
    label.set_owner(root)

    var btn_type_a = Button.new()
    btn_type_a.name = "BtnTypeA"
    btn_type_a.text = "Type A: Scrolling Notes"
    btn_type_a.custom_minimum_size = Vector2(200, 50)
    vbox.add_child(btn_type_a)
    btn_type_a.set_owner(root)

    var btn_type_b = Button.new()
    btn_type_b.name = "BtnTypeB"
    btn_type_b.text = "Type B: Command (Patapon)"
    btn_type_b.custom_minimum_size = Vector2(200, 50)
    vbox.add_child(btn_type_b)
    btn_type_b.set_owner(root)

    # --- Script ---
    var script = GDScript.new()
    script.source_code = """
extends Control

@onready var btn_type_a = $VBoxContainer/BtnTypeA
@onready var btn_type_b = $VBoxContainer/BtnTypeB

func _ready():
    btn_type_a.pressed.connect(_on_type_a_pressed)
    btn_type_b.pressed.connect(_on_type_b_pressed)
    print("Rhythm Demo Menu Loaded")

func _on_type_a_pressed():
    get_tree().change_scene_to_file("res://samples/rhythm/rhythm_demo_type_a.tscn")

func _on_type_b_pressed():
    get_tree().change_scene_to_file("res://samples/rhythm/rhythm_demo_type_b.tscn")
"""
    if script.reload() != OK:
        print("Error reloading script")
        quit(1)
        return
        
    root.set_script(script)

    var scene = PackedScene.new()
    scene.pack(root)
    ResourceSaver.save(scene, "res://samples/rhythm/rhythm_demo.tscn")
    
    print("Scene saved to res://samples/rhythm/rhythm_demo.tscn")
    quit(0)
