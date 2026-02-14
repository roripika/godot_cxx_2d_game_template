# language_switcher_scene.gd
# 言語切り替えメニュー - 任意のシーンに追加可能
extends Control
class_name LanguageSwitcher

@onready var english_button: Button
@onready var japanese_button: Button
@onready var translated_label: Label

func _ready():
	# ボタンを動的に作成
	_create_buttons()
	
	# 翻訳されたラベルを取得
	translated_label = get_node_or_null("CenterContainer/VBoxContainer/TranslatedLabel")
	
	# 現在の言語でボタン状態を更新
	_update_button_states()
	_update_translated_text()

func _create_buttons():
	"""言語切り替えボタンを作成"""
	# HBoxContainerを作成
	var hbox = HBoxContainer.new()
	hbox.name = "LanguageButtons"
	hbox.set_anchors_preset(Control.PRESET_TOP_RIGHT)
	hbox.offset_left = -220  # 右から220px
	hbox.offset_top = 10
	hbox.offset_right = -10  # 右から10px
	hbox.offset_bottom = 60
	add_child(hbox)
	
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

func _on_english_pressed():
	TranslationServer.set_locale("en")
	_update_button_states()
	_update_translated_text()

func _on_japanese_pressed():
	TranslationServer.set_locale("ja")
	_update_button_states()
	_update_translated_text()

func _update_button_states():
	var current_locale = TranslationServer.get_locale()
	if english_button:
		english_button.disabled = (current_locale == "en")
	if japanese_button:
		japanese_button.disabled = (current_locale == "ja")

func _update_translated_text():
	"""テスト用：翻訳されたテキストを表示"""
	if translated_label:
		translated_label.text = tr("office_title") + " / " + tr("warehouse_title")
