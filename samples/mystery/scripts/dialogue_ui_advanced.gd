extends Control
class_name DialogueUIAdvanced

signal choice_selected(index: int, text: String)
signal dialogue_finished()

@onready var ui_part_label: Label = get_node_or_null("UiPartLabel")
@onready var name_label: Label = get_node_or_null("VBoxContainer/NameLabel")
@onready var text_label: Label = get_node_or_null("VBoxContainer/TextLabel")
@onready var choices_container: VBoxContainer = get_node_or_null("VBoxContainer/ChoicesContainer")
@onready var portrait_rect: TextureRect = get_node_or_null("PortraitRect")

@export var typing_speed: float = 0.05
@export var text_color: Color = Color.WHITE
@export var name_color: Color = Color.YELLOW

var is_typing: bool = false
var current_text: String = ""

var _message_speaker_key: String = ""
var _message_speaker_text: String = ""
var _message_text_key: String = ""
var _message_text: String = ""
var _choice_defs: Array[Dictionary] = []
var _mode_input_enabled: bool = true
var _waiting_for_click: bool = false

func _ready() -> void:
	visible = false
	_clear_choices()
	_connect_localization_service()
	_refresh_locale()

func show_message(speaker: String, text: String) -> void:
	show_message_with_keys("", speaker, "", text)

func show_message_with_keys(speaker_key: String, speaker: String, text_key: String, text: String) -> void:
	visible = true
	_message_speaker_key = speaker_key
	_message_speaker_text = speaker
	_message_text_key = text_key
	_message_text = text
	_choice_defs.clear()
	_clear_choices()
	_update_portrait(speaker_key if speaker_key != "" else speaker)
	_apply_message_name()
	_type_text(_resolve_text(_message_text_key, _message_text))

func show_choices(choices: Array) -> int:
	var defs: Array[Dictionary] = []
	for choice in choices:
		defs.append({"text": str(choice), "text_key": ""})
	return await show_choices_with_defs(defs)

func show_choices_with_defs(choice_defs: Array[Dictionary]) -> int:
	_choice_defs = []
	for i in range(choice_defs.size()):
		var src := choice_defs[i]
		if src == null:
			continue
		var d: Dictionary = src
		_choice_defs.append({
			"text": str(d.get("text", "")),
			"text_key": str(d.get("text_key", "")),
		})

	_rebuild_choices()
	var args = await choice_selected
	return int(args[0])

func hide_dialogue() -> void:
	visible = false
	_choice_defs.clear()
	_clear_choices()

func set_portrait(texture: Texture2D) -> void:
	if portrait_rect:
		portrait_rect.texture = texture
		portrait_rect.visible = true

func clear_portrait() -> void:
	if portrait_rect:
		portrait_rect.visible = false

func _update_portrait(speaker_name: String) -> void:
	var portrait_id = ""
	match speaker_name:
		"Detective", "探偵", "speaker.detective": portrait_id = "detective"
		"Boss", "所長", "speaker.boss": portrait_id = "boss"
		"Rat Witness", "倉庫管理人", "ネズミの証人", "容疑者", "speaker.rat_witness": portrait_id = "rat_witness"
	
	if portrait_id != "":
		var path = "res://assets/mystery/characters/%s.png" % portrait_id
		if ResourceLoader.exists(path):
			set_portrait(load(path))
		else:
			clear_portrait()
	else:
		clear_portrait()

func skip_typing() -> void:
	if not is_typing:
		return
	if text_label:
		text_label.text = current_text
	is_typing = false
	_waiting_for_click = true

func _type_text(text: String) -> void:
	if is_typing:
		return

	is_typing = true
	current_text = text
	if text_label:
		text_label.modulate = text_color
		text_label.text = ""

	if typing_speed <= 0.0:
		if text_label:
			text_label.text = text
		is_typing = false
		_waiting_for_click = true
		return

	for i in range(len(text)):
		if not is_typing:
			return
		if text_label:
			text_label.text += str(text[i])
		await get_tree().create_timer(typing_speed).timeout

	if is_typing:
		is_typing = false
		_waiting_for_click = true

func _on_choice_pressed(index: int, text: String) -> void:
	if not _mode_input_enabled:
		return
	_clear_choices()
	choice_selected.emit(index, text)

func _rebuild_choices() -> void:
	_clear_choices()
	for i in range(_choice_defs.size()):
		var def: Dictionary = _choice_defs[i]
		var choice_btn := Button.new()
		var choice_text := _resolve_text(str(def.get("text_key", "")), str(def.get("text", "")))
		choice_btn.text = choice_text
		choice_btn.disabled = not _mode_input_enabled
		choice_btn.pressed.connect(_on_choice_pressed.bind(i, choice_text))
		if choices_container:
			choices_container.add_child(choice_btn)

func _apply_message_name() -> void:
	if not name_label:
		return
	name_label.modulate = name_color
	name_label.text = _resolve_text(_message_speaker_key, _message_speaker_text)

func _resolve_text(key: String, fallback: String) -> String:
	if key.is_empty():
		return fallback
	return tr(key)

func _refresh_locale() -> void:
	if ui_part_label:
		ui_part_label.text = tr("mystery.ui.dialogue_panel")
	if _message_speaker_key != "" or _message_speaker_text != "":
		_apply_message_name()
	if _message_text_key != "":
		current_text = _resolve_text(_message_text_key, _message_text)
		if is_typing:
			skip_typing()
		if text_label:
			text_label.text = current_text
	if _choice_defs.size() > 0:
		_rebuild_choices()

func on_mode_enter(_mode_id: String, _scene_id: String) -> void:
	_refresh_locale()

func on_mode_exit(_mode_id: String, _next_scene_id: String) -> void:
	_clear_choices()

func set_mode_input_enabled(enabled: bool) -> void:
	_mode_input_enabled = enabled
	_rebuild_choices()

func _connect_localization_service() -> void:
	var service := get_tree().root.get_node_or_null("KarakuriLocalization")
	if service == null:
		return
	if service.has_signal("locale_changed"):
		var cb := Callable(self, "_on_locale_changed")
		if not service.is_connected("locale_changed", cb):
			service.connect("locale_changed", cb)

func _on_locale_changed(_locale: String) -> void:
	_refresh_locale()

func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if is_typing:
			skip_typing()
			get_viewport().set_input_as_handled()
		elif _waiting_for_click:
			_waiting_for_click = false
			dialogue_finished.emit()
			get_viewport().set_input_as_handled()

func _clear_choices() -> void:
	if not choices_container:
		return
	for child in choices_container.get_children():
		child.queue_free()

