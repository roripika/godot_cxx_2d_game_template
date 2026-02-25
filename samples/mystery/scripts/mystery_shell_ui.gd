extends Node2D

@onready var ui_guide_label: Label = $SystemUiLayer/UiGuideLabel
@onready var health_label: Label = $SystemUiLayer/HealthLabel
@onready var inventory_btn: Button = $SystemUiLayer/InventoryButton
@onready var inventory_ui: EvidenceInventoryUI = $InstantSubInfoUiLayer/InventoryUI
@onready var scenario_runner = $ScenarioRunner

func _ready() -> void:
	if scenario_runner:
		scenario_runner.register_mystery_actions()
	
	if inventory_btn:
		inventory_btn.pressed.connect(_on_inventory_btn_pressed)
		
	_connect_localization_service()
	_refresh_locale()

func _on_inventory_btn_pressed() -> void:
	if inventory_ui:
		if inventory_ui.visible:
			inventory_ui.call("hide_inventory")
		else:
			inventory_ui.call("show_inventory")

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

