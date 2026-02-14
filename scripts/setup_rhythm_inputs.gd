@tool
extends SceneTree

func _init():
    print("Setting up Rhythm Input Map...")
    
    var inputs = {
        "rhythm_pon": [KEY_F, JOY_BUTTON_B],   # Circle/A
        "rhythm_pata": [KEY_D, JOY_BUTTON_X],  # Square/X
        "rhythm_chaka": [KEY_J, JOY_BUTTON_Y], # Triangle/Y
        "rhythm_don": [KEY_K, JOY_BUTTON_A]    # Cross/B
    }
    
    for action in inputs:
        if not InputMap.has_action(action):
            InputMap.add_action(action)
            print("Added action: " + action)
            
        var key = InputEventKey.new()
        key.keycode = inputs[action][0]
        InputMap.action_add_event(action, key)
        
        var joy = InputEventJoypadButton.new()
        joy.button_index = inputs[action][1]
        InputMap.action_add_event(action, joy)
        
    ProjectSettings.save()
    print("Rhythm Input Map saved to project.godot")
    quit()
