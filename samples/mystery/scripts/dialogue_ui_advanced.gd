# DialogueUIAdvanced.gd
# 拡張会話UI（選択肢、立ち絵、タイプライター対応）
extends Control
class_name DialogueUIAdvanced

# シグナル
signal choice_selected(index: int, text: String)
signal dialogue_finished()

# UI コンポーネント
@onready var name_label = $VBoxContainer/NameLabel
@onready var text_label = $VBoxContainer/TextLabel
@onready var choices_container = $VBoxContainer/ChoicesContainer
@onready var portrait_rect = $PortraitRect

# 設定
@export var typing_speed: float = 0.05  # 文字表示速度（秒）
@export var text_color: Color = Color.WHITE
@export var name_color: Color = Color.YELLOW

var is_typing: bool = false
var current_text: String = ""

func _ready():
	# UIを初期状態で非表示
	visible = false
	choices_container.clear()

func show_message(speaker: String, text: String):
	"""基本的なメッセージ表示"""
	visible = true
	name_label.text = speaker
	name_label.modulate = name_color
	
	# 前回のテキストがあればクリア
	choices_container.clear()
	
	# タイプライター効果で表示
	_type_text(text)

func _type_text(text: String):
	"""タイプライター効果でテキストを表示"""
	if is_typing:
		return
	
	is_typing = true
	current_text = text
	text_label.text = ""
	
	for i in range(len(text)):
		text_label.text += text[i]
		await get_tree().create_timer(typing_speed).timeout
	
	is_typing = false
	dialogue_finished.emit()

func show_choices(choices: Array[String]) -> int:
	"""
	選択肢を表示して、選択されるまで待つ
	戻り値: 選択されたインデックス
	"""
	choices_container.clear()
	
	var choice_selected_signal = Signal(self, "choice_selected")
	
	for i in range(choices.size()):
		var choice_btn = Button.new()
		choice_btn.text = choices[i]
		choice_btn.pressed.connect(func(): 
			_on_choice_selected(i, choices[i])
		)
		choices_container.add_child(choice_btn)
	
	# 選択されるまで待つ
	var result = await choice_selected
	return result[0]

func _on_choice_selected(index: int, text: String):
	"""選択肢がクリックされた時"""
	choices_container.clear()
	choice_selected.emit([index, text])

func set_portrait(texture: Texture2D):
	"""立ち絵を設定"""
	if portrait_rect:
		portrait_rect.texture = texture
		portrait_rect.visible = true

func clear_portrait():
	"""立ち絵をクリア"""
	if portrait_rect:
		portrait_rect.visible = false

func hide_dialogue():
	"""会話UIを非表示"""
	visible = false
	choices_container.clear()

func skip_typing():
	"""タイピングアニメーションをスキップ"""
	if is_typing:
		text_label.text = current_text
		is_typing = false
		dialogue_finished.emit()

func _input(event: InputEvent):
	"""クリックでテキストスキップ"""
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if is_typing:
			skip_typing()
			get_tree().root.set_input_as_handled()
