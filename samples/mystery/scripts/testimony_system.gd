# TestimonySystem.gd
# 証言システム（法廷モード）
extends Control
class_name TestimonySystem

class Testimony:
	var speaker: String
	var text: String
	var contradiction_evidence: String  # この証言に矛盾する証拠品ID
	var shake_result: String  # ゆさぶった時の追加情報
	var correct_evidence_presented: bool = false
	
	func _init(p_speaker: String, p_text: String, p_evidence: String = "", p_shake: String = ""):
		speaker = p_speaker
		text = p_text
		contradiction_evidence = p_evidence
		shake_result = p_shake

signal testimony_started()
signal testimony_finished(success: bool)
signal round_complete()
signal all_rounds_complete(success: bool)
signal evidence_required()

@onready var speaker_label = $VBoxContainer/SpeakerLabel
@onready var testimony_text = $VBoxContainer/TestimonyText
@onready var action_buttons = $VBoxContainer/ActionContainer
@onready var next_btn = $VBoxContainer/ActionContainer/NextButton
@onready var shake_btn = $VBoxContainer/ActionContainer/ShakeButton
@onready var present_btn = $VBoxContainer/ActionContainer/PresentButton

var testimonies: Array[Testimony] = []
var current_round_idx: int = 0
var current_testimony_idx: int = 0
var max_rounds: int = 3
var round_errors: int = 0
var total_errors: int = 0

var inventory_ui: InventoryUI = null
var dialogue_ui: DialogueUIAdvanced = null

func _ready():
	visible = false
	_setup_buttons()

func _setup_buttons():
	"""ボタンの接続"""
	if next_btn:
		next_btn.pressed.connect(_on_next_pressed)
	if shake_btn:
		shake_btn.pressed.connect(_on_shake_pressed)
	if present_btn:
		present_btn.pressed.connect(_on_present_pressed)

func add_testimony(speaker: String, text: String, evidence_id: String = "", shake_text: String = "") -> Testimony:
	"""証言を追加"""
	var t = Testimony.new(speaker, text, evidence_id, shake_text)
	testimonies.append(t)
	return t

func set_inventory_ui(ui: InventoryUI):
	"""インベントリUIを設定"""
	inventory_ui = ui

func set_dialogue_ui(ui: DialogueUIAdvanced):
	"""ダイアログUIを設定"""
	dialogue_ui = ui

func start_testimony():
	"""尋問開始"""
	visible = true
	current_round_idx = 0
	current_testimony_idx = 0
	round_errors = 0
	total_errors = 0
	
	testimony_started.emit()
	_show_testimony()

func _show_testimony():
	"""現在の証言を表示"""
	if current_testimony_idx >= testimonies.size():
		_on_round_complete()
		return
	
	var testimony = testimonies[current_testimony_idx]
	speaker_label.text = testimony.speaker
	testimony_text.text = testimony.text
	
	# ボタン有効化
	_enable_buttons()

func _on_next_pressed():
	"""次の証言へ"""
	current_testimony_idx += 1
	if current_testimony_idx >= testimonies.size():
		_on_round_complete()
	else:
		_show_testimony()

func _on_shake_pressed():
	"""証言をゆさぶる"""
	if current_testimony_idx >= testimonies.size():
		return
	
	var testimony = testimonies[current_testimony_idx]
	if testimony.shake_result != "":
		# ダイアログUIで追加情報を表示
		if dialogue_ui:
			dialogue_ui.show_message(testimony.speaker, testimony.shake_result)
		else:
			print("追加情報: ", testimony.shake_result)

func _on_present_pressed():
	"""証拠品をつきつける"""
	if current_testimony_idx >= testimonies.size():
		return
	
	evidence_required.emit()
	
	# インベントリUIを表示して選択待ち
	if inventory_ui:
		inventory_ui.show_inventory()
		var selected = await inventory_ui.evidence_selected
		var evidence_id = selected
		
		_check_evidence(evidence_id)

func _check_evidence(evidence_id: String):
	"""証拠品が正しいか確認"""
	var testimony = testimonies[current_testimony_idx]
	
	if evidence_id == testimony.contradiction_evidence:
		# 正解！
		if dialogue_ui:
			dialogue_ui.show_message(testimony.speaker, "あ!...これは...")
		testimony.correct_evidence_presented = true
		current_testimony_idx += 1
		
		if current_testimony_idx >= testimonies.size():
			_on_round_complete()
		else:
			await get_tree().create_timer(1.0).timeout
			_show_testimony()
	else:
		# 不正解
		round_errors += 1
		total_errors += 1
		AdventureGameState.take_damage()
		
		if dialogue_ui:
			dialogue_ui.show_message(testimony.speaker, "その証拠は関係ない!")
		
		if round_errors >= 3:
			# ゲームオーバー
			_on_game_over()
		else:
			# 証言を最初から
			current_testimony_idx = 0
			await get_tree().create_timer(1.0).timeout
			_show_testimony()

func _on_round_complete():
	"""ラウンド完了"""
	current_round_idx += 1
	
	if current_round_idx >= max_rounds:
		# 全ラウンド完了
		_on_all_complete()
	else:
		# 次のラウンド
		current_testimony_idx = 0
		round_errors = 0
		await get_tree().create_timer(1.0).timeout
		_show_testimony()
	
	round_complete.emit()

func _on_all_complete():
	"""全ラウンド完了"""
	visible = false
	all_rounds_complete.emit(true)

func _on_game_over():
	"""ゲームオーバー"""
	visible = false
	testimony_finished.emit(false)

func _enable_buttons():
	"""ボタンを有効化"""
	if next_btn:
		next_btn.disabled = false
	if shake_btn:
		shake_btn.disabled = false
	if present_btn:
		present_btn.disabled = false

func _disable_buttons():
	"""ボタンを無効化"""
	if next_btn:
		next_btn.disabled = true
	if shake_btn:
		shake_btn.disabled = true
	if present_btn:
		present_btn.disabled = true

func get_total_errors() -> int:
	"""総ミス数を取得"""
	return total_errors

func is_perfect_round() -> bool:
	"""ノーミスクリアか"""
	return total_errors == 0
