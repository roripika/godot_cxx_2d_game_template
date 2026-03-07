extends Node2D

@onready var scenario_runner = $ScenarioRunner
@onready var player = $MysteryPlayer
@onready var camera = $MysteryPlayer/Camera2D
@onready var interact_icon = $CanvasLayer/InteractIcon
@onready var interaction_component = $MysteryPlayer/InteractionComponent

func _ready() -> void:
	# Connect interaction component signals
	interaction_component.focus_changed.connect(_on_focus_changed)
	
	# Connect all mystery objects in the scene
	for child in get_children():
		if child is MysteryObject:
			child.interacted.connect(_on_object_interacted)
	
	# Initial scenario load if needed
	scenario_runner.load_scenario()

func _process(_delta: float) -> void:
	# Update "!" icon position to follow focused object
	if interact_icon.visible:
		var focused_node = interaction_component.get_focused_node()
		if focused_node:
			var screen_pos = get_viewport().get_camera_2d().get_screen_center_position()
			# Simple overhead positioning
			var pos = focused_node.global_position + Vector2(0, -80)
			interact_icon.global_position = pos - interact_icon.size / 2

func _on_focus_changed(node: Node, is_focused: bool) -> void:
	interact_icon.visible = is_focused
	if is_focused:
		interact_icon.text = "！ " + node.get_interaction_label()

func _on_object_interacted(scenario_id: String) -> void:
	print("[InvestigationStage] Object interacted! scenario_id: ", scenario_id)
	# Camera zoom effect
	var tween = create_tween()
	tween.tween_property(camera, "zoom", Vector2(1.2, 1.2), 0.3).set_trans(Tween.TRANS_SINE)
	
	# Execute scenario
	print("[InvestigationStage] Calling scenario_runner.load_scene_by_id: ", scenario_id)
	scenario_runner.load_scene_by_id(scenario_id)
	
	# Wait for scenario to finish (simplified)
	# In a real app, we might wait for a signal from ScenarioRunner
	await get_tree().create_timer(1.0).timeout
	
	# Reset camera
	var tween_back = create_tween()
	tween_back.tween_property(camera, "zoom", Vector2(1.0, 1.0), 0.5).set_trans(Tween.TRANS_SINE)
