# EndingScene.gd
# エンディング（4種類）
extends Node2D
class_name EndingScene

@onready var game_result_label = $CanvasLayer/GameResultLabel
@onready var dialogue_ui = $CanvasLayer/DialogueUI

func _ready():
	# 初期化
	if game_result_label:
		game_result_label.hide()
	
	var ags = get_tree().root.get_node_or_null("AdventureGameState")
	
	# フラグに基づいてエンディングを分岐
	if GameMaster.get_flag("game_over"):
		_set_result_title("GAME OVER", Color.CRIMSON)
		await _show_failure_ending()
	elif GameMaster.get_flag("perfect_ending"):
		_set_result_title("PERFECT!", Color.GOLD)
		await _show_perfect_ending()
	elif GameMaster.get_flag("case_solved"):
		_set_result_title("THE END", Color.SKY_BLUE)
		await _show_normal_ending()
	else:
		_set_result_title("GAME OVER", Color.CRIMSON)
		await _show_failure_ending()

func _set_result_title(text: String, color: Color):
	if game_result_label:
		game_result_label.text = text
		game_result_label.modulate = color
		game_result_label.show()

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

	var ags = get_tree().root.get_node_or_null("AdventureGameState")
	if choice_idx == 0:
		if ags: ags.call("reset_game")
		if ags: ags.call("change_scene", "res://samples/mystery/office_scene.tscn")
	else:
		if ags: ags.call("reset_game")
		if ags: ags.call("change_scene", "res://samples/main_menu.tscn")
