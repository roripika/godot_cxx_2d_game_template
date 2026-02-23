extends Node2D

@onready var door_label: Label = $DoorVisual/Label

func _ready() -> void:
	_connect_localization_service()
	_refresh_locale()

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
