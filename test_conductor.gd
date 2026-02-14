extends SceneTree

func _init():
    print("Testing Conductor Registration...")
    
    if not ClassDB.class_exists("Conductor"):
        print("ERROR: Conductor class not found.")
        quit(1)
        return
        
    print("Conductor found. Instantiating...")
    var conductor = ClassDB.instantiate("Conductor")
    if not conductor:
        print("ERROR: Failed to instantiate Conductor.")
        quit(1)
        return
        
    print("Setting BPM to 120...")
    conductor.set_bpm(120.0)
    var current_bpm = conductor.get_bpm()
    print("Current BPM: ", current_bpm)
    
    if abs(current_bpm - 120.0) > 0.001:
        print("ERROR: BPM mismatch. Expected 120.0, got ", current_bpm)
        quit(1)
        return
        
    print("Conductor Test: SUCCESS")
    conductor.free()
    quit(0)
