extends Node2D

@onready var dialogue_ui = $CanvasLayer/DialogueUI
@onready var interaction_manager = $InteractionManager

func _ready():
	interaction_manager.clicked_at.connect(_on_clicked_at)
	
	if not AdventureGameState.get_flag("intro_done"):
		dialogue_ui.show_message("Boss", tr("office_boss_intro"))
		AdventureGameState.set_flag("intro_done", true)
	else:
		dialogue_ui.show_message("Boss", tr("office_boss_back"))

func _on_clicked_at(pos: Vector2):
	# Door Area
	var door_rect = Rect2(900, 200, 100, 300)
	# Boss Area
	var boss_rect = Rect2(100, 200, 150, 300)
	
	if door_rect.has_point(pos):
		if AdventureGameState.get_flag("case_solved"):
			dialogue_ui.show_message("System", tr("office_system_case_closed"))
		else:
			dialogue_ui.show_message("System", tr("office_system_going_to_warehouse"))
			await get_tree().create_timer(1.0).timeout
			AdventureGameState.change_scene("res://samples/mystery/warehouse_investigation.tscn")
			
	elif boss_rect.has_point(pos):
		if AdventureGameState.get_flag("all_evidence_collected") and not AdventureGameState.get_flag("deduction_complete"):
			dialogue_ui.show_message("Boss", tr("deduction_title"))
			await get_tree().create_timer(0.8).timeout
			AdventureGameState.change_scene("res://samples/mystery/office_deduction.tscn")
		elif AdventureGameState.get_flag("case_solved"):
			dialogue_ui.show_message("Boss", tr("office_boss_great_work"))
		else:
			dialogue_ui.show_message("Boss", tr("office_boss_go_warehouse"))
