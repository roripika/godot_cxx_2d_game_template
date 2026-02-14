# EndingScene.gd
# エンディング（4種類）
extends Node2D
class_name EndingScene

@onready var dialogue_ui = $CanvasLayer/DialogueUI

func _ready():
	# フラグに基づいてエンディングを分岐
	if AdventureGameState.get_flag("game_over"):
		await _show_failure_ending()
	elif AdventureGameState.get_flag("perfect_ending"):
		await _show_perfect_ending()
	elif AdventureGameState.get_flag("case_solved"):
		await _show_normal_ending()
	else:
		await _show_failure_ending()

func _show_perfect_ending():
	"""完全勝利エンディング"""
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("ending_perfect"))
	
	await get_tree().create_timer(1.5).timeout
	
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("ending_perfect") + "\n\n〜THE END〜")
	
	await get_tree().create_timer(3.0).timeout
	await _create_retry_menu()

func _show_normal_ending():
	"""通常クリアエンディング"""
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("ending_normal"))
	
	await get_tree().create_timer(1.5).timeout
	
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("ending_normal") + "\n\n〜THE END〜")
	
	await get_tree().create_timer(3.0).timeout
	await _create_retry_menu()

func _show_failure_ending():
	"""失敗エンディング"""
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("ending_failure"))
	
	await get_tree().create_timer(2.0).timeout
	await _create_retry_menu()

func _create_retry_menu():
	"""リトライ/メニュー遷移"""
	if not dialogue_ui:
		return
	
	var choices: Array[String] = [
		tr("ending_choice_retry"),
		tr("ending_choice_menu")
	]
	
	dialogue_ui.show_message("System", tr("ending_title"))
	var choice_idx := 1
	if dialogue_ui.has_method("show_choices"):
		choice_idx = await dialogue_ui.show_choices(choices)

	if choice_idx == 0:
		AdventureGameState.reset_game()
		AdventureGameState.change_scene("res://samples/mystery/office_scene.tscn")
	else:
		AdventureGameState.reset_game()
		AdventureGameState.change_scene("res://samples/main_menu.tscn")
