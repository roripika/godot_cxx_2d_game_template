extends Node2D

# ─── モード定義 ───────────────────────────────────────────
enum WarehouseMode { INVESTIGATE, TALK }
var current_mode: WarehouseMode = WarehouseMode.INVESTIGATE

# ─── UI参照 ───────────────────────────────────────────────
@onready var hint_label: Label            = $LocalUiLayer/Hint
@onready var exit_label: Label            = $LocalUiLayer/ExitVisual/Label
@onready var mode_btn:   Button           = $LocalUiLayer/ModeToggleButton

# ─── 証拠ノード（調査モードのみ表示）────────────────────────
@onready var ecto_visual: Sprite2D = get_node_or_null("FloorVisual")
@onready var foot_visual: Sprite2D = get_node_or_null("FootprintsVisual")
@onready var memo_visual: Sprite2D = get_node_or_null("MemoVisual")

# 証拠ホットスポットのArea2D
@onready var hs_floor:     Node2D = get_node_or_null("hs_floor_area")
@onready var hs_footprints: Node2D = get_node_or_null("hs_footprints")
@onready var hs_memo:      Node2D = get_node_or_null("hs_memo")

# ─── NPCノード（会話モードのみ表示）─────────────────────────
@onready var npc_tanaka:  Node2D = get_node_or_null("hs_manager")
@onready var npc_sato:    Node2D = get_node_or_null("hs_worker")
@onready var npc_suzuki:  Node2D = get_node_or_null("hs_delivery")
@onready var npc_kenta:   Node2D = get_node_or_null("hs_kenta")

# ─── 準備 ─────────────────────────────────────────────────
func _ready() -> void:
	_connect_localization_service()
	_refresh_locale()
	if mode_btn:
		mode_btn.pressed.connect(_on_mode_toggled)
	_apply_mode()

# ─── モード切替 ────────────────────────────────────────────
func _on_mode_toggled() -> void:
	if current_mode == WarehouseMode.INVESTIGATE:
		current_mode = WarehouseMode.TALK
	else:
		current_mode = WarehouseMode.INVESTIGATE
	_apply_mode()

func _apply_mode() -> void:
	var in_investigate := (current_mode == WarehouseMode.INVESTIGATE)
	var inventory := _get_inventory()

	# --- 証拠品ビジュアル：調査モードかつ未入手のみ表示 ---
	if ecto_visual:
		ecto_visual.visible = in_investigate and (inventory == null or not inventory.has_evidence("ectoplasm"))
	if foot_visual:
		foot_visual.visible = in_investigate and (inventory == null or not inventory.has_evidence("footprint"))
	if memo_visual:
		memo_visual.visible = in_investigate and (inventory == null or not inventory.has_evidence("torn_memo"))

	# 証拠ホットスポットのコリジョン有効/無効
	if hs_floor:     hs_floor.set_process_mode(PROCESS_MODE_INHERIT if in_investigate else PROCESS_MODE_DISABLED)
	if hs_footprints: hs_footprints.set_process_mode(PROCESS_MODE_INHERIT if in_investigate else PROCESS_MODE_DISABLED)
	if hs_memo:      hs_memo.set_process_mode(PROCESS_MODE_INHERIT if in_investigate else PROCESS_MODE_DISABLED)

	# --- NPCビジュアル：会話モードのみ表示 ---
	if npc_tanaka:
		npc_tanaka.visible = not in_investigate
		npc_tanaka.set_process_mode(PROCESS_MODE_INHERIT if not in_investigate else PROCESS_MODE_DISABLED)
	if npc_sato:
		npc_sato.visible = not in_investigate
		npc_sato.set_process_mode(PROCESS_MODE_INHERIT if not in_investigate else PROCESS_MODE_DISABLED)
	if npc_suzuki:
		npc_suzuki.visible = not in_investigate
		npc_suzuki.set_process_mode(PROCESS_MODE_INHERIT if not in_investigate else PROCESS_MODE_DISABLED)
	if npc_kenta:
		npc_kenta.visible = not in_investigate
		npc_kenta.set_process_mode(PROCESS_MODE_INHERIT if not in_investigate else PROCESS_MODE_DISABLED)

	# --- 探偵の立ち絵：調査モード中は非表示（クリックを邪魔しないためと演出上の理由） ---
	var dui := _get_dialogue_ui()
	if dui:
		if in_investigate:
			dui.call("clear_portrait")

	# --- モード切替ボタンのラベル更新 ---
	if mode_btn:
		mode_btn.text = tr("mystery.ui.warehouse_mode_talk") if in_investigate else tr("mystery.ui.warehouse_mode_investigate")

	# --- ヒントラベル更新 ---
	if hint_label:
		hint_label.text = tr("mystery.ui.warehouse_hint_investigate") if in_investigate else tr("mystery.ui.warehouse_hint_talk")

# ─── 証拠入手後の再反映（_process で継続監視）─────────────
func _process(_delta: float) -> void:
	if current_mode == WarehouseMode.INVESTIGATE:
		_apply_mode()

# ─── インベントリ取得 ──────────────────────────────────────
func _get_inventory() -> EvidenceInventoryUI:
	return get_tree().root.find_child("InventoryUI", true, false) as EvidenceInventoryUI

func _get_dialogue_ui() -> Control:
	return get_tree().root.find_child("DialogueUI", true, false)

# ─── ローカライズ ──────────────────────────────────────────
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
	if exit_label:
		exit_label.text = tr("mystery.ui.warehouse_exit")
	_apply_mode()
