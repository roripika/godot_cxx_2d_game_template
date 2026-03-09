## invader_enemy_view.gd
## InvaderEnemy ノードにアタッチするビュースクリプト。
##
## 役割:
##   - _ready() 内でコードからグループ登録・シグナル接続を完結させる。
##   - GUI 手作業（エディタ上のグループ追加・シグナル接続）は一切不要。
extends InvaderEnemy

func _ready() -> void:
	# ────────────────────────────────────────────
	# 1. グループ登録 — コードで保証するので tscn の groups 設定は不要
	# ────────────────────────────────────────────
	add_to_group("invader_enemies")

	# ────────────────────────────────────────────
	# 2. area_entered → C++ on_area_entered 接続
	#    tscn の [connection] が既に繋いでいる場合は二重接続を防ぐ
	# ────────────────────────────────────────────
	if not area_entered.is_connected(on_area_entered):
		area_entered.connect(on_area_entered)
