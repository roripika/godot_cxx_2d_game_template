# OfficeDeductionScene.gd
# 推理パート（オフィス）
extends Node2D
class_name OfficeDeductionScene

@onready var dialogue_ui = $CanvasLayer/DialogueUI
@onready var inventory_ui = $CanvasLayer/InventoryUI

func _ready():
	# 所長に報告
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("deduction_title"))
	
	await get_tree().create_timer(1.0).timeout
	await _show_choices()

func _show_choices():
	"""推理の選択肢を表示"""
	if not dialogue_ui:
		return
	
	dialogue_ui.show_message("Detective", tr("deduction_question"))
	
	await get_tree().create_timer(1.0).timeout
	
	# 選択肢を表示
	var choices: Array[String] = [
		tr("deduction_choice1"),
		tr("deduction_choice2"),
		tr("deduction_choice3")
	]
	
	# もし show_choices が実装されていれば使用
	if dialogue_ui.has_method("show_choices"):
		var choice_index = await dialogue_ui.show_choices(choices)
		_on_choice_made(choice_index)
	else:
		# フォールバック：正しい推理を自動選択
		dialogue_ui.show_message("Detective", choices[1])
		_on_choice_made(1)

func _on_choice_made(choice_index: int):
	"""選択が完了した"""
	var is_correct = (choice_index == 0)  # index 0 が正解（証拠）
	
	if is_correct:
		AdventureGameState.set_flag("deduction_complete", true)
		if dialogue_ui:
			dialogue_ui.show_message("Boss", tr("deduction_correct"))
	else:
		AdventureGameState.take_damage()
		if dialogue_ui:
			dialogue_ui.show_message("Boss", tr("deduction_wrong"))
		if AdventureGameState.get_health() <= 0:
			AdventureGameState.set_flag("game_over", true)
			await get_tree().create_timer(1.0).timeout
			AdventureGameState.change_scene("res://samples/mystery/ending.tscn")
			return
	
	await get_tree().create_timer(1.0).timeout
	_go_to_confrontation(is_correct)

func _go_to_confrontation(is_correct: bool):
	"""対決シーンへ"""
	if dialogue_ui and not is_correct:
		dialogue_ui.show_message("System", tr("deduction_hp_lost"))
	
	await get_tree().create_timer(1.0).timeout
	AdventureGameState.change_scene("res://samples/mystery/warehouse_confrontation.tscn")
