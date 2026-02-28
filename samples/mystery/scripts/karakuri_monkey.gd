##
## karakuri_monkey.gd – monkey test (ランダム操作耐久テスト)
##
## 目的: 想定外の順序・タイミングで操作を投げつけてもクラッシュ・
##       無限ループ・デッドロックが起きないことを確認する。
##
## 対象: AI が実装した機能を含む C++ / GDScript 変更後の必須テスト。
##
## 検証内容:
##   - ランダムな順番でホットスポットをクリック(倉庫ホットスポット全座標)
##   - 画面外・背景を含む完全ランダム座標クリック
##   - インベントリの高速開閉 (CloseButton連打)
##   - モード切替ボタン連打 (Investigate ↔ Talk)
##   - 会話中のスキップ連打
##   - 上記をランダムに組み合わせて MONKEY_ACTIONS 回実行
##
## 実行後の不変条件チェック:
##   1. インベントリが開閉できる (デッドロックしていない)
##   2. ModeToggleButton を押すとテキストが変化する
##   3. シーンツリーが生きている (SceneContainer が存在)
##
## 使用方法:
##   ./dev.sh monkey
##   godot --headless --path . --script res://samples/mystery/scripts/karakuri_monkey.gd
##
## 環境変数:
##   MONKEY_ACTIONS=<int>   実行するランダムアクション数 (デフォルト 400)
##   MONKEY_SEED=<int>      乱数シード (0 = 時刻ベース, デフォルト 0)
##
extends SceneTree

const SHELL := "res://samples/mystery/karakuri_mystery_shell.tscn"

# ── ランダムアクション数 (環境変数で上書き可) ─────────────────
var MONKEY_ACTIONS: int = 400
# 0 = 時刻ベース乱数
var MONKEY_SEED: int = 0

# ── 倉庫シーンの既知ホットスポット座標 ────────────────────────
const HS_POSITIONS: Array = [
	Vector2(520, 320),   # hs_floor_area  (証拠: ectoplasm)
	Vector2(330, 380),   # hs_footprints  (証拠: footprint)
	Vector2(740, 250),   # hs_memo        (証拠: torn_memo)
	Vector2(900, 300),   # hs_manager     (NPC: 田中)
	Vector2(700, 450),   # hs_worker      (NPC: 佐藤)
	Vector2(200, 200),   # hs_delivery    (NPC: 鈴木)
	Vector2(400, 150),   # hs_kenta       (NPC: 健太)
	Vector2(80,  500),   # hs_exit        (出口)
]

var _failed := false
var _rng := RandomNumberGenerator.new()
var _action_count := 0

# ── 統計 ──────────────────────────────────────────────────────
var _stats := {
	"click_random": 0,
	"click_hotspot": 0,
	"mode_toggle": 0,
	"inventory_open": 0,
	"inventory_close": 0,
	"dialogue_skip": 0,
}

func _initialize() -> void:
	# 環境変数の読み込み
	var actions_env := OS.get_environment("MONKEY_ACTIONS")
	if actions_env != "":
		MONKEY_ACTIONS = int(actions_env)
		if MONKEY_ACTIONS <= 0:
			MONKEY_ACTIONS = 400
	var seed_env := OS.get_environment("MONKEY_SEED")
	if seed_env != "":
		MONKEY_SEED = int(seed_env)

	if MONKEY_SEED == 0:
		MONKEY_SEED = int(Time.get_unix_time_from_system())
	_rng.seed = MONKEY_SEED
	print("[KARAKURI_MONKEY] seed=%d  actions=%d" % [MONKEY_SEED, MONKEY_ACTIONS])
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		_failed = true
		push_error("[KARAKURI_MONKEY] FAIL: " + msg)

func _wait_frames(n: int) -> void:
	for _i in range(n):
		await process_frame

# ─── ノード取得ヘルパー ─────────────────────────────────────
func _scene_container() -> Node:
	return current_scene.get_node_or_null("SceneContainer")

