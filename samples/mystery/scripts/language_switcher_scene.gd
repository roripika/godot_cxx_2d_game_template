extends Control
class_name LanguageSwitcher

@onready var english_button: Button
@onready var japanese_button: Button
@onready var translated_label: Label

func _ready() -> void:
	_create_ui()
	_connect_localization_service()
	_update_ui()

func _create_ui() -> void:
	var vbox := VBoxContainer.new()
	vbox.name = "LanguageWidget"
	vbox.set_anchors_preset(Control.PRESET_TOP_RIGHT)
	vbox.offset_left = -320
	vbox.offset_top = 10
	vbox.offset_right = -10
	vbox.offset_bottom = 90
	add_child(vbox)

	var hbox := HBoxContainer.new()
	hbox.name = "LanguageButtons"
	vbox.add_child(hbox)

	english_button = Button.new()
	english_button.text = "English"
	english_button.custom_minimum_size = Vector2(100, 40)
	english_button.pressed.connect(_on_english_pressed)
	hbox.add_child(english_button)

	japanese_button = Button.new()
	japanese_button.text = "日本語"
	japanese_button.custom_minimum_size = Vector2(100, 40)
	japanese_button.pressed.connect(_on_japanese_pressed)
	hbox.add_child(japanese_button)

	translated_label = Label.new()
	translated_label.name = "TranslatedLabel"
	translated_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	vbox.add_child(translated_label)

func _connect_localization_service() -> void:
	var service := _localization_service()
	if service == null:
		return
	if service.has_signal("locale_changed"):
		var cb := Callable(self, "_on_locale_changed")
		if not service.is_connected("locale_changed", cb):
			service.connect("locale_changed", cb)

func _on_english_pressed() -> void:
	_apply_locale_prefix("en")

func _on_japanese_pressed() -> void:
	_apply_locale_prefix("ja")

func _apply_locale_prefix(prefix: String) -> void:
	var service := _localization_service()
	if service and service.has_method("set_locale_prefix"):
		service.call("set_locale_prefix", prefix)
	else:
		_set_locale_by_prefix_fallback(prefix)
	_update_ui()

func _set_locale_by_prefix_fallback(prefix: String) -> void:
	for loc in TranslationServer.get_loaded_locales():
		if String(loc).begins_with(prefix):
			TranslationServer.set_locale(String(loc))
			return
	TranslationServer.set_locale(prefix)

func _on_locale_changed(_locale: String) -> void:
	_update_ui()

func _update_ui() -> void:
	var current_locale := TranslationServer.get_locale()
	if english_button:
		english_button.disabled = current_locale.begins_with("en")
	if japanese_button:
		japanese_button.disabled = current_locale.begins_with("ja")

	if translated_label:
		translated_label.text = tr("office_title") + " / " + tr("warehouse_title") + " (" + current_locale + ")"

func on_mode_enter(_mode_id: String, _scene_id: String) -> void:
	_update_ui()

func on_mode_exit(_mode_id: String, _next_scene_id: String) -> void:
	pass

func set_mode_input_enabled(enabled: bool) -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE if enabled else Control.MOUSE_FILTER_STOP
	if english_button:
		english_button.disabled = not enabled or TranslationServer.get_locale().begins_with("en")
	if japanese_button:
		japanese_button.disabled = not enabled or TranslationServer.get_locale().begins_with("ja")

func _localization_service() -> Node:
	return get_tree().root.get_node_or_null("KarakuriLocalization")
