extends Control
class_name EvidenceInventoryUI

signal evidence_selected(evidence_id: String)

@onready var ui_part_label: Label = get_node_or_null("UiPartLabel")
@onready var list_label: Label = get_node_or_null("VBoxContainer/Label")
@onready var evidence_grid: GridContainer = get_node_or_null("VBoxContainer/GridContainer")
@onready var detail_panel: Control = get_node_or_null("VBoxContainer/DetailPanel")
@onready var detail_name: Label = get_node_or_null("VBoxContainer/DetailPanel/VBoxContainer/NameLabel")
@onready var detail_desc: Label = get_node_or_null("VBoxContainer/DetailPanel/VBoxContainer/DescriptionLabel")
@onready var detail_icon: TextureRect = get_node_or_null("VBoxContainer/DetailPanel/VBoxContainer/IconRect")

var evidence_list: Array[EvidenceItem] = []
var selected_evidence: EvidenceItem = null
var evidence_resources: Dictionary = {}
var _mode_input_enabled: bool = true
var _ui_ready: bool = false

func _ready() -> void:
	_ui_ready = true
	visible = false
	if detail_panel:
		detail_panel.visible = false
	_load_evidence_resources()
	_connect_localization_service()
	_refresh_static_labels()

func _load_evidence_resources() -> void:
	var evidence_dir := "res://samples/mystery/data/evidence/"
	var dir := DirAccess.open(evidence_dir)
	if dir == null:
		return

	dir.list_dir_begin()
	var file_name := dir.get_next()
	while file_name != "":
		if file_name.ends_with(".tres"):
			var evidence := load(evidence_dir + file_name) as EvidenceItem
			if evidence:
				evidence_resources[evidence.id] = evidence
		file_name = dir.get_next()
	dir.list_dir_end()

func add_evidence(evidence_id: String) -> void:
	if not evidence_resources.has(evidence_id):
		return
	var evidence: EvidenceItem = evidence_resources[evidence_id]
	if evidence_list.has(evidence):
		return
	evidence_list.append(evidence)
	_refresh_ui()

func remove_evidence(evidence_id: String) -> void:
	for i in range(evidence_list.size()):
		if evidence_list[i].id == evidence_id:
			evidence_list.remove_at(i)
			_refresh_ui()
			break

func has_evidence(evidence_id: String) -> bool:
	for evidence in evidence_list:
		if evidence.id == evidence_id:
			return true
	return false

func show_inventory() -> void:
	visible = true
	_refresh_ui()

func hide_inventory() -> void:
	visible = false

func get_selected_evidence_id() -> String:
	if selected_evidence == null:
		return ""
	return selected_evidence.id

func _refresh_ui() -> void:
	if not _ui_ready:
		return
	if evidence_grid == null:
		return
	_refresh_static_labels()
	for child in evidence_grid.get_children():
		child.queue_free()

	for evidence in evidence_list:
		var btn := Button.new()
		btn.text = tr("evidence_" + evidence.id)
		btn.custom_minimum_size = Vector2(140, 50)
		btn.disabled = not _mode_input_enabled
		if evidence.icon:
			btn.icon = evidence.icon
		btn.pressed.connect(_on_evidence_selected.bind(evidence))
		evidence_grid.add_child(btn)

	if selected_evidence:
		_apply_detail(selected_evidence)

func _on_evidence_selected(evidence: EvidenceItem) -> void:
	if not _mode_input_enabled:
		return
	selected_evidence = evidence
	_apply_detail(evidence)
	evidence_selected.emit(evidence.id)

func _apply_detail(evidence: EvidenceItem) -> void:
	if not _ui_ready:
		return
	if detail_panel == null:
		return
	detail_panel.visible = true
	if detail_name:
		detail_name.text = tr("evidence_" + evidence.id)
	if detail_desc:
		detail_desc.text = tr("evidence_" + evidence.id + "_desc")
	if detail_icon:
		detail_icon.texture = evidence.icon if evidence.icon else null

func get_evidence_by_id(id: String) -> EvidenceItem:
	return evidence_resources.get(id)

func on_mode_enter(_mode_id: String, _scene_id: String) -> void:
	_refresh_ui()

func on_mode_exit(_mode_id: String, _next_scene_id: String) -> void:
	hide_inventory()

func set_mode_input_enabled(enabled: bool) -> void:
	_mode_input_enabled = enabled
	_refresh_ui()

func _refresh_static_labels() -> void:
	if not _ui_ready:
		return
	if ui_part_label:
		ui_part_label.text = tr("mystery.ui.inventory_panel")
	if list_label:
		list_label.text = tr("mystery.ui.inventory_list")
	if selected_evidence == null:
		if detail_name:
			detail_name.text = tr("mystery.ui.detail_name_placeholder")
		if detail_desc:
			detail_desc.text = tr("mystery.ui.detail_desc_placeholder")

func _connect_localization_service() -> void:
	var service := get_tree().root.get_node_or_null("KarakuriLocalization")
	if service == null:
		return
	if service.has_signal("locale_changed"):
		var cb := Callable(self, "_on_locale_changed")
		if not service.is_connected("locale_changed", cb):
			service.connect("locale_changed", cb)

func _on_locale_changed(_locale: String) -> void:
	_refresh_ui()
