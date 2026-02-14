extends SceneTree

func _init():
    print("Creating Rhythm Demo Scene (Type A)...")
    
    var root = Node2D.new()
    root.name = "RhythmDemo"
    
    # --- Conductor ---
    if not ClassDB.class_exists("Conductor"):
        print("Error: Conductor class missing.")
        quit(1)
        return
    var conductor = ClassDB.instantiate("Conductor")
    conductor.name = "Conductor"
    root.add_child(conductor)
    conductor.set_owner(root)
    
    # --- RhythmGameManager ---
    if not ClassDB.class_exists("RhythmGameManager"):
        print("Error: RhythmGameManager class missing.")
        quit(1)
        return
    var manager = ClassDB.instantiate("RhythmGameManager")
    manager.name = "RhythmGameManager"
    root.add_child(manager)
    manager.set_owner(root)
    
    # --- Note Lanes ---
    var lane_container = Node2D.new()
    lane_container.name = "Lanes"
    root.add_child(lane_container)
    lane_container.set_owner(root)
    
    # 4 Lanes
    var actions = ["ui_left", "ui_down", "ui_up", "ui_right"]
    var x_positions = [100, 200, 300, 400]
    
    for i in range(4):
        if not ClassDB.class_exists("NoteLane"):
            print("Error: NoteLane class missing.")
            quit(1)
            return
        
        var lane = ClassDB.instantiate("NoteLane")
        lane.name = "Lane_" + str(i+1)
        lane.position = Vector2(x_positions[i], 0)
        lane.set_input_action(actions[i])
        lane.set_speed(300.0) # faster
        lane.set_target_y(500.0)
        
        lane_container.add_child(lane)
        lane.set_owner(root)
        
        # Add visual guideline (ColorRect)
        var line = ColorRect.new()
        line.name = "Line"
        line.color = Color(1, 1, 1, 0.2)
        line.size = Vector2(50, 600)
        line.position = Vector2(-25, 0)
        lane.add_child(line)
        line.set_owner(root)
        
        # Add target line (ColorRect)
        var target = ColorRect.new()
        target.name = "Target"
        target.color = Color(1, 0, 0, 0.5)
        target.size = Vector2(60, 10)
        target.position = Vector2(-30, 495)
        lane.add_child(target)
        target.set_owner(root)

    # --- UI ---
    var canvas_layer = CanvasLayer.new()
    canvas_layer.name = "UI"
    root.add_child(canvas_layer)
    canvas_layer.set_owner(root)
    
    var score_label = Label.new()
    score_label.name = "ScoreLabel"
    score_label.text = "Score: 0"
    score_label.position = Vector2(500, 50)
    canvas_layer.add_child(score_label)
    score_label.set_owner(root)
    
    var combo_label = Label.new()
    combo_label.name = "ComboLabel"
    combo_label.text = "Combo: 0"
    combo_label.position = Vector2(500, 80)
    canvas_layer.add_child(combo_label)
    combo_label.set_owner(root)
    
    var judgment_label = Label.new()
    judgment_label.name = "JudgmentLabel"
    judgment_label.text = ""
    judgment_label.position = Vector2(500, 110)
    canvas_layer.add_child(judgment_label)
    judgment_label.set_owner(root)
    
    # --- Logic Script ---
    # Create GDScript dynamically and attach
    var script = GDScript.new()
    script.source_code = """
extends Node2D

@onready var conductor = $Conductor
@onready var manager = $RhythmGameManager
@onready var score_label = $UI/ScoreLabel
@onready var combo_label = $UI/ComboLabel
@onready var judgment_label = $UI/JudgmentLabel
@onready var lanes = $Lanes

func _ready():
    print("Rhythm Demo Started")
    
    # Connect signals
    manager.score_changed.connect(_on_score_changed)
    manager.combo_changed.connect(_on_combo_changed)
    manager.judgment.connect(_on_judgment)
    
    # Load notes
    # Simple pattern: every beat on different lanes
    var lane1_notes: PackedFloat64Array = [1.0, 5.0, 9.0]
    var lane2_notes: PackedFloat64Array = [2.0, 6.0, 10.0]
    var lane3_notes: PackedFloat64Array = [3.0, 7.0, 11.0]
    var lane4_notes: PackedFloat64Array = [4.0, 8.0, 12.0]
    
    $Lanes/Lane_1.load_notes(lane1_notes)
    $Lanes/Lane_2.load_notes(lane2_notes)
    $Lanes/Lane_3.load_notes(lane3_notes)
    $Lanes/Lane_4.load_notes(lane4_notes)
    
    # Start music (silence/dummy if none)
    conductor.play(null, 120.0, 0.0, 4) # 4 beats lead-in
    
func _on_score_changed(score):
    score_label.text = "Score: " + str(score)

func _on_combo_changed(combo):
    combo_label.text = "Combo: " + str(combo)

func _on_judgment(type):
    judgment_label.text = type.to_upper()
    # Simple animation via Tween could be added here
"""
    if script.reload() != OK:
        print("Error reloading script")
        quit(1)
        return
        
    root.set_script(script)

    # Save Scene
    var scene = PackedScene.new()
    scene.pack(root)
    ResourceSaver.save(scene, "res://samples/rhythm/rhythm_demo_type_a.tscn")
    
    print("Scene saved to res://samples/rhythm/rhythm_demo_type_a.tscn")
    quit(0)
