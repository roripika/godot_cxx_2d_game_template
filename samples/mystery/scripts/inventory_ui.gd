# InventoryUI.gd
# 証拠品インベントリUI
extends Control
class_name InventoryUI

signal evidence_selected(evidence_id: String)

@onready var evidence_grid = $VBoxContainer/GridContainer
@onready var detail_panel = $VBoxContainer/DetailPanel
@onready var detail_name = $VBoxContainer/DetailPanel/VBoxContainer/NameLabel
@onready var detail_desc = $VBoxContainer/DetailPanel/VBoxContainer/DescriptionLabel
@onready var detail_icon = $VBoxContainer/DetailPanel/VBoxContainer/IconRect

var evidence_list: Array[EvidenceItem] = []
var selected_evidence: EvidenceItem = null
var evidence_resources: Dictionary = {}  # id -> EvidenceItem

func _ready():
	visible = false
	detail_panel.visible = false
	_load_evidence_resources()

func _load_evidence_resources():
	"""証拠品リソースをロード"""
	var evidence_dir = "res://samples/mystery/data/evidence/"
	var dir = DirAccess.open(evidence_dir)
	
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if file_name.ends_with(".tres"):
				var evidence = load(evidence_dir + file_name) as EvidenceItem
				if evidence:
					evidence_resources[evidence.id] = evidence
			file_name = dir.get_next()

func add_evidence(evidence_id: String):
	"""証拠品を追加表示"""
	if evidence_resources.has(evidence_id):
		var evidence = evidence_resources[evidence_id]
		if not evidence_list.has(evidence):
			evidence_list.append(evidence)
			_refresh_ui()

func remove_evidence(evidence_id: String):
	"""証拠品を削除"""
	for i in range(evidence_list.size()):
		if evidence_list[i].id == evidence_id:
			evidence_list.remove_at(i)
			_refresh_ui()
			break

func has_evidence(evidence_id: String) -> bool:
	"""証拠品を所持しているか確認"""
	for e in evidence_list:
		if e.id == evidence_id:
			return true
	return false

func show_inventory():
	"""インベントリUIを表示"""
	visible = true
	_refresh_ui()

func hide_inventory():
	"""インベントリUIを非表示"""
	visible = false

func _refresh_ui():
	"""UIを更新"""
	# グリッドをクリア
	for child in evidence_grid.get_children():
		child.queue_free()
	
	# 証拠品ボタンを追加
	for evidence in evidence_list:
		var btn = Button.new()
		btn.text = tr("evidence_" + evidence.id)  # 翻訳キーを使用
		btn.custom_minimum_size = Vector2(100, 50)
		if evidence.icon:
			btn.icon = evidence.icon
		btn.pressed.connect(func(): _on_evidence_selected(evidence))
		evidence_grid.add_child(btn)

func _on_evidence_selected(evidence: EvidenceItem):
	"""証拠品が選択された"""
	selected_evidence = evidence
	detail_panel.visible = true
	detail_name.text = tr("evidence_" + evidence.id)  # 翻訳キーを使用
	detail_desc.text = tr("evidence_" + evidence.id + "_desc")  # 翻訳キーを使用
	if evidence.icon:
		detail_icon.texture = evidence.icon

func select_evidence() -> EvidenceItem:
	"""ユーザーが選択した証拠品を返す"""
	if selected_evidence:
		evidence_selected.emit(selected_evidence.id)
		return selected_evidence
	return null

func get_evidence_by_id(id: String) -> EvidenceItem:
	"""IDから証拠品を取得"""
	return evidence_resources.get(id)
