extends Control

@onready var start_button: Button = $VBoxContainer/StartButton
@onready var quit_button: Button = $VBoxContainer/QuitButton

func _ready() -> void:
	if start_button:
		start_button.pressed.connect(_on_start_pressed)
	if quit_button:
		quit_button.pressed.connect(_on_quit_pressed)
		# WEBプラットフォームでは終了ボタンを非表示にする
		if OS.has_feature("web"):
			quit_button.hide()

func _on_start_pressed() -> void:
	# アドベンチャーステートのリセット（念のため）
	var ags = get_tree().root.get_node_or_null("AdventureGameState")
	if ags:
		ags.call("reset_game")
	
	# シェルシーンへ遷移
	get_tree().change_scene_to_file("res://samples/mystery/karakuri_mystery_shell.tscn")

func _on_quit_pressed() -> void:
	get_tree().quit()
