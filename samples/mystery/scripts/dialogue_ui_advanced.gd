# DialogueUIAdvanced.gd
# 拡張会話UI（選択肢、立ち絵、タイプライター対応）
extends Control
class_name DialogueUIAdvanced

# シグナル
signal choice_selected(index: int, text: String)
signal dialogue_finished()

# UI コンポーネント
@onready var name_label: Label = get_node_or_null("VBoxContainer/NameLabel")
@onready var text_label: Label = get_node_or_null("VBoxContainer/TextLabel")
@onready var choices_container: VBoxContainer = get_node_or_null("VBoxContainer/ChoicesContainer")
@onready var portrait_rect: TextureRect = get_node_or_null("PortraitRect")

# 設定
@export var typing_speed: float = 0.05  # 文字表示速度（秒）
@export var text_color: Color = Color.WHITE
@export var name_color: Color = Color.YELLOW

var is_typing: bool = false
var current_text: String = ""

func _ready():
	# UIを初期状態で非表示
	visible = false
	_clear_choices()

func show_message(speaker: String, text: String):
	"""基本的なメッセージ表示"""
	visible = true
	if name_label:
		name_label.text = speaker
		name_label.modulate = name_color
	
	# 前回のテキストがあればクリア
	_clear_choices()
	
	# タイプライター効果で表示
	_type_text(text)

func _type_text(text: String):
	"""タイプライター効果でテキストを表示"""
	if is_typing:
		return
	
	is_typing = true
	current_text = text
	if text_label:
		text_label.text = ""
	
	for i in range(len(text)):
		if text_label:
			text_label.text += str(text[i])
		await get_tree().create_timer(typing_speed).timeout
	
	is_typing = false
	dialogue_finished.emit()

func show_choices(choices: Array) -> int:
	"""
	選択肢を表示して、選択されるまで待つ
	戻り値: 選択されたインデックス
	"""
	_clear_choices()
	
	for i in range(choices.size()):
		var choice_btn = Button.new()
		var choice_text := str(choices[i])
		choice_btn.text = choice_text
		choice_btn.pressed.connect(func(): 
			_on_choice_selected(i, choice_text)
		)
		if choices_container:
			choices_container.add_child(choice_btn)
	
	# 選択されるまで待つ
	var args = await choice_selected
	return int(args[0])

func _on_choice_selected(index: int, text: String):
	"""選択肢がクリックされた時"""
	_clear_choices()
	choice_selected.emit(index, text)

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
	_clear_choices()

func skip_typing():
	"""タイピングアニメーションをスキップ"""
	if is_typing:
		if text_label:
			text_label.text = current_text
		is_typing = false
		dialogue_finished.emit()

func _input(event: InputEvent):
	"""クリックでテキストスキップ"""
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if is_typing:
			skip_typing()
			get_tree().root.set_input_as_handled()

func _clear_choices():
	"""選択肢ボタンを全削除"""
	if not choices_container:
		return
	for child in choices_container.get_children():
		child.queue_free()
