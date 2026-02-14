# WarehouseConfrontationScene.gd
# 対決パート（倉庫 - 尋問）
extends Node2D
class_name WarehouseConfrontationScene

@onready var dialogue_ui = $CanvasLayer/DialogueUI
@onready var inventory_ui = $CanvasLayer/InventoryUI
@onready var testimony_system = $CanvasLayer/TestimonySystem
@onready var health_bar = $CanvasLayer/HealthBar

var is_testimony_complete: bool = false

func _ready():
	# UIを初期化
	if testimony_system:
		testimony_system.set_dialogue_ui(dialogue_ui)
		testimony_system.set_inventory_ui(inventory_ui)
		testimony_system.all_rounds_complete.connect(_on_testimony_complete)
	
	# 初期メッセージ
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("confrontation_intro"))
	
	await get_tree().create_timer(1.0).timeout
	_setup_testimonies()
	_start_confrontation()

func _setup_testimonies():
	"""証言を設定"""
	if not testimony_system:
		return
	
	# 3ラウンド、各3つの証言
	var testimonies_data = [
		# ラウンド1
		{
			"speaker": "容疑者",
			"text": tr("testimony_1"),
			"evidence": "footprint",
			"shake": tr("press_response")
		},
		{
			"speaker": "容疑者",
			"text": tr("testimony_2"),
			"evidence": "torn_memo",
			"shake": tr("press_response")
		},
		{
			"speaker": "容疑者",
			"text": tr("testimony_3"),
			"evidence": "ectoplasm",
			"shake": tr("press_response")
		}
	]
	
	for t_data in testimonies_data:
		testimony_system.add_testimony(
			t_data["speaker"],
			t_data["text"],
			t_data["evidence"],
			t_data["shake"]
		)

func _start_confrontation():
	"""尋問を開始"""
	if testimony_system:
		testimony_system.start_testimony()

func _on_testimony_complete(success: bool):
	"""尋問が完了した"""
	is_testimony_complete = true
	
	if success:
		_on_victory()
	else:
		_on_defeat()

func _on_victory():
	"""勝利"""
	if dialogue_ui:
		dialogue_ui.show_message("容疑者", tr("victory"))
	
	AdventureGameState.set_flag("case_solved", true)
	# Perfect = 全証拠 + ノーダメージ（=HPが減っていない）
	if AdventureGameState.get_flag("all_evidence_collected") and AdventureGameState.get_health() >= 3:
		AdventureGameState.set_flag("perfect_ending", true)
	
	await get_tree().create_timer(2.0).timeout
	AdventureGameState.change_scene("res://samples/mystery/ending.tscn")

func _on_defeat():
	"""敗北"""
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("ending_failure"))
	
	await get_tree().create_timer(2.0).timeout
	AdventureGameState.set_flag("game_over", true)
	AdventureGameState.change_scene("res://samples/mystery/ending.tscn")
