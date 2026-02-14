# WarehouseInvestigationScene.gd
# 調査シーン（倉庫）
extends Node2D
class_name WarehouseInvestigationScene

@onready var dialogue_ui = $CanvasLayer/DialogueUI
@onready var inventory_ui = $CanvasLayer/InventoryUI
@onready var interaction_manager = $InteractionManager
@onready var hotspot_manager = $HotspotManager

# ホットスポット定義
var hotspots_data = [
	{
		"id": "floor_area",
		"name": "床",
		"description": "床には何か妙な物質が落ちている...",
		"evidence": "ectoplasm",
		"pos": Vector2(520, 380),
		"radius": 60
	},
	{
		"id": "footprints",
		"name": "足跡",
		"description": "大量の足跡がある。人間のものではなさそうだ。",
		"evidence": "footprint",
		"pos": Vector2(330, 450),
		"radius": 60
	},
	{
		"id": "memo",
		"name": "破れたメモ",
		"description": "メモが落ちている。「午前3時に...」という文字が見える。",
		"evidence": "torn_memo",
		"pos": Vector2(740, 310),
		"radius": 60
	},
	{
		"id": "exit",
		"name": "出口",
		"description": "オフィスに戻る",
		"evidence": "",
		"pos": Vector2(70, 320),
		"radius": 90
	}
]

func _ready():
	# 初期メッセージ
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("warehouse_intro"))
	
	# ホットスポット設定
	_setup_hotspots()
	
	# シグナル接続
	if hotspot_manager:
		hotspot_manager.evidence_found.connect(_on_evidence_found)
		hotspot_manager.hotspot_clicked.connect(_on_hotspot_clicked)

func _setup_hotspots():
	"""ホットスポットを設定"""
	if not hotspot_manager:
		return
	
	for hs_data in hotspots_data:
		# ホットスポット用のエリアを作成（簡易版）
		var area = Area2D.new()
		area.name = hs_data["id"]
		if hs_data.has("pos"):
			area.position = hs_data["pos"]
		add_child(area)
		
		# CollisionShape2D を追加
		var collision = CollisionShape2D.new()
		var shape = CircleShape2D.new()
		shape.radius = hs_data.get("radius", 50)
		collision.shape = shape
		area.add_child(collision)
		
		# ホットスポットマネージャに登録
		var hotspot = hotspot_manager.add_hotspot(
			hs_data["id"],
			area,
			hs_data["name"],
			hs_data["description"]
		)
		
		if hs_data["evidence"] != "":
			hotspot_manager.set_evidence_for_hotspot(hs_data["id"], hs_data["evidence"])

func _on_evidence_found(evidence_id: String):
	"""証拠品が見つかった時"""
	if dialogue_ui:
		var evidence_name = tr("evidence_" + evidence_id)
		dialogue_ui.show_message("Detective", tr("evidence_found") + " [" + evidence_name + "]")
	
	# インベントリUIを更新
	if inventory_ui:
		inventory_ui.add_evidence(evidence_id)

func _on_hotspot_clicked(hotspot):
	"""ホットスポットがクリックされた"""
	if hotspot.id == "exit":
		_exit_warehouse()
	elif dialogue_ui:
		# ホットスポットの説明を表示（翻訳キーを使用）
		var hotspot_key = "hotspot_" + hotspot.id
		dialogue_ui.show_message("Detective", tr(hotspot_key))

func _get_evidence_name(evidence_id: String) -> String:
	"""証拠品IDから名前を取得"""
	match evidence_id:
		"ectoplasm":
			return "エクトプラズム"
		"footprint":
			return "血の足跡"
		"torn_memo":
			return "破れたメモ"
	return evidence_id

func _exit_warehouse():
	"""倉庫から出る"""
	if dialogue_ui:
		dialogue_ui.show_message("System", tr("return_to_office"))
	
	await get_tree().create_timer(1.0).timeout
	AdventureGameState.change_scene("res://samples/mystery/office_scene.tscn")

func _process(delta):
	# 全証拠品を集めたか確認
	if AdventureGameState.has_item("ectoplasm") and \
	   AdventureGameState.has_item("footprint") and \
	   AdventureGameState.has_item("torn_memo"):
		if not AdventureGameState.get_flag("all_evidence_collected"):
			AdventureGameState.set_flag("all_evidence_collected", true)
			if dialogue_ui:
				dialogue_ui.show_message("System", tr("investigation_complete"))