func _runner() -> Node:
	return current_scene.get_node_or_null("ScenarioRunner")

func _interaction_manager() -> Node:
	return current_scene.get_node_or_null("InteractionManager")

func _dialogue_ui() -> Node:
	return current_scene.get_node_or_null("MainInfoUiLayer/DialogueUI")

func _inventory_ui() -> Node:
	return current_scene.get_node_or_null("InstantSubInfoUiLayer/InventoryUI")

# ─── プロローグスキップ ────────────────────────────────────────
func _skip_prologue() -> void:
	print("[KARAKURI_MONKEY] skipping prologue...")
	var dui := _dialogue_ui()
	if dui:
		dui.set("typing_speed", 0.0)
	# 最大3000フレーム内でWarehouseBaseが来るまでクリックし続ける
	for _i in range(3000):
		var sc := _scene_container()
		if sc and sc.get_child_count() > 0 and sc.get_child(0).name == "WarehouseBase":
			break
		_safe_clear()
		await _wait_frames(20)

func _safe_clear() -> void:
	var ui := _dialogue_ui()
	if ui == null:
		return
	if ui.get("is_typing"):
		ui.call("skip_typing")
	if ui.get("_waiting_for_click"):
		ui.set("_waiting_for_click", false)
		ui.emit_signal("dialogue_finished")
	if ui.get("_choice_defs") != null and (ui.get("_choice_defs") as Array).size() > 0:
		var runner := _runner()
		if runner:
			runner.call("on_choice_selected", 0, "")

# ─── ランダムアクション ────────────────────────────────────────

## ランダム座標クリック (画面全体 1280×720)
func _action_random_click() -> void:
	var pos := Vector2(
		_rng.randf_range(0, 1280),
		_rng.randf_range(0, 720)
	)
	_interaction_manager().emit_signal("clicked_at", pos)
	_stats["click_random"] += 1

## 既知ホットスポットのひとつをランダムにクリック
func _action_hotspot_click() -> void:
	var idx := _rng.randi() % HS_POSITIONS.size()
	_interaction_manager().emit_signal("clicked_at", HS_POSITIONS[idx])
	_stats["click_hotspot"] += 1

## モード切替ボタンを押す
func _action_mode_toggle() -> void:
	var sc := _scene_container()
	if sc == null or sc.get_child_count() == 0:
		return
	var wb := sc.get_child(0)
	if wb.name != "WarehouseBase":
		return
	var btn := wb.get_node_or_null("ModeToggleButton")
	if btn:
		btn.emit_signal("pressed")
	_stats["mode_toggle"] += 1

## インベントリを開く
func _action_inventory_open() -> void:
	var inv := _inventory_ui()
	if inv and inv.has_method("show_inventory"):
		inv.call("show_inventory")
	_stats["inventory_open"] += 1

## インベントリの CloseButton を押す
func _action_inventory_close() -> void:
	var inv := _inventory_ui()
	if inv == null:
		return
	var close_btn := inv.get_node_or_null("CloseButton")
	if close_btn:
		close_btn.emit_signal("pressed")
	_stats["inventory_close"] += 1

## 会話スキップ(ランナーが動いている最中でも安全に呼ぶ)
func _action_dialogue_skip() -> void:
	_safe_clear()
	_stats["dialogue_skip"] += 1

## ランダムにひとつのアクションを選んで実行
func _dispatch_random_action() -> void:
	# action_id を比重付きで選ぶ
	# 0-3: click_random(多め) 4-6: hotspot  7-8: toggle  9-11: inv_open  12-14: inv_close  15-19: skip
	var roll := _rng.randi() % 20
	if roll < 4:
		_action_random_click()
	elif roll < 7:
		_action_hotspot_click()
	elif roll < 9:
		_action_mode_toggle()
	elif roll < 12:
		_action_inventory_open()
	elif roll < 15:
		_action_inventory_close()
	else:
		_action_dialogue_skip()

