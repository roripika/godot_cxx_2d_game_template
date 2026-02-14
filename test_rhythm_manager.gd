extends SceneTree

func _init():
    print("Testing RhythmGameManager...")
    
    if not ClassDB.class_exists("RhythmGameManager"):
        print("ERROR: RhythmGameManager class not found.")
        quit(1)
        return
        
    var manager = ClassDB.instantiate("RhythmGameManager")
    
    # Test Init
    if manager.get_score() != 0 or manager.get_combo() != 0:
        print("ERROR: Initial state incorrect.")
        quit(1)
        return
        
    # Test Perfect
    print("Testing Perfect Hit (0.0s)...")
    manager.register_hit(0.0)
    if manager.get_combo() != 1:
        print("ERROR: Combo should be 1, got ", manager.get_combo())
        quit(1)
        return
        
    # Test Good (0.08s, default good window is 0.1s)
    print("Testing Good Hit (0.08s)...")
    manager.register_hit(0.08)
    if manager.get_combo() != 2:
        print("ERROR: Combo should be 2, got ", manager.get_combo())
        quit(1)
        return
        
    # Test Miss (0.2s, outside 0.1s window)
    print("Testing Miss (0.2s)...")
    manager.register_hit(0.2)
    if manager.get_combo() != 0:
        print("ERROR: Combo should be reset to 0, got ", manager.get_combo())
        quit(1)
        return
        
    print("RhythmGameManager Test: SUCCESS")
    manager.free()
    quit(0)
