extends Node3D

@onready var camera: Camera3D = $Camera3D
@onready var hs_boss: Area3D = $hs_boss
@onready var hs_door: Area3D = $hs_door

const HOTSPOT_MAP = {
	"hs_boss": "boss_spot",
	"hs_door": "door_spot"
}

var dialogue_ui = null
var current_hidden_npc: Node3D = null

func _ready() -> void:
	dialogue_ui = get_tree().root.find_child("DialogueUI", true, false)
	if dialogue_ui:
		if dialogue_ui.has_signal("portrait_shown"):
			dialogue_ui.portrait_shown.connect(_on_portrait_shown)
		if dialogue_ui.has_signal("portrait_hidden"):
			dialogue_ui.portrait_hidden.connect(_on_portrait_hidden)

# --- 立体空間内でのキャラ・立ち絵排他制御 ---
func _on_portrait_shown(speaker_id: String) -> void:
	if current_hidden_npc:
		current_hidden_npc.visible = true
		current_hidden_npc = null
	
	if speaker_id == "boss":
		if hs_boss and hs_boss.visible:
			current_hidden_npc = hs_boss
			hs_boss.visible = false

func _on_portrait_hidden() -> void:
	if current_hidden_npc:
		current_hidden_npc.visible = true
		current_hidden_npc = null

# --- 3D インタラクション (Raycast) ---
func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		_perform_raycast(event.position)

func _perform_raycast(screen_pos: Vector2) -> void:
	if not camera: return
	
	var runner = get_tree().root.find_child("ScenarioRunner", true, false)
	if not runner or runner.is_running(): return
	
	var from = camera.project_ray_origin(screen_pos)
	var to = from + camera.project_ray_normal(screen_pos) * 1000.0
	
	var space_state = get_world_3d().direct_space_state
	var query = PhysicsRayQueryParameters3D.create(from, to)
	query.collide_with_areas = true
	
	var result = space_state.intersect_ray(query)
	if result:
		var hit_collider = result.collider
		if hit_collider is Area3D:
			var node_name = hit_collider.name
			if HOTSPOT_MAP.has(node_name):
				var hs_id = HOTSPOT_MAP[node_name]
				print("[Office3D] Raycast hit: ", node_name, " -> Hotspot: ", hs_id)
				runner.trigger_hotspot_by_id(hs_id)
