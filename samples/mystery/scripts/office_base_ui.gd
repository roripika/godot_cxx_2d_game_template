extends Node2D

@onready var hs_boss: Area2D = get_node_or_null("hs_boss")
@onready var door_label: Label = get_node_or_null("DoorVisual/Label")
var dialogue_ui = null

func _ready() -> void:
	_connect_localization_service()
	_refresh_locale()
	# Try to find DialogueUIAdvanced globally
	dialogue_ui = get_tree().root.find_child("DialogueUI", true, false)
	if dialogue_ui:
		dialogue_ui.portrait_shown.connect(_on_portrait_shown)
		dialogue_ui.portrait_hidden.connect(_on_portrait_hidden)

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
	if door_label:
		door_label.text = tr("mystery.ui.office_door")

func _on_portrait_shown(speaker_id: String) -> void:
	if speaker_id == "boss" and hs_boss:
		hs_boss.visible = false

func _on_portrait_hidden() -> void:
	if hs_boss:
		hs_boss.visible = true
