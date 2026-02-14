extends Node2D

@onready var dialogue_ui = $CanvasLayer/DialogueUI
@onready var interaction_manager = $InteractionManager
@onready var ghost_sprite = $GhostArea/Sprite2D

func _ready():
	if not dialogue_ui or not interaction_manager:
		return
		
	interaction_manager.clicked_at.connect(_on_clicked_at)
	
	dialogue_ui.show_message("System", "You arrived at the Haunted Spot.\nFind the source of the paranormal activity.")

func _on_clicked_at(pos: Vector2):
	var ghost_pos = $GhostArea.position
	var ghost_rect = Rect2(ghost_pos - Vector2(32, 32), Vector2(64, 64))
	
	# Exit Area (Left side of screen)
	var exit_rect = Rect2(0, 0, 100, 648)
	
	if ghost_rect.has_point(pos):
		if AdventureGameState.get_flag("has_evidence"):
			dialogue_ui.show_message("Detective", "I already have the evidence. I should report to the Boss.")
		else:
			dialogue_ui.show_message("Detective", "Aha! This ectoplasm proves it.\n(Evidence Collected)")
			AdventureGameState.set_flag("has_evidence", true)
			AdventureGameState.add_item("Ectoplasm Sample")
			
	elif exit_rect.has_point(pos):
		dialogue_ui.show_message("System", "Returning to Office...")
		await get_tree().create_timer(1.0).timeout
		AdventureGameState.change_scene("res://samples/mystery/office_scene.tscn")
		
	else:
		dialogue_ui.hide_dialogue()

func _process(delta):
	if ghost_sprite:
		ghost_sprite.position.y = sin(Time.get_ticks_msec() / 200.0) * 10.0
