extends Node3D

# --- モード定義 (2D版と共通) ---
enum WarehouseMode { INVESTIGATE, TALK }
var current_mode: WarehouseMode = WarehouseMode.TALK

# --- UI参照 ---
@onready var hint_label: Label            = $LocalUiLayer/Hint
@onready var mode_btn:   Button           = $LocalUiLayer/ModeToggleButton
@onready var exit_label: Label            = get_node_or_null("LocalUiLayer/ExitVisual/Label")

# --- 3D 証拠・NPC ノード ---
@onready var hs_footprints: Area3D = $hs_footprints
@onready var hs_manager:    Area3D = $hs_manager
@onready var hs_worker:     Area3D = $hs_worker
@onready var camera:        Camera3D = $Camera3D

# ノードと Scenario ホットスポット ID のマッピング
const HOTSPOT_MAP = {
	"hs_footprints": "footprint_spot",
	"hs_manager": "tanaka_spot",
	"hs_worker": "sato_spot"
}

var dialogue_ui = null
var current_hidden_npc: Node3D = null

func _ready() -> void:
	_connect_localization_service()
	_refresh_locale()
	if mode_btn:
		mode_btn.pressed.connect(_on_mode_toggled)
	_apply_mode()
	
	dialogue_ui = get_tree().root.find_child("DialogueUI", true, false)
	if dialogue_ui:
		if dialogue_ui.has_signal("portrait_shown"):
			dialogue_ui.portrait_shown.connect(_on_portrait_shown)
		if dialogue_ui.has_signal("portrait_hidden"):
			dialogue_ui.portrait_hidden.connect(_on_portrait_hidden)

# --- 立ち絵連携 (3D Billboard NPC) ---
func _on_portrait_shown(speaker_id: String) -> void:
	if current_hidden_npc:
		current_hidden_npc.visible = true
		current_hidden_npc = null
	
	match speaker_id:
		"tanaka":
			if hs_manager and hs_manager.visible:
				current_hidden_npc = hs_manager
				hs_manager.visible = false
		"sato":
			if hs_worker and hs_worker.visible:
				current_hidden_npc = hs_worker
				hs_worker.visible = false

func _on_portrait_hidden() -> void:
	if current_hidden_npc:
		current_hidden_npc.visible = true
		current_hidden_npc = null

# --- モード切替 ---
func _on_mode_toggled() -> void:
	if current_mode == WarehouseMode.INVESTIGATE:
		current_mode = WarehouseMode.TALK
	else:
		current_mode = WarehouseMode.INVESTIGATE
	_apply_mode()

func _apply_mode() -> void:
	var in_investigate := (current_mode == WarehouseMode.INVESTIGATE)
	var inventory = _get_inventory()

	# 証拠品：調査モードかつ未入手のみ表示
	if hs_footprints:
		var has_foot = inventory != null and inventory.has_evidence("footprint")
		hs_footprints.visible = in_investigate and not has_foot
		hs_footprints.set_process_mode(PROCESS_MODE_INHERIT if in_investigate and not has_foot else PROCESS_MODE_DISABLED)

	# NPC：会話モードのみ表示 (confrontation 中は runner 側で制御されるが、ここではシンプルに)
	var sr = get_tree().root.find_child("ScenarioRunner", true, false)
	var is_confrontation: bool = (sr != null and str(sr.get("current_mode")) == "confrontation")
	var show_npcs: bool = (not in_investigate) and (not is_confrontation)
	
	if hs_manager:
		hs_manager.visible = show_npcs
		hs_manager.set_process_mode(PROCESS_MODE_INHERIT if show_npcs else PROCESS_MODE_DISABLED)
	if hs_worker:
		hs_worker.visible = show_npcs
		hs_worker.set_process_mode(PROCESS_MODE_INHERIT if show_npcs else PROCESS_MODE_DISABLED)

	# UI 更新
	if mode_btn:
		mode_btn.text = tr("mystery.ui.warehouse_mode_talk") if in_investigate else tr("mystery.ui.warehouse_mode_investigate")
		mode_btn.text += " (3D)"
	if hint_label:
		hint_label.text = tr("mystery.ui.warehouse_hint_investigate") if in_investigate else tr("mystery.ui.warehouse_hint_talk")
		hint_label.text += " (3D Prototype)"

# --- 3D インタラクション (Raycast) ---
func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		_perform_raycast(event.position)

func _perform_raycast(screen_pos: Vector2) -> void:
	if not camera: return
	
	var runner = get_tree().root.find_child("ScenarioRunner", true, false)
	if not runner or runner.is_running(): return
	
	# レイの開始点と方向を計算
	var from = camera.project_ray_origin(screen_pos)
	var to = from + camera.project_ray_normal(screen_pos) * 1000.0
	
	# 物理空間の状態を取得
	var space_state = get_world_3d().direct_space_state
	var query = PhysicsRayQueryParameters3D.create(from, to)
	query.collide_with_areas = true
	query.collide_with_bodies = true
	
	var result = space_state.intersect_ray(query)
	if result:
		var hit_collider = result.collider
		if hit_collider is Area3D:
			var node_name = hit_collider.name
			if HOTSPOT_MAP.has(node_name):
				var hs_id = HOTSPOT_MAP[node_name]
				print("[Warehouse3D] Raycast hit: ", node_name, " -> Hotspot: ", hs_id)
				runner.trigger_hotspot_by_id(hs_id)

# --- ユーティリティ ---
func _get_inventory():
	return get_tree().root.find_child("InventoryUI", true, false)

func _connect_localization_service() -> void:
	var service := get_tree().root.get_node_or_null("KarakuriLocalization")
	if service and service.has_signal("locale_changed"):
		service.connect("locale_changed", Callable(self, "_on_locale_changed"))

func _on_locale_changed(_locale: String) -> void:
	_refresh_locale()

func _refresh_locale() -> void:
	if exit_label:
		exit_label.text = tr("mystery.ui.warehouse_exit")
	_apply_mode()
