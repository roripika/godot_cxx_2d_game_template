# HotspotManager.gd
# 調査シーンのホットスポット管理
extends Node
class_name HotspotManager

class Hotspot:
	var id: String
	var area: Area2D  # クリック領域
	var name_str: String
	var description: String
	var evidence_id: String = ""
	var requires_flag: String = ""
	var examined: bool = false
	var on_click: Callable = func(): pass
	
	func _init(p_id: String, p_area: Area2D, p_name: String, p_desc: String):
		id = p_id
		area = p_area
		name_str = p_name
		description = p_desc

signal hotspot_clicked(hotspot: Hotspot)
signal evidence_found(evidence_id: String)
signal interaction_started(hotspot: Hotspot)
signal interaction_finished(hotspot: Hotspot)

var hotspots: Array[Hotspot] = []
var interaction_manager: Node = null

func _ready():
	# InteractionManagerを自動検索
	if interaction_manager == null:
		interaction_manager = get_parent().find_child("InteractionManager")
	
	if interaction_manager and interaction_manager.has_signal("clicked_at"):
		interaction_manager.clicked_at.connect(_on_clicked_at)

func add_hotspot(id: String, area: Area2D, name_str: String, description: String) -> Hotspot:
	"""ホットスポットを追加"""
	var hs = Hotspot.new(id, area, name_str, description)
	hotspots.append(hs)
	return hs

func set_evidence_for_hotspot(hotspot_id: String, evidence_id: String):
	"""ホットスポットに証拠品を設定"""
	for hs in hotspots:
		if hs.id == hotspot_id:
			hs.evidence_id = evidence_id
			break

func set_required_flag(hotspot_id: String, flag_name: String):
	"""ホットスポットに必要なフラグを設定"""
	for hs in hotspots:
		if hs.id == hotspot_id:
			hs.requires_flag = flag_name
			break

func set_hotspot_callback(hotspot_id: String, callback: Callable):
	"""ホットスポットのコールバックを設定"""
	for hs in hotspots:
		if hs.id == hotspot_id:
			hs.on_click = callback
			break

func _on_clicked_at(pos: Vector2):
	"""クリック位置をチェック"""
	for hs in hotspots:
		if not hs.area:
			continue
		var hs_pos = hs.area.global_position
		var col = hs.area.get_node_or_null("CollisionShape2D")
		if not col:
			continue
		var hs_shape = col.shape as Shape2D
		
		# 簡易判定：円形で判定
		if hs_shape is CircleShape2D:
			var dist = hs_pos.distance_to(pos)
			if dist <= hs_shape.radius:
				_trigger_hotspot(hs)
				return
		# または矩形判定
		elif hs_shape is RectangleShape2D:
			var rect = Rect2(hs_pos - hs_shape.size / 2, hs_shape.size)
			if rect.has_point(pos):
				_trigger_hotspot(hs)
				return

func _trigger_hotspot(hotspot: Hotspot):
	"""ホットスポットを発動"""
	# フラグチェック
	if hotspot.requires_flag != "" and not AdventureGameState.get_flag(hotspot.requires_flag):
		return
	
	interaction_started.emit(hotspot)
	
	# 証拠品があれば追加
	if hotspot.evidence_id != "":
		AdventureGameState.add_item(hotspot.evidence_id)
		evidence_found.emit(hotspot.evidence_id)
		hotspot.examined = true
	
	# カスタムコールバック実行
	if hotspot.on_click:
		hotspot.on_click.call()
	
	hotspot_clicked.emit(hotspot)
	interaction_finished.emit(hotspot)

func get_hotspot_by_id(id: String) -> Hotspot:
	"""IDからホットスポットを取得"""
	for hs in hotspots:
		if hs.id == id:
			return hs
	return null

func is_hotspot_examined(id: String) -> bool:
	"""ホットスポットが調査済みか確認"""
	var hs = get_hotspot_by_id(id)
	return hs != null and hs.examined
