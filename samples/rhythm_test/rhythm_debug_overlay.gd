extends Control
## rhythm_test 専用デバッグオブザーバーオーバーレイ
##
## WorldState の SCOPE_SESSION から rhythm_test の全キーを読み取り、
## judge 状態パネル + note 進行バー + clock 表示を毎フレーム更新する。
## ゲームロジック・Tasks・YAML・src/core には一切変更を加えない。

const NS := "rhythm_test"
const S  := 1  # WorldState.SCOPE_SESSION

@onready var info_label: Label  = $InfoLabel
@onready var note_label: Label  = $NoteLabel
@onready var judge_label: Label = $JudgeLabel

var ws: Node = null

func _ready() -> void:
	# 親チェーン: DebugOverlay → CanvasLayer → RhythmDebugScene → WorldState
	ws = get_parent().get_parent().get_node_or_null("WorldState")
	if ws == null:
		push_error("[RhythmDebugOverlay] WorldState ノードが見つかりません。シーンに WorldState を配置してください。")

func _process(_delta: float) -> void:
	if ws == null:
		return
	_refresh()

func _refresh() -> void:
	# ── WorldState キー読み取り ──────────────────────────────────────────
	var clock_ms:    int    = int(ws.get_state(NS, S, "clock:now_ms",         0))
	var chart_idx:  int    = int(ws.get_state(NS, S, "chart:index",          0))
	var note_count: int    = int(ws.get_state(NS, S, "chart:note_count",     0))
	var last_judge: String = str(ws.get_state(NS, S, "judge:last_result",    "none"))
	var perfect:    int    = int(ws.get_state(NS, S, "judge:perfect_count",  0))
	var good:       int    = int(ws.get_state(NS, S, "judge:good_count",     0))
	var miss:       int    = int(ws.get_state(NS, S, "judge:miss_count",     0))
	var tap_ms:     int    = int(ws.get_state(NS, S, "tap:last_time_ms",     -1))
	var status:     String = str(ws.get_state(NS, S, "round:status",         ""))
	var result:     String = str(ws.get_state(NS, S, "round:result",         ""))

	# ── 設定キー ──────────────────────────────────────────────────────────
	var adv_ms:     int    = int(ws.get_state(NS, S, "config:advance_ms",        1000))
	var pfct_win:   int    = int(ws.get_state(NS, S, "config:perfect_window_ms",  50))
	var good_win:   int    = int(ws.get_state(NS, S, "config:good_window_ms",    150))
	var clr_hits:   int    = int(ws.get_state(NS, S, "config:clear_hit_count",     0))
	var max_miss:   int    = int(ws.get_state(NS, S, "config:max_miss_count",      1))

	# ── note 進行バー ──────────────────────────────────────────────────────
	# 例: [P][G][.][.][.] のような ASCII バー。
	# 各マスは perfect=P / good=G / miss=X / 未到達=. で表現する
	var bar := _build_note_bar(chart_idx, note_count, perfect, good, miss)

	# ── 最新判定ハイライト ─────────────────────────────────────────────────
	var judge_color := _judge_color(last_judge)
	judge_label.text = "last judge: %s" % last_judge
	judge_label.add_theme_color_override("font_color", judge_color)

	# ── note 進行パネル ────────────────────────────────────────────────────
	note_label.text = (
		"notes: [%s]\n" % bar
		+ "index / total  : %d / %d\n" % [chart_idx, note_count]
		+ "tap:last_ms    : %d\n"       % tap_ms
		+ "clock:now_ms   : %d\n"       % clock_ms
	)

	# ── メイン情報パネル ───────────────────────────────────────────────────
	var result_line := result if result != "" else "(pending)"
	info_label.text = (
		"=== RHYTHM DEBUG ===\n"
		+ "round:status      = %s\n"   % status
		+ "round:result      = %s\n"   % result_line
		+ "judge:last_result = %s\n"   % last_judge
		+ "judge:perfect     = %d\n"   % perfect
		+ "judge:good        = %d\n"   % good
		+ "judge:miss        = %d\n"   % miss
		+ "---\n"
		+ "chart:index       = %d\n"   % chart_idx
		+ "chart:note_count  = %d\n"   % note_count
		+ "clock:now_ms      = %d\n"   % clock_ms
		+ "tap:last_time_ms  = %d\n"   % tap_ms
		+ "---\n"
		+ "config:advance_ms       = %d\n"   % adv_ms
		+ "config:perfect_window   = %d\n"   % pfct_win
		+ "config:good_window      = %d\n"   % good_win
		+ "config:clear_hit_count  = %d\n"   % clr_hits
		+ "config:max_miss_count   = %d\n"   % max_miss
	)

func _build_note_bar(idx: int, total: int, perf: int, good: int, miss: int) -> String:
	## note 進行内容を ASCII バーで表現する。
	## 到達済みノートを判定種別で色分け表示する (凡例: P=perfect G=good X=miss .=未到達)
	if total <= 0:
		return "---"
	var bar := ""
	for i in range(total):
		if i < idx:
			# 到達済み。判定カウントから種別を推定する（凡例は概算）
			var hit_total := perf + good
			if i < perf:
				bar += "P"
			elif i < hit_total:
				bar += "G"
			else:
				bar += "X"
		elif i == idx:
			bar += ">"  # 現在位置
		else:
			bar += "."
	return bar

func _judge_color(jr: String) -> Color:
	match jr:
		"perfect": return Color(0.2, 1.0, 0.4)   # 緑
		"good":    return Color(0.2, 0.8, 1.0)   # 水色
		"miss":    return Color(1.0, 0.3, 0.3)   # 赤
		_:         return Color(0.8, 0.8, 0.8)   # グレー