# ─── 不変条件チェック ─────────────────────────────────────────
func _check_invariants_after_monkey() -> void:
	print("[KARAKURI_MONKEY] running post-monkey invariant checks...")

	# 1. SceneContainer が存在する
	var sc := _scene_container()
	_assert(sc != null, "SceneContainer is missing after monkey run")

	# 2. インベントリが開閉できる
	var inv := _inventory_ui()
	_assert(inv != null, "InventoryUI is missing after monkey run")
	if inv:
		# まず強制的に閉じる
		inv.call("hide_inventory")
		await _wait_frames(3)
		_assert(inv.visible == false, "inventory should be closeable after monkey run")

		inv.call("show_inventory")
		await _wait_frames(3)
		_assert(inv.visible == true, "inventory should be openable after monkey run")

		var close_btn := inv.get_node_or_null("CloseButton")
		if close_btn:
			close_btn.emit_signal("pressed")
			await _wait_frames(3)
			_assert(inv.visible == false, "CloseButton should close inventory after monkey run")

	# 3. WarehouseBase であればモード切替ボタンが機能する
	if sc != null and sc.get_child_count() > 0 and sc.get_child(0).name == "WarehouseBase":
		var wb := sc.get_child(0)
		var btn := wb.get_node_or_null("ModeToggleButton")
		if btn:
			var before: String = str(btn.get("text"))
			btn.emit_signal("pressed")
			await _wait_frames(5)
			var after: String = str(btn.get("text"))
			_assert(before != after, "ModeToggleButton text should change after monkey run")
			# もとに戻す
			btn.emit_signal("pressed")
			await _wait_frames(3)

# ─── メイン ────────────────────────────────────────────────
func _run() -> void:
	# 起動
	print("[KARAKURI_MONKEY] booting shell...")
	change_scene_to_file(SHELL)
	await _wait_frames(3)

	var capture_driver := current_scene.get_node_or_null("UiCaptureDriver")
	if capture_driver:
		capture_driver.queue_free()
		await _wait_frames(1)

	var dui := _dialogue_ui()
	if dui:
		dui.set("typing_speed", 0.0)

	# プロローグをスキップして調査シーンへ
	await _skip_prologue()

	# WarehouseBase に到達できたか確認
	var sc := _scene_container()
	var on_warehouse := (sc != null and sc.get_child_count() > 0
						 and sc.get_child(0).name == "WarehouseBase")
	if not on_warehouse:
		print("[KARAKURI_MONKEY] WARNING: could not reach WarehouseBase; running monkey on current scene anyway")
	else:
		print("[KARAKURI_MONKEY] reached WarehouseBase — starting monkey actions")

	# ── モンキーアクション ────────────────────────────────────
	var interval_min := 1   # アクション後の最小待機フレーム
	var interval_max := 4   # アクション後の最大待機フレーム

	for i in range(MONKEY_ACTIONS):
		_dispatch_random_action()
		var wait_frames := _rng.randi_range(interval_min, interval_max)
		await _wait_frames(wait_frames)
		_action_count += 1

		if i % 50 == 0:
			print("[KARAKURI_MONKEY] progress %d/%d  (random:%d hotspot:%d toggle:%d inv_open:%d inv_close:%d skip:%d)" % [
				i, MONKEY_ACTIONS,
				_stats["click_random"],
				_stats["click_hotspot"],
				_stats["mode_toggle"],
				_stats["inventory_open"],
				_stats["inventory_close"],
				_stats["dialogue_skip"],
			])

	print("[KARAKURI_MONKEY] monkey phase done. total actions=%d" % _action_count)
	print("[KARAKURI_MONKEY] stats: ", _stats)

	# ── 不変条件チェック ──────────────────────────────────────
	await _check_invariants_after_monkey()

	# ── 結果 ─────────────────────────────────────────────────
	if _failed:
		print("[KARAKURI_MONKEY] FAILED")
		quit(1)
	else:
		print("[KARAKURI_MONKEY] passed  (seed=%d)" % MONKEY_SEED)
		quit(0)
