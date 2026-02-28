##
## karakuri_smoke.gd – lightweight smoke test
##
## 目的: Godot 起動 → Mystery シェルが正常にロードされることだけを確認する
## 対象: CI /プッシュごとの最速チェック（想定 ~5-10 秒）
##
## 含む検証:
##   1. シェルシーン (karakuri_mystery_shell.tscn) のロード
##   2. 必須ノードの存在確認
##         - ScenarioRunner
##         - InteractionManager
##         - MainInfoUiLayer/DialogueUI
##         - InstantSubInfoUiLayer/InventoryUI
##         - SystemUiLayer (HealthLabel 等)
##   3. プロローグダイアログが表示されること
##   4. EN/JA ロケール切替でテキストが変わること
##
## 含まない（→ karakuri_scenario_smoke.gd を使うこと）:
##   - シーン遷移・調査・推理・対決・エンディング等のゲームフロー全体
##
extends SceneTree

const SHELL := "res://samples/mystery/karakuri_mystery_shell.tscn"
const PROLOGUE_KEY := "mystery.prologue.system"
const DIALOGUE_PLACEHOLDER := "ここに会話文が表示されます"

var _failed := false

func _initialize() -> void:
	call_deferred("_run")

func _assert(cond: bool, msg: String) -> void:
	if not cond:
		_failed = true
		push_error("[KARAKURI_SMOKE] FAIL: " + msg)

func _wait_frames(n: int) -> void:
	for _i in range(n):
		await process_frame

# ─── ノード取得ヘルパー ─────────────────────────────────────
func _dialogue_ui() -> Node:
	return current_scene.get_node_or_null("MainInfoUiLayer/DialogueUI")

func _inventory_ui() -> Node:
	return current_scene.get_node_or_null("InstantSubInfoUiLayer/InventoryUI")

func _set_locale(prefix: String) -> void:
	var service := get_root().get_node_or_null("KarakuriLocalization")
	if service and service.has_method("set_locale_prefix"):
		service.call("set_locale_prefix", prefix)
	else:
		TranslationServer.set_locale(prefix)

# ─── メイン ────────────────────────────────────────────────
func _run() -> void:
	# 1. シェルシーンをロード
	print("[KARAKURI_SMOKE] booting shell...")
	change_scene_to_file(SHELL)
	await _wait_frames(4)

	# UiCaptureDriver はヘッドレスでは不要なので除去
	var capture_driver := current_scene.get_node_or_null("UiCaptureDriver")
	if capture_driver:
		capture_driver.queue_free()
		await _wait_frames(1)

	# タイピング速度を 0 にしてダイアログをすぐ完了させる
	var dui := _dialogue_ui()
	if dui:
		dui.set("typing_speed", 0.0)

	# ─────────────────────────────────────────
	# 2. 必須ノード存在確認
	# ─────────────────────────────────────────
	print("[KARAKURI_SMOKE] checking required nodes...")

	_assert(current_scene.get_node_or_null("ScenarioRunner") != null,
		"ScenarioRunner is missing from shell scene")

	_assert(current_scene.get_node_or_null("InteractionManager") != null,
		"InteractionManager is missing from shell scene")

	_assert(dui != null,
		"DialogueUI is missing at MainInfoUiLayer/DialogueUI")

	_assert(_inventory_ui() != null,
		"InventoryUI is missing at InstantSubInfoUiLayer/InventoryUI")

	_assert(current_scene.get_node_or_null("SystemUiLayer") != null,
		"SystemUiLayer is missing from shell scene")

	# ─────────────────────────────────────────
	# 3. プロローグダイアログが表示されること
	# ─────────────────────────────────────────
	print("[KARAKURI_SMOKE] waiting for prologue dialogue key...")
	var found_prologue := false
	for _i in range(600):
		if dui != null:
			var key := str(dui.get("_message_text_key")).strip_edges()
			if key == PROLOGUE_KEY:
				found_prologue = true
				break
		await process_frame

	_assert(found_prologue, "prologue dialogue key '%s' was not rendered within timeout" % PROLOGUE_KEY)
	if found_prologue:
		print("[KARAKURI_SMOKE] prologue key observed: ", PROLOGUE_KEY)

	# ─────────────────────────────────────────
	# 4. ロケール切替確認
	# ─────────────────────────────────────────
	print("[KARAKURI_SMOKE] checking locale switch...")
	_set_locale("en")
	await _wait_frames(5)
	var en_title: String = str(TranslationServer.translate("office_title"))

	_set_locale("ja")
	await _wait_frames(5)
	var ja_title: String = str(TranslationServer.translate("office_title"))

	_assert(en_title != ja_title,
		"office_title translation does not differ between EN and JA (en='%s', ja='%s')" % [en_title, ja_title])
	if en_title != ja_title:
		print("[KARAKURI_SMOKE] locale switch OK: en='%s' ja='%s'" % [en_title, ja_title])

	# ─────────────────────────────────────────
	# 結果
	# ─────────────────────────────────────────
	if _failed:
		print("[KARAKURI_SMOKE] FAILED")
		quit(1)
	else:
		print("[KARAKURI_SMOKE] passed")
		quit(0)
