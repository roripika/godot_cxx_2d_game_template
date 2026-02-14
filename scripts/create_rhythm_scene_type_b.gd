extends SceneTree

func _init():
    print("Creating Rhythm Demo Scene (Type B)...")
    
    var root = Node2D.new()
    root.name = "RhythmDemoTypeB"
    
    # --- Conductor ---
    if not ClassDB.class_exists("Conductor"):
        print("Error: Conductor class missing.")
        quit(1)
        return
    var conductor = ClassDB.instantiate("Conductor")
    conductor.name = "Conductor"
    root.add_child(conductor)
    conductor.set_owner(root)
    
    # --- RhythmCommandListener ---
    if not ClassDB.class_exists("RhythmCommandListener"):
        print("Error: RhythmCommandListener class missing.")
        quit(1)
        return
    var listener = ClassDB.instantiate("RhythmCommandListener")
    listener.name = "RhythmCommandListener"
    listener.set_conductor(conductor)
    root.add_child(listener)
    listener.set_owner(root)
    
    # --- UI ---
    var canvas_layer = CanvasLayer.new()
    canvas_layer.name = "UI"
    root.add_child(canvas_layer)
    canvas_layer.set_owner(root)
    
    var feedback_label = Label.new()
    feedback_label.name = "FeedbackLabel"
    feedback_label.text = "Press SPACE (Pon) or ESC (Pata) on beat!"
    feedback_label.position = Vector2(400, 200)
    canvas_layer.add_child(feedback_label)
    feedback_label.set_owner(root)
    
    var command_log = Label.new()
    command_log.name = "CommandLog"
    command_log.text = ""
    command_log.position = Vector2(400, 250)
    canvas_layer.add_child(command_log)
    command_log.set_owner(root)
    
    # --- Logic Script ---
    var script = GDScript.new()
    script.source_code = """
extends Node2D

@onready var conductor = $Conductor
@onready var listener = $RhythmCommandListener
@onready var feedback_label = $UI/FeedbackLabel
@onready var command_log = $UI/CommandLog

func _ready():
    print("Rhythm Demo Type B Started")
    
    listener.rhythm_input.connect(_on_rhythm_input)
    listener.command_executed.connect(_on_command_executed)
    listener.rhythm_failed.connect(_on_rhythm_failed)
    
    # Start music loop
    conductor.play(null, 120.0, 0.0, 4)

func _on_rhythm_input(type, judgment):
    feedback_label.text = type.to_upper() + " (" + judgment + ")"
    if judgment == "miss":
        feedback_label.modulate = Color(1, 0, 0)
    else:
        feedback_label.modulate = Color(0, 1, 0)

func _on_command_executed(command):
    command_log.text = "Executed: " + str(command)
    feedback_label.text = "FEVER!!"
    feedback_label.modulate = Color(1, 1, 0)
    
func _on_rhythm_failed():
    feedback_label.text = "FAIL"
    feedback_label.modulate = Color(0.5, 0.5, 0.5)
"""
    if script.reload() != OK:
        print("Error reloading script")
        quit(1)
        return
        
    root.set_script(script)

    # Save Scene
    var scene = PackedScene.new()
    scene.pack(root)
    ResourceSaver.save(scene, "res://samples/rhythm/rhythm_demo_type_b.tscn")
    
    print("Scene saved to res://samples/rhythm/rhythm_demo_type_b.tscn")
    quit(0)
