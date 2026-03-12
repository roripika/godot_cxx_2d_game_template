extends Node2D

@onready var ui_guide_label: Label = $SystemUiLayer/UiGuideLabel
@onready var health_label: Label = $SystemUiLayer/HealthLabel
@onready var inventory_btn: Button = $SystemUiLayer/InventoryButton
@onready var inventory_ui: EvidenceInventoryUI = $InstantSubInfoUiLayer/InventoryUI
@onready var portrait_rect: TextureRect = $PortraitLayer/PortraitContainer/PortraitRect
@onready var scene_container: Node2D = $SceneContainer
@onready var scenario_runner = $ScenarioRunner

func _ready() -> void:
	# Note: Mystery actions are now registered automatically in MysteryGameState._ready()
	
	# Connect to MysteryGameState (C++ Singleton via Autoload)
	var mgs = MysteryGameState.get_singleton()
	if mgs:
		mgs.portrait_requested.connect(_on_portrait_requested)
		mgs.background_requested.connect(_on_background_requested)
		mgs.set_reset_hook(Callable(self, "_on_mystery_reset"))
	
	if inventory_btn:
		inventory_btn.pressed.connect(_on_inventory_btn_pressed)
		
	_connect_scenario_runner_signals()
	_connect_localization_service()
	_refresh_locale()

func _connect_scenario_runner_signals() -> void:
	if not scenario_runner: return
	
	scenario_runner.dialogue_requested.connect(_on_dialogue_requested)
	scenario_runner.choices_requested.connect(_on_choices_requested)
	scenario_runner.mode_entered.connect(_on_mode_entered)
	scenario_runner.mode_exited.connect(_on_mode_exited)
	scenario_runner.mode_input_enabled_changed.connect(_on_mode_input_enabled_changed)
	scenario_runner.scene_change_requested.connect(_on_scene_change_requested)

func _on_scene_change_requested(scene_path: String) -> void:
	if scene_path == "" or not scene_container: return
	
	print("[Shell] Switching scene to: ", scene_path)
	# Clear existing scene
	for child in scene_container.get_children():
		child.queue_free()
		
	# Instance new scene
	if ResourceLoader.exists(scene_path):
		var packed = load(scene_path)
		var inst = packed.instantiate()
		scene_container.add_child(inst)
	else:
		printerr("[Shell] Scene NOT FOUND: ", scene_path)

func _on_dialogue_requested(speaker: String, text: String) -> void:
	var dialogue_ui = get_node_or_null("MainInfoUiLayer/DialogueUI")
	if dialogue_ui and dialogue_ui.has_method("show_message"):
		dialogue_ui.show_message(speaker, text)

func _on_choices_requested(choices: Array) -> void:
	var dialogue_ui = get_node_or_null("MainInfoUiLayer/DialogueUI")
	if dialogue_ui and dialogue_ui.has_method("show_choices"):
		dialogue_ui.show_choices(choices)

func _on_mode_entered(mode_id: String, scene_id: String) -> void:
	var dialogue_ui = get_node_or_null("MainInfoUiLayer/DialogueUI")
	if dialogue_ui and dialogue_ui.has_method("on_mode_enter"):
		dialogue_ui.on_mode_enter(mode_id, scene_id)

func _on_mode_exited(current_mode_id: String, next_scene_id: String) -> void:
	var dialogue_ui = get_node_or_null("MainInfoUiLayer/DialogueUI")
	if dialogue_ui and dialogue_ui.has_method("on_mode_exit"):
		dialogue_ui.on_mode_exit(current_mode_id, next_scene_id)

func _on_mode_input_enabled_changed(enabled: bool) -> void:
	var dialogue_ui = get_node_or_null("MainInfoUiLayer/DialogueUI")
	if dialogue_ui and dialogue_ui.has_method("set_mode_input_enabled"):
		dialogue_ui.set_mode_input_enabled(enabled)

func _on_inventory_btn_pressed() -> void:
	if inventory_ui:
		if inventory_ui.visible:
			inventory_ui.call("hide_inventory")
		else:
			inventory_ui.call("show_inventory")

## Mystery 固有のリセット処理（DI: AdventureGameStateBase.set_reset_hook で登録）
## - EvidenceManager, MysteryManager はシーンルートを動的に検索して実行
func _on_mystery_reset() -> void:
	var root := get_tree().root
	for child in root.get_children():
		if child.is_class("EvidenceManager") and child.has_method("clear_all_evidence"):
			child.clear_all_evidence()
		if child.is_class("MysteryManager") and child.has_method("deserialize_flags"):
			child.deserialize_flags({})

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

func _refresh_locale() -> void:
	if ui_guide_label: ui_guide_label.text = tr("mystery.ui.guide")
	if health_label: health_label.text = tr("mystery.ui.health_label")
	if inventory_btn: inventory_btn.text = tr("mystery.ui.inventory_button")

# --- 演出ハンドラ (C++ 信号の受け皿) ---------------------------------

func _on_portrait_requested(character_id: String, emotion: String) -> void:
	if character_id == "" or character_id == "none":
		portrait_rect.texture = null
		return
		
	# アセットパスの解決 (res://assets/mystery/characters/portraits/{id}.png)
	var path = "res://assets/mystery/characters/portraits/%s.png" % character_id
	# TODO: emotion (表情) に応じたパス分岐が必要ならここで実装
	
	if ResourceLoader.exists(path):
		var tex = load(path)
		portrait_rect.texture = tex
		portrait_rect.visible = true
		print("[View] Portrait changed to: ", character_id, " (Path: ", path, ")")
		print("[View] PortraitRect Status - Visible: ", portrait_rect.visible, " Size: ", portrait_rect.size, " Texture: ", portrait_rect.texture)
	else:
		printerr("[View] Portrait NOT FOUND: ", path)

func _on_background_requested(background_id: String) -> void:
	if background_id == "":
		return
		
	# アセットパスの解決 (res://assets/mystery/backgrounds/bg_{id}.png)
	var path = "res://assets/mystery/backgrounds/bg_%s.png" % background_id
	
	if ResourceLoader.exists(path):
		var tex = load(path)
		# SceneContainer 内の既存の背景ノードを探すか、Sprite2D を生成
		var bg_node = scene_container.get_node_or_null("BackgroundSprite")
		if not bg_node:
			bg_node = Sprite2D.new()
			bg_node.name = "BackgroundSprite"
			bg_node.centered = false
			scene_container.add_child(bg_node)
		
		bg_node.texture = tex
		print("[View] Background changed to: ", background_id)
	else:
		printerr("[View] Background NOT FOUND: ", path)

