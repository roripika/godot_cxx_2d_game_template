extends Node

func _ready():
    print("Billiards Phase 2 Test: Starting...")
    
    var manager = BilliardsManager.new()
    add_child(manager)
    
    # Connect signal
    manager.ball_position_updated.connect(_on_ball_position_updated)
    
    print("BilliardsTest: Manager created. Starting simulation...")
    manager.start_simulation()
    
    # Strike the ball after 1 second
    await get_tree().create_timer(1.0).timeout
    print("BilliardsTest: Striking cue ball!")
    manager.strike_cue_ball(Vector3(10, 0, 0), 100.0)
    
    # Wait for more simulation
    await get_tree().create_timer(2.0).timeout
    print("BilliardsTest: Finished Phase 2 test.")

func _on_ball_position_updated(new_pos):
    # Print position occasionally to avoid spam
    if Engine.get_physics_frames() % 60 == 0:
        print("BilliardsTest: Ball Position: ", new_pos)
