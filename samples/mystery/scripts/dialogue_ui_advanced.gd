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

const PORTRAIT_TOP := -310.0
const PORTRAIT_BOTTOM := 220.0
const PORTRAIT_INSIDE_MARGIN := 50.0
const PORTRAIT_OUTSIDE_MARGIN := 325.0
const PORTRAIT_FADE_DURATION := 0.2

var is_typing: bool = false
var current_text: String = ""

var _message_speaker_key: String = ""
var _message_speaker_text: String = ""
var _message_text_key: String = ""
var _message_text: String = ""
var _choice_defs: Array[Dictionary] = []
var _mode_input_enabled: bool = true
var _waiting_for_click: bool = false
var _portrait_side: String = "left"
var _portrait_side_override: String = "auto"
var _portrait_enter_transition: String = "none"
var _portrait_exit_transition: String = "none"
var _portrait_tween: Tween = null

func _ready() -> void:
	visible = false
	_clear_choices()
	_connect_localization_service()
	resized.connect(_on_dialogue_resized)
	_refresh_locale()

func show_message(speaker: String, text: String) -> void:
	show_message_with_keys("", speaker, "", text)

func show_message_with_keys(speaker_key: String, speaker: String, text_key: String, text: String, portrait_side: String = "auto") -> void:
	set_portrait_side(portrait_side)
	visible = true
	_message_speaker_key = speaker_key
	_message_speaker_text = speaker
	_message_text_key = text_key
	_message_text = text
	_choice_defs.clear()
	_clear_choices()
	await _update_portrait_for_dialogue(speaker_key if speaker_key != "" else speaker)
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
		portrait_rect.modulate.a = 1.0

func clear_portrait() -> void:
	if portrait_rect:
		portrait_rect.visible = false
		portrait_rect.modulate.a = 1.0

func _update_portrait(speaker_name: String) -> void:
	var portrait_id = _resolve_portrait_id(speaker_name)
	
	if portrait_id != "":
		var path = "res://assets/mystery/characters/%s.png" % portrait_id
		if ResourceLoader.exists(path):
			set_portrait(load(path))
			_update_portrait_side(portrait_id)
			_apply_portrait_layout()
		else:
			clear_portrait()
	else:
		clear_portrait()

func _resolve_portrait_id(speaker_name: String) -> String:
	match speaker_name:
		"Detective", "探偵", "speaker.detective":
			return "detective"
		"Boss", "所長", "speaker.boss":
			return "boss"
		"Rat Witness", "倉庫管理人", "ネズミの証人", "容疑者", "speaker.rat_witness":
			return "rat_witness"
		_:
			return ""

func _update_portrait_for_dialogue(speaker_name: String) -> void:
	if not portrait_rect:
		return

	var portrait_id := _resolve_portrait_id(speaker_name)
	var had_visible_portrait := portrait_rect.visible and portrait_rect.texture != null

	if _portrait_exit_transition == "fade_out" and had_visible_portrait:
		await _fade_portrait_to(0.0, PORTRAIT_FADE_DURATION)

	if portrait_id == "":
		clear_portrait()
		_reset_portrait_transitions()
		return

	var path = "res://assets/mystery/characters/%s.png" % portrait_id
	if not ResourceLoader.exists(path):
		clear_portrait()
		_reset_portrait_transitions()
		return

	set_portrait(load(path))
	_update_portrait_side(portrait_id)
	_apply_portrait_layout()

	if _portrait_enter_transition == "fade_in":
		portrait_rect.modulate.a = 0.0
		await _fade_portrait_to(1.0, PORTRAIT_FADE_DURATION)
	else:
		portrait_rect.modulate.a = 1.0

	_reset_portrait_transitions()

func _fade_portrait_to(alpha: float, duration: float) -> void:
	if not portrait_rect:
		return
	if _portrait_tween and _portrait_tween.is_valid():
		_portrait_tween.kill()
	_portrait_tween = portrait_rect.create_tween()
	_portrait_tween.tween_property(portrait_rect, "modulate:a", alpha, duration).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	await _portrait_tween.finished

func _reset_portrait_transitions() -> void:
	_portrait_enter_transition = "none"
	_portrait_exit_transition = "none"

func _update_portrait_side(portrait_id: String) -> void:
	if _portrait_side_override != "auto":
		_portrait_side = _portrait_side_override
		return
	match portrait_id:
		"boss", "rat_witness":
			_portrait_side = "right"
		_:
			_portrait_side = "left"

func _apply_portrait_layout() -> void:
	if not portrait_rect:
		return

	portrait_rect.offset_top = PORTRAIT_TOP
	portrait_rect.offset_bottom = PORTRAIT_BOTTOM
	
	var panel_width := size.x
	if panel_width <= 0.0:
		panel_width = 900.0

	match _portrait_side:
		"right":
			portrait_rect.offset_left = panel_width - PORTRAIT_INSIDE_MARGIN
			portrait_rect.offset_right = panel_width + PORTRAIT_OUTSIDE_MARGIN
		"center":
			portrait_rect.offset_left = (panel_width * 0.5) - (PORTRAIT_OUTSIDE_MARGIN * 0.5)
			portrait_rect.offset_right = (panel_width * 0.5) + (PORTRAIT_OUTSIDE_MARGIN * 0.5)
		_:
			portrait_rect.offset_left = -PORTRAIT_OUTSIDE_MARGIN
			portrait_rect.offset_right = PORTRAIT_INSIDE_MARGIN

func _on_dialogue_resized() -> void:
	if portrait_rect and portrait_rect.visible:
		_apply_portrait_layout()

func set_portrait_side(side: String) -> void:
	var normalized := side.strip_edges().to_lower()
	if normalized == "":
		normalized = "auto"
	match normalized:
		"left", "right", "center", "auto":
			_portrait_side_override = normalized
		_:
			_portrait_side_override = "auto"
	if portrait_rect and portrait_rect.visible:
		_apply_portrait_layout()

func set_portrait_enter(mode: String) -> void:
	var normalized := mode.strip_edges().to_lower()
	if normalized in ["fade", "fade_in", "in"]:
		_portrait_enter_transition = "fade_in"
	else:
		_portrait_enter_transition = "none"

func set_portrait_exit(mode: String) -> void:
	var normalized := mode.strip_edges().to_lower()
	if normalized in ["fade", "fade_out", "out"]:
		_portrait_exit_transition = "fade_out"
	else:
		_portrait_exit_transition = "none"

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
	var localized := tr(key)
	if localized != "":
		return localized
	if fallback != "":
		return fallback
	return key

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
	if not visible:
		return
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
