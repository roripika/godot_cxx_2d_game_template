# EndingScene.gd
# エンディング（4種類）
extends Node2D
class_name EndingScene

@onready var dialogue_ui = $CanvasLayer/DialogueUI

func _ready():
	# フラグに基づいてエンディングを分岐
	if AdventureGameState.get_flag("perfect_ending"):
		_show_perfect_ending()
	elif AdventureGameState.get_flag("case_solved"):
		_show_normal_ending()
	else:
		_show_failure_ending()

func _show_perfect_ending():
	"""完全勝利エンディング"""
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("ending_perfect"))
	
	await get_tree().create_timer(1.5).timeout
	
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("ending_perfect") + "\n\n〜THE END〜")
	
	await get_tree().create_timer(3.0).timeout
	_create_retry_menu()

func _show_normal_ending():
	"""通常クリアエンディング"""
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("ending_normal"))
	
	await get_tree().create_timer(1.5).timeout
	
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("ending_normal") + "\n\n〜THE END〜")
	
	await get_tree().create_timer(3.0).timeout
	_create_retry_menu()

func _show_failure_ending():
	"""失敗エンディング"""
	if dialogue_ui:
		dialogue_ui.show_message("Boss", tr("ending_failure"))
	
	await get_tree().create_timer(2.0).timeout
	_create_retry_menu()

func _create_retry_menu():
	"""リトライメニューを作成"""
	if not dialogue_ui:
		return
	
	var choices = [
		"もう一度プレイ",
		"メニューに戻る"
	]
	
	# 簡易版：メッセージで表示
	dialogue_ui.show_message("System", "ゲーム終了")
	
	await get_tree().create_timer(1.0).timeout
	
	# メニューに戻る
	AdventureGameState.reset_game()
	AdventureGameState.change_scene("res://samples/main_menu.tscn")
