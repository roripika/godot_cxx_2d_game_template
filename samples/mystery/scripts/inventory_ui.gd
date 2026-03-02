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
@onready var present_btn: Button = get_node_or_null("VBoxContainer/DetailPanel/VBoxContainer/PresentButton")
@onready var close_btn: Button = get_node_or_null("CloseButton")

var evidence_list: Array[EvidenceItem] = []
var selected_evidence: EvidenceItem = null
var evidence_resources: Dictionary = {}
var _mode_input_enabled: bool = true
var _presentation_mode: bool = false
var _ui_ready: bool = false

func _ready() -> void:
	_ui_ready = true
	visible = false
	if close_btn:
		close_btn.pressed.connect(_on_close_pressed)
	if present_btn:
		present_btn.pressed.connect(_on_present_pressed)
	_load_evidence_resources()
	_clear_detail()
	_connect_localization_service()
	_refresh_static_labels()

func _on_close_pressed() -> void:
	if not _mode_input_enabled:
		return
	hide_inventory()
	evidence_selected.emit("")

func _load_evidence_resources() -> void:
	var yaml_path := "res://samples/mystery/data/items.yaml"
	if not FileAccess.file_exists(yaml_path):
		var gpath = ProjectSettings.globalize_path(yaml_path)
		if not FileAccess.file_exists(gpath):
			print("[Inventory] WARNING: items.yaml not found at ", yaml_path)
			return
		yaml_path = gpath
	
	var file = FileAccess.open(yaml_path, FileAccess.READ)
	if not file:
		return
	var content = file.get_as_text()
	file.close()
	
	_parse_items_yaml(content)

func _parse_items_yaml(yaml_text: String) -> void:
	var lines = yaml_text.split("\n")
	var current_item_id = ""
	var current_item: EvidenceItem = null
	
	for i in range(lines.size()):
		var line = lines[i]
		var raw = line
		var stripped = line.strip_edges()
		if stripped == "" or stripped.begins_with("#"):
			continue
			
		if stripped == "items:":
			continue
			
		# YAMLの階層をインデントで判定 (スペース数)
		var indent_level = raw.length() - raw.lstrip(" \t").length()
		
		if (indent_level == 2 or indent_level == 1) and stripped.ends_with(":"):
			current_item_id = stripped.trim_suffix(":")
			current_item = EvidenceItem.new()
			current_item.id = current_item_id
			evidence_resources[current_item_id] = current_item
			continue
			
		if (indent_level == 4 or indent_level == 2) and current_item != null:
			if ":" in stripped:
				var parts = stripped.split(":", true, 1)
				var key = parts[0].strip_edges()
				var val = parts[1].strip_edges().trim_prefix("\"").trim_suffix("\"").trim_prefix("'").trim_suffix("'")
				
				if key == "sort_order":
					current_item.sort_order = val.to_int()
				elif key == "name_key":
					current_item.display_name = val
				elif key == "desc_key":
					current_item.description = val
				elif key == "icon":
					if ResourceLoader.exists(val):
						current_item.icon = load(val)
					else:
						var img_path = ProjectSettings.globalize_path(val)
						if FileAccess.file_exists(img_path):
							var img = Image.load_from_file(img_path)
							if img:
								current_item.icon = ImageTexture.create_from_image(img)

func add_evidence(evidence_id: String) -> void:
	if not evidence_resources.has(evidence_id):
		return
	var evidence: EvidenceItem = evidence_resources[evidence_id]
	if evidence_list.has(evidence):
		return
	evidence_list.append(evidence)
	evidence_list.sort_custom(_sort_evidence)
	selected_evidence = evidence # 自動選択
	_refresh_ui()
	_apply_detail(evidence)

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
	_presentation_mode = false
	visible = true
	mouse_filter = MOUSE_FILTER_STOP
	_update_present_button()
	_refresh_ui()

func show_inventory_for_presentation() -> void:
	_presentation_mode = true
	visible = true
	mouse_filter = MOUSE_FILTER_STOP
	_update_present_button()
	_refresh_ui()

func hide_inventory() -> void:
	_presentation_mode = false
	visible = false
	mouse_filter = MOUSE_FILTER_IGNORE

func get_selected_evidence_id() -> String:
	if selected_evidence == null:
		return ""
	return selected_evidence.id

func _sort_evidence(a: EvidenceItem, b: EvidenceItem) -> bool:
	return a.sort_order < b.sort_order

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
		btn.custom_minimum_size = Vector2(140, 60)
		btn.disabled = not _mode_input_enabled
		btn.expand_icon = true
		if evidence.icon:
			btn.icon = evidence.icon
		btn.pressed.connect(_on_evidence_selected.bind(evidence))
		evidence_grid.add_child(btn)

	if selected_evidence:
		_apply_detail(selected_evidence)
	_update_present_button()

func _on_evidence_selected(evidence: EvidenceItem) -> void:
	if not _mode_input_enabled:
		return
	selected_evidence = evidence
	_apply_detail(evidence)
	# シグナルは発行しない。「突きつける」ボタンで発行する。
	_update_present_button()

func _on_present_pressed() -> void:
	if not _mode_input_enabled or not _presentation_mode:
		return
	if selected_evidence == null:
		return
	hide_inventory()
	evidence_selected.emit(selected_evidence.id)

func _update_present_button() -> void:
	if present_btn == null:
		return
	present_btn.visible = _presentation_mode
	present_btn.disabled = not _mode_input_enabled or selected_evidence == null
	if _presentation_mode:
		present_btn.text = tr("mystery.ui.present_evidence")

func _apply_detail(evidence: EvidenceItem) -> void:
	if not _ui_ready or detail_panel == null:
		return
	detail_panel.visible = (evidence != null)
	if not evidence:
		_clear_detail()
		return
		
	if detail_name:
		detail_name.text = tr("evidence_" + evidence.id)
	if detail_desc:
		detail_desc.text = tr("evidence_" + evidence.id + "_desc")
	if detail_icon:
		detail_icon.texture = evidence.icon
		detail_icon.visible = (evidence.icon != null)

func _clear_detail() -> void:
	if detail_panel:
		detail_panel.visible = false
	if detail_name:
		detail_name.text = ""
	if detail_desc:
		detail_desc.text = ""
	if detail_icon:
		detail_icon.texture = null
		detail_icon.visible = false

func get_evidence_by_id(id: String) -> EvidenceItem:
	return evidence_resources.get(id)

func on_mode_enter(_mode_id: String, _scene_id: String) -> void:
	_refresh_ui()

func on_mode_exit(_mode_id: String, _next_scene_id: String) -> void:
	hide_inventory()

func set_mode_input_enabled(enabled: bool) -> void:
	_mode_input_enabled = enabled
	_refresh_ui()
	_update_present_button()

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
