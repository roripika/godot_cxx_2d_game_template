extends Node

const CAPTURE_ENV := "MYSTERY_UI_CAPTURE"

var state_label: Label = null

func _ready() -> void:
	if OS.get_environment(CAPTURE_ENV) != "1":
		return

	await get_tree().process_frame

	_setup_state_label()
	_apply_base_state()
	_set_state_text("CAPTURE: BASE")

	await _wait_frames(25)
	_apply_inventory_state()
	_set_state_text("CAPTURE: INVENTORY OVERLAY")

	await _wait_frames(25)
	_apply_testimony_state()
	_set_state_text("CAPTURE: TESTIMONY")

	await _wait_frames(25)
	_apply_all_state()
	_set_state_text("CAPTURE: ALL VISIBLE")

func _wait_frames(count: int) -> void:
	for _i in range(count):
		await get_tree().process_frame

func _setup_state_label() -> void:
	var layer = get_node_or_null("../SystemUiLayer")
	if layer == null:
		return

	state_label = Label.new()
	state_label.name = "CaptureStateLabel"
	state_label.anchor_left = 0.74
	state_label.anchor_top = 0.12
	state_label.anchor_right = 0.98
	state_label.anchor_bottom = 0.17
	state_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	state_label.autowrap_mode = TextServer.AUTOWRAP_WORD
	layer.add_child(state_label)

func _set_state_text(text: String) -> void:
	if state_label != null:
		state_label.text = text

func _apply_base_state() -> void:
	var dialogue = _dialogue_ui()
	var inventory = _inventory_ui()
	var testimony = _testimony_ui()

	if dialogue:
		dialogue.visible = true
		var n = dialogue.get_node_or_null("VBoxContainer/NameLabel")
		var t = dialogue.get_node_or_null("VBoxContainer/TextLabel")
		if n:
			n.text = "System"
		if t:
			t.text = "UIポリシー確認: ベース状態"

	if inventory:
		inventory.visible = false

	if testimony:
		testimony.visible = false

func _apply_inventory_state() -> void:
	_apply_base_state()
	var inventory = _inventory_ui()
	if inventory:
		if inventory.has_method("show_inventory"):
			inventory.show_inventory()
		else:
			inventory.visible = true

func _apply_testimony_state() -> void:
	_apply_base_state()
	var testimony = _testimony_ui()
	if testimony:
		testimony.visible = true
		var s = testimony.get_node_or_null("VBoxContainer/SpeakerLabel")
		var t = testimony.get_node_or_null("VBoxContainer/TestimonyText")
		if s:
			s.text = "Witness"
		if t:
			t.text = "UIポリシー確認: 証言パネル状態"

func _apply_all_state() -> void:
	_apply_testimony_state()
	var inventory = _inventory_ui()
	if inventory:
		if inventory.has_method("show_inventory"):
			inventory.show_inventory()
		else:
			inventory.visible = true

func _dialogue_ui() -> Control:
	return get_node_or_null("../MainInfoUiLayer/DialogueUI") as Control

func _inventory_ui() -> Control:
	return get_node_or_null("../InstantSubInfoUiLayer/InventoryUI") as Control

func _testimony_ui() -> Control:
	return get_node_or_null("../MainInfoUiLayer/TestimonySystem") as Control
