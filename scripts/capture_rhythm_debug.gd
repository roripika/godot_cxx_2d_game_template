## capture_rhythm_debug.gd
##
## rhythm_test デバッグシーンのスクリーンショットを自動保存する SceneTree スクリプト。
##
## 使い方:
##   godot --path <project_path> --script scripts/capture_rhythm_debug.gd -- clear
##   godot --path <project_path> --script scripts/capture_rhythm_debug.gd -- fail
##   godot --path <project_path> --script scripts/capture_rhythm_debug.gd -- continue
##
## `--` 以降のユーザー引数第1要素をシーンキーとして使用する。
## 出力: docs/images/screenshots/rhythm_<key>.png
extends SceneTree

const SCENES: Dictionary = {
	"clear": "res://samples/rhythm_test/rhythm_debug_clear.tscn",
	"fail": "res://samples/rhythm_test/rhythm_debug_fail.tscn",
	"continue": "res://samples/rhythm_test/rhythm_debug_continue.tscn",
}

const OUT_DIR: String = "res://docs/images/screenshots/"

## シナリオが全タスク完了するまでの実時間待機秒数。
## clear は clock=3000ms のシミュレーション後に終わるが、
## フレーム処理なので実時間 3 秒あれば十分。
const WAIT_SEC: float = 4.0


func _init() -> void:
	var user_args := OS.get_cmdline_user_args()
	var key: String = user_args[0] if user_args.size() > 0 else "clear"

	if not SCENES.has(key):
		printerr("ERROR: unknown scene key '%s'. Use: clear | fail | continue" % key)
		quit(1)
		return

	print("[capture_rhythm_debug] Loading scene: ", key)

	var root := get_root()
	var packed: PackedScene = load(SCENES[key])
	var scene: Node = packed.instantiate()
	root.add_child(scene)

	# ScenarioRunner は _ready() で自動開始し _process() でステップを進める。
	# ウィンドウが立ち上がりレンダリングされるまで deferred で待機してからキャプチャ。
	call_deferred("_capture", root, key)


func _capture(root: Node, key: String) -> void:
	print("[capture_rhythm_debug] Waiting %.1f sec for scenario to complete..." % WAIT_SEC)
	await root.get_tree().create_timer(WAIT_SEC).timeout

	# WorldState 実測値を stdout に出力する
	_print_world_state(root)

	var img := root.get_viewport().get_texture().get_image()

	# res:// パスを実ファイルシステムパスに変換して保存。
	var out_path := ProjectSettings.globalize_path(OUT_DIR + "rhythm_" + key + ".png")
	DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(OUT_DIR))

	var err := img.save_png(out_path)
	if err == OK:
		print("[capture_rhythm_debug] Saved: ", out_path)
	else:
		printerr("[capture_rhythm_debug] FAILED to save: ", out_path, " (err=", err, ")")

	quit()


func _print_world_state(root: Node) -> void:
	# RhythmDebugScene の直下にある WorldState ノードを取得
	var debug_scene: Node = root.get_child(0) if root.get_child_count() > 0 else null
	if debug_scene == null:
		print("[WorldState] (scene not found)")
		return
	var ws: Node = debug_scene.get_node_or_null("WorldState")
	if ws == null:
		print("[WorldState] (WorldState node not found)")
		return

	const NS := "rhythm_test"
	const S  := 1  # SCOPE_SESSION
	print("--- WorldState (rhythm_test) ---")
	print("  round:status            = ", ws.get_state(NS, S, "round:status", ""))
	print("  round:result            = ", ws.get_state(NS, S, "round:result", ""))
	print("  judge:last_result       = ", ws.get_state(NS, S, "judge:last_result", ""))
	print("  judge:perfect_count     = ", ws.get_state(NS, S, "judge:perfect_count", 0))
	print("  judge:good_count        = ", ws.get_state(NS, S, "judge:good_count", 0))
	print("  judge:miss_count        = ", ws.get_state(NS, S, "judge:miss_count", 0))
	print("  chart:index             = ", ws.get_state(NS, S, "chart:index", 0))
	print("  chart:note_count        = ", ws.get_state(NS, S, "chart:note_count", 0))
	print("  clock:now_ms            = ", ws.get_state(NS, S, "clock:now_ms", 0))
	print("  tap:last_time_ms        = ", ws.get_state(NS, S, "tap:last_time_ms", -1))
	print("  config:clear_hit_count  = ", ws.get_state(NS, S, "config:clear_hit_count", 0))
	print("  config:max_miss_count   = ", ws.get_state(NS, S, "config:max_miss_count", 0))
	print("--------------------------------")
