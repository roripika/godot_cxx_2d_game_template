extends Area2D

@export var hotspot_id: String

func _ready():
    # Make clickable
    input_pickable = true
    mouse_entered.connect(_on_mouse_entered)
    mouse_exited.connect(_on_mouse_exited)
    # Note: Logic is now handled by KarakuriScenarioRunner via YAML based on node name/hotspot_id

func _on_mouse_entered():
    Input.set_default_cursor_shape(Input.CURSOR_POINTING_HAND)

func _on_mouse_exited():
    Input.set_default_cursor_shape(Input.CURSOR_ARROW)

