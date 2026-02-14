extends Node

func _ready():
    # Only add if not already present
    if has_node("CommonCanvas"):
        return

    var canvas = CanvasLayer.new()
    canvas.name = "CommonCanvas"
    add_child(canvas)
    
    var btn = Button.new()
    btn.name = "BtnBackToMenu"
    btn.text = "Back to Menu (ESC)"
    btn.position = Vector2(10, 10)
    btn.size = Vector2(150, 30)
    btn.pressed.connect(_on_back_pressed)
    canvas.add_child(btn)

func _unhandled_input(event):
    if event.is_action_pressed("ui_cancel"):
        _on_back_pressed()

func _on_back_pressed():
    # If the scene is Main Menu, do nothing
    if get_tree().current_scene.scene_file_path == "res://samples/main_menu.tscn":
        return
        
    print("Returning to Main Menu...")
    get_tree().change_scene_to_file("res://samples/main_menu.tscn")
