extends Control

## Language Selector UI
## 言語切り替えUI - can be added to any scene

@onready var english_button = $EnglishButton
@onready var japanese_button = $JapaneseButton

func _ready():
	# Set up buttons
	if english_button:
		english_button.pressed.connect(_on_english_pressed)
	if japanese_button:
		japanese_button.pressed.connect(_on_japanese_pressed)
	
	# Initialize with current locale
	_update_button_states()

func _on_english_pressed():
	TranslationServer.set_locale("en")
	_update_button_states()

func _on_japanese_pressed():
	TranslationServer.set_locale("ja")
	_update_button_states()

func _update_button_states():
	var current_locale = TranslationServer.get_locale()
	if english_button:
		english_button.disabled = (current_locale == "en")
	if japanese_button:
		japanese_button.disabled = (current_locale == "ja")
