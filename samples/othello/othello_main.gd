extends Control

# Othello UI implementation using Karakuri OthelloManager (C++)

var manager: OthelloManager
var cells: Array = []
var turn_label: Label
var score_label: Label
var status_overlay: ColorRect
var result_label: Label

func _ready() -> void:
	# 1. OthelloManager (C++) の初期化
	manager = OthelloManager.new()
	add_child(manager)
	
	# 2. UI要素の構築
	_setup_ui()
	
	# 3. シグナル結線
	manager.board_updated.connect(_update_board_view)
	manager.turn_changed.connect(_on_turn_changed)
	manager.game_over.connect(_on_game_over)
	
	# 4. ゲーム開始
	manager.start_game()

func _setup_ui() -> void:
	# 画面全体を覆うベース（背景クリック用などの想定）
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	
	# 画面中央に配置するための CenterContainer
	var center = CenterContainer.new()
	center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(center)
	
	# 縦に並べるための VBoxContainer
	var vbox = VBoxContainer.new()
	vbox.add_theme_constant_override("separation", 20)
	center.add_child(vbox)
	
	# 情報表示用ラベル（上部）
	turn_label = Label.new()
	turn_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	turn_label.add_theme_font_size_override("font_size", 24)
	turn_label.text = "ターン: 黒"
	vbox.add_child(turn_label)
	
	# 盤面の土台 (ColorRect)
	var board_panel = ColorRect.new()
	board_panel.color = Color(0.1, 0.1, 0.1) # ダークグレー
	board_panel.custom_minimum_size = Vector2(440, 440)
	vbox.add_child(board_panel)
	
	# 盤面中央に寄せるための CenterContainer (ボードパネル内)
	var board_center = CenterContainer.new()
	board_center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	board_panel.add_child(board_center)
	
	# グリッドレイアウト
	var grid = GridContainer.new()
	grid.columns = 8
	grid.add_theme_constant_override("h_separation", 4)
	grid.add_theme_constant_override("v_separation", 4)
	board_center.add_child(grid)
	
	# セルの生成 (8x8)
	for y in 8:
		for x in 8:
			var cell = Button.new()
			cell.custom_minimum_size = Vector2(50, 50)
			
			# セルの背景色を設定
			var style = StyleBoxFlat.new()
			style.bg_color = Color(0, 0.5, 0) # 緑
			style.border_width_left = 1
			style.border_width_right = 1
			style.border_width_top = 1
			style.border_width_bottom = 1
			style.border_color = Color(0, 0.4, 0)
			cell.add_theme_stylebox_override("normal", style)
			cell.add_theme_stylebox_override("hover", style)
			cell.add_theme_stylebox_override("pressed", style)
			
			# 石の表示用
			var stone_container = CenterContainer.new()
			stone_container.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
			stone_container.mouse_filter = Control.MOUSE_FILTER_IGNORE
			cell.add_child(stone_container)
			
			var stone = ColorRect.new()
			stone.name = "Stone"
			stone.custom_minimum_size = Vector2(40, 40)
			stone.mouse_filter = Control.MOUSE_FILTER_IGNORE
			stone.visible = false
			stone_container.add_child(stone)
			
			cell.pressed.connect(_on_cell_clicked.bind(x, y))
			grid.add_child(cell)
			cells.append(cell)
	
	# スコアラベル（下部）
	score_label = Label.new()
	score_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	score_label.add_theme_font_size_override("font_size", 20)
	score_label.text = "黒: 2  白: 2"
	vbox.add_child(score_label)
	
	# リザルトオーバーレイ
	status_overlay = ColorRect.new()
	status_overlay.color = Color(0, 0, 0, 0.7)
	status_overlay.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	status_overlay.visible = false
	add_child(status_overlay)
	
	var res_vbox = VBoxContainer.new()
	res_vbox.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
	res_vbox.add_theme_constant_override("separation", 20)
	status_overlay.add_child(res_vbox)
	
	result_label = Label.new()
	result_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	result_label.add_theme_font_size_override("font_size", 32)
	res_vbox.add_child(result_label)
	
	var restart_btn = Button.new()
	restart_btn.text = "再挑戦"
	restart_btn.custom_minimum_size = Vector2(160, 50)
	restart_btn.pressed.connect(func(): 
		status_overlay.visible = false
		manager.start_game()
	)
	res_vbox.add_child(restart_btn)

func _update_board_view() -> void:
	for y in 8:
		for x in 8:
			var piece = manager.get_piece(x, y)
			var cell = cells[y * 8 + x]
			var stone = cell.find_child("Stone", true, false)
			
			if piece == 0:
				stone.visible = false
			else:
				stone.visible = true
				stone.color = Color.BLACK if piece == 1 else Color.WHITE
				# 角を丸くして円に見せる (StyleBoxは使わず単純に)
				# 実際にはここでアニメーションさせるとより良い

	score_label.text = "黒: %d  白: %d" % [manager.get_score(1), manager.get_score(2)]

func _on_cell_clicked(x: int, y: int) -> void:
	var success = manager.place_piece(x, y)
	if not success:
		# 手が打てない場所がクリックされた時のフィードバック（オプション）
		print("そこには置けません: ", x, ", ", y)

func _on_turn_changed(new_turn: int) -> void:
	turn_label.text = "ターン: %s" % ("黒" if new_turn == 1 else "白")

func _on_game_over(black_score: int, white_score: int) -> void:
	status_overlay.visible = true
	var winner = ""
	if black_score > white_score: winner = "黒の勝ち！"
	elif white_score > black_score: winner = "白の勝ち！"
	else: winner = "引き分け！"
	
	result_label.text = "ゲーム終了\n%s\n黒: %d - 白: %d" % [winner, black_score, white_score]
