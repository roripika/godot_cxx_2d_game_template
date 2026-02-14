# language_switcher_scene.gd
# 言語切り替えメニュー - 任意のシーンに追加可能
extends Control
class_name LanguageSwitcher

@onready var english_button: Button
@onready var japanese_button: Button
@onready var translated_label: Label

func _ready():
	# ボタン/ラベルを動的に作成（どのシーンにも貼れるようにする）
	_create_ui()
	_update_ui()

func _create_ui():
	"""言語切り替えUI（ボタン + 現在言語の確認用ラベル）を作成"""
	var vbox = VBoxContainer.new()
	vbox.name = "LanguageWidget"
	vbox.set_anchors_preset(Control.PRESET_TOP_RIGHT)
	vbox.offset_left = -320  # 右からの余白（ラベル込みで少し広め）
	vbox.offset_top = 10
	vbox.offset_right = -10
	vbox.offset_bottom = 90
	add_child(vbox)
	
	var hbox = HBoxContainer.new()
	hbox.name = "LanguageButtons"
	vbox.add_child(hbox)
	
	# English button
	english_button = Button.new()
	english_button.text = "English"
	english_button.custom_minimum_size = Vector2(100, 40)
	english_button.pressed.connect(_on_english_pressed)
	hbox.add_child(english_button)
	
	# Japanese button
	japanese_button = Button.new()
	japanese_button.text = "日本語"
	japanese_button.custom_minimum_size = Vector2(100, 40)
	japanese_button.pressed.connect(_on_japanese_pressed)
	hbox.add_child(japanese_button)
	
	translated_label = Label.new()
	translated_label.name = "TranslatedLabel"
	translated_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	vbox.add_child(translated_label)

func _on_english_pressed():
	_set_locale_by_prefix("en")
	_update_ui()

func _on_japanese_pressed():
	_set_locale_by_prefix("ja")
	_update_ui()

func _set_locale_by_prefix(prefix: String):
	"""
	Translation locale can be returned as "en_US" etc.
	Prefer a loaded locale that matches the prefix; fall back to the prefix itself.
	"""
	if TranslationServer.has_method("get_loaded_locales"):
		for loc in TranslationServer.get_loaded_locales():
			if String(loc).begins_with(prefix):
				TranslationServer.set_locale(String(loc))
				return
	TranslationServer.set_locale(prefix)

func _update_ui():
	var current_locale = TranslationServer.get_locale()
	# Godot は "en_US" のような値を返すことがあるので prefix で判定する
	if english_button:
		english_button.disabled = current_locale.begins_with("en")
	if japanese_button:
		japanese_button.disabled = current_locale.begins_with("ja")

	# 目視確認用: 翻訳キーが切り替わるか表示
	if translated_label:
		translated_label.text = tr("office_title") + " / " + tr("warehouse_title") + " (" + current_locale + ")"
