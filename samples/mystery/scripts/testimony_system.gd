extends Control
class_name TestimonySystem

signal next_requested()
signal shake_requested()
signal present_requested()

@onready var ui_part_label: Label = get_node_or_null("UiPartLabel")
@onready var speaker_label: Label = get_node_or_null("VBoxContainer/SpeakerLabel")
@onready var testimony_text: Label = get_node_or_null("VBoxContainer/TestimonyText")
@onready var progress_label: Label = get_node_or_null("VBoxContainer/ProgressLabel")
@onready var next_btn: Button = get_node_or_null("VBoxContainer/ActionContainer/NextButton")
@onready var shake_btn: Button = get_node_or_null("VBoxContainer/ActionContainer/ShakeButton")
@onready var present_btn: Button = get_node_or_null("VBoxContainer/ActionContainer/PresentButton")

var _speaker_key: String = ""
var _speaker_text: String = ""
var _text_key: String = ""
var _text_text: String = ""
var _mode_input_enabled: bool = true
var _ui_ready: bool = false

func _ready() -> void:
	_ui_ready = true
	visible = false
	_connect_buttons()
	_connect_localization_service()
	_refresh_locale()

func show_panel() -> void:
	visible = true

func hide_panel() -> void:
	visible = false

func show_testimony_line(speaker: String, text: String) -> void:
	show_testimony_line_with_keys("", speaker, "", text)

func show_testimony_line_with_keys(speaker_key: String, speaker: String, text_key: String, text: String) -> void:
	_speaker_key = speaker_key
	_speaker_text = speaker
	_text_key = text_key
	_text_text = text
	_apply_texts()

func set_line_progress(current_index: int, total_count: int) -> void:
	if not _ui_ready or progress_label == null:
		return
	var fmt := tr("mystery.ui.testimony_progress")
	if fmt.find("%") == -1:
		fmt = "%d / %d"
	progress_label.text = fmt % [current_index, total_count]

func set_actions_enabled(enabled: bool) -> void:
	if not _ui_ready:
		return
	if next_btn:
		next_btn.disabled = not enabled
	if shake_btn:
		shake_btn.disabled = not enabled
	if present_btn:
		present_btn.disabled = not enabled

func on_mode_enter(mode_id: String, _scene_id: String) -> void:
	visible = (mode_id == "confrontation")
	_refresh_locale()

func on_mode_exit(_mode_id: String, _next_scene_id: String) -> void:
	visible = false

func set_mode_input_enabled(enabled: bool) -> void:
	_mode_input_enabled = enabled
	set_actions_enabled(enabled)

func _connect_buttons() -> void:
	if next_btn:
		next_btn.pressed.connect(_on_next_pressed)
	if shake_btn:
		shake_btn.pressed.connect(_on_shake_pressed)
	if present_btn:
		present_btn.pressed.connect(_on_present_pressed)

func _on_next_pressed() -> void:
	if _mode_input_enabled:
		next_requested.emit()

func _on_shake_pressed() -> void:
	if _mode_input_enabled:
		shake_requested.emit()

func _on_present_pressed() -> void:
	if _mode_input_enabled:
		present_requested.emit()

func _refresh_locale() -> void:
	if not _ui_ready:
		return
	if ui_part_label:
		ui_part_label.text = tr("mystery.ui.testimony_panel")
	if next_btn:
		next_btn.text = tr("button_next")
	if shake_btn:
		shake_btn.text = tr("button_press")
	if present_btn:
		present_btn.text = tr("button_present")
	_apply_texts()
	if progress_label and progress_label.text.is_empty():
		var fmt := tr("mystery.ui.testimony_progress")
		if fmt.find("%") == -1:
			fmt = "%d / %d"
		progress_label.text = fmt % [1, 1]

func _apply_texts() -> void:
	if not _ui_ready:
		return
	if speaker_label:
		speaker_label.text = _resolve_text(_speaker_key, _speaker_text)
	if testimony_text:
		testimony_text.text = _resolve_text(_text_key, _text_text)

func _resolve_text(key: String, fallback: String) -> String:
	if key.is_empty():
		return fallback
	return tr(key)

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
