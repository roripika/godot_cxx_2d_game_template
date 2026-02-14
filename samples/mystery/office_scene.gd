extends Node2D

@onready var dialogue_ui = $CanvasLayer/DialogueUI
@onready var interaction_manager = $InteractionManager

func _ready():
	interaction_manager.clicked_at.connect(_on_clicked_at)
	
	if not AdventureGameState.get_flag("intro_done"):
		dialogue_ui.show_message("Boss", "Detectives! We have a new case.\nA Level 3 Apparition at the old warehouse.\nGo investigate properly.")
		AdventureGameState.set_flag("intro_done", true)
	else:
		dialogue_ui.show_message("Boss", "Back already? Did you solve it?")

func _on_clicked_at(pos: Vector2):
	# Door Area
	var door_rect = Rect2(900, 200, 100, 300)
	# Boss Area
	var boss_rect = Rect2(100, 200, 150, 300)
	
	if door_rect.has_point(pos):
		if AdventureGameState.get_flag("case_solved"):
			dialogue_ui.show_message("System", "Case Closed. Thanks for playing!")
		else:
			dialogue_ui.show_message("System", "Going to the Warehouse...")
			await get_tree().create_timer(1.0).timeout
			AdventureGameState.change_scene("res://samples/mystery/warehouse_investigation.tscn")
			
	elif boss_rect.has_point(pos):
		if AdventureGameState.get_flag("has_evidence"):
			dialogue_ui.show_message("Boss", "Great work! That evidence proves it was a poltergeist.\nYou solved the case!")
			AdventureGameState.set_flag("case_solved", true)
		else:
			dialogue_ui.show_message("Boss", "What are you doing here? Go to the warehouse!")
