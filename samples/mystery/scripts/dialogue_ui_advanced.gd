extends Control
class_name DialogueUIAdvanced

signal choice_selected(index: int, text: String)
signal dialogue_finished()
signal portrait_shown(speaker_id: String)
signal portrait_hidden()

@onready var ui_part_label: Label = get_node_or_null("UiPartLabel")
@onready var name_label: Label = get_node_or_null("VBoxContainer/NameLabel")
@onready var text_label: Label = get_node_or_null("VBoxContainer/TextLabel")
@onready var main_ui_layer: CanvasLayer = get_node_or_null("..")
@onready var screen_flash: ColorRect = get_node_or_null("../ScreenFlash")
@onready var portrait_rect: TextureRect = get_node_or_null("../PortraitContainer/PortraitRect")
@onready var proceed_indicator: Polygon2D = get_node_or_null("ProceedIndicator")

# Use a lazy getter for ChoicesContainer to avoid null before @onready
var _choices_container_cache: VBoxContainer = null
func _get_choices_container() -> VBoxContainer:
	if _choices_container_cache and is_instance_valid(_choices_container_cache):
		return _choices_container_cache
	_choices_container_cache = get_node_or_null("VBoxContainer/ChoicesContainer")
	if _choices_container_cache == null:
		_choices_container_cache = find_child("ChoicesContainer", true, false)
	return _choices_container_cache

@export var typing_speed: float = 0.05
@export var text_color: Color = Color.WHITE
@export var name_color: Color = Color.YELLOW

const PORTRAIT_FADE_DURATION := 0.2

var is_typing: bool = false
var current_text: String = ""

var _message_speaker_key: String = ""
var _message_speaker_text: String = ""
var _message_text_key: String = ""
var _message_text: String = ""
var _choice_defs: Array[Dictionary] = []
var _mode_input_enabled: bool = true
var _waiting_for_click: bool = false
var _portrait_side: String = "left"
var _portrait_side_override: String = "auto"
var _portrait_enter_transition: String = "none"
var _portrait_exit_transition: String = "none"
var _portrait_tween: Tween = null

func _ready() -> void:
	visible = false
	_clear_choices()
	_connect_localization_service()
	resized.connect(_on_dialogue_resized)
	_refresh_locale()
	
	var cc = _get_choices_container()
	if cc == null:
		print("[DialogueUI] ERROR: Could not find ChoicesContainer!")

func show_message(speaker: String, text: String) -> void:
	show_message_with_keys("", speaker, "", text)

func show_message_with_keys(speaker_key: String, speaker: String, text_key: String, text: String, portrait_side: String = "auto") -> void:
	set_portrait_side(portrait_side)
	visible = true
	_message_speaker_key = speaker_key
	_message_speaker_text = speaker
	_message_text_key = text_key
	_message_text = text
	_choice_defs.clear()
	_clear_choices()
	await _update_portrait_for_dialogue(speaker_key if speaker_key != "" else speaker)
	_apply_message_name()
	_type_text(_resolve_text(_message_text_key, _message_text))

func show_choices(choices: Array) -> int:
	var defs: Array[Dictionary] = []
	for choice in choices:
		defs.append({"text": str(choice), "text_key": ""})
	return await show_choices_with_defs(defs)

func show_choices_with_defs(choice_defs: Array) -> int:
	visible = true
	_choice_defs = []
	for i in range(choice_defs.size()):
		var src = choice_defs[i]
		if src == null:
			continue
		var d: Dictionary = src
		_choice_defs.append({
			"text": str(d.get("text", "")),
			"text_key": str(d.get("text_key", "")),
		})

	_rebuild_choices()
	var args = await choice_selected
	return int(args[0])

func hide_dialogue() -> void:
	visible = false
	_choice_defs.clear()
	_clear_choices()
	clear_portrait()

func set_portrait(texture: Texture2D) -> void:
	if portrait_rect:
		portrait_rect.texture = texture
		portrait_rect.visible = (texture != null)
		portrait_rect.modulate.a = 1.0
		if portrait_rect.visible:
			_apply_portrait_layout()

func clear_portrait() -> void:
	if portrait_rect:
		var was_visible = portrait_rect.visible  
		portrait_rect.texture = null
		portrait_rect.visible = false
		portrait_rect.modulate.a = 1.0
		if was_visible:
			portrait_hidden.emit()

func _update_portrait(speaker_name: String) -> void:
	var portrait_id = _resolve_portrait_id(speaker_name)
	
	if portrait_id != "":
		var path = "res://assets/mystery/characters/portraits/%s.png" % portrait_id
		if ResourceLoader.exists(path):
			set_portrait(load(path))
			_update_portrait_side(portrait_id)
			_apply_portrait_layout()
			portrait_shown.emit(portrait_id)
		else:
			clear_portrait()
	else:
		clear_portrait()

func _resolve_portrait_id(speaker_name: String) -> String:
	match speaker_name:
		"Detective", "探偵", "speaker.detective", "Ren", "speaker.ren":
			return "detective"
		"Boss", "所長", "speaker.boss", "Ken", "speaker.ken":
			return "boss"
		"Rat Witness", "ネズミの証人", "容疑者", "speaker.rat_witness":
			return "rat_witness"
		"Assistant", "助手", "speaker.yui", "Yui":
			return "yui"
		"Manager", "管理人", "speaker.tanaka", "Tanaka":
			return "tanaka"
		"Worker", "作業員", "speaker.sato", "Sato":
			return "sato"
		"Delivery", "配送人", "speaker.suzuki", "Suzuki":
			return "suzuki"
		"Kenta", "ケンタ", "speaker.kenta":
			return "kenta"
		_:
			return ""
func _update_portrait_for_dialogue(speaker_name: String) -> void:
	if not portrait_rect:
		return

	var portrait_id := _resolve_portrait_id(speaker_name)
	var had_visible_portrait := portrait_rect.visible and portrait_rect.texture != null

	if _portrait_exit_transition != "none" and had_visible_portrait:
		match _portrait_exit_transition:
			"fade_out":
				await _fade_portrait_to(0.0, PORTRAIT_FADE_DURATION)
			"slide_out_left":
				var tw = portrait_rect.create_tween().set_parallel(true)
				tw.tween_property(portrait_rect, "position:x", portrait_rect.position.x - 100, 0.3).set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_IN)
				tw.tween_property(portrait_rect, "modulate:a", 0.0, 0.2)
				await tw.finished
			"slide_out_right":
				var tw = portrait_rect.create_tween().set_parallel(true)
				tw.tween_property(portrait_rect, "position:x", portrait_rect.position.x + 100, 0.3).set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_IN)
				tw.tween_property(portrait_rect, "modulate:a", 0.0, 0.2)
				await tw.finished

	if portrait_id == "":
		clear_portrait()
		_reset_portrait_transitions()
		return

	var path = "res://assets/mystery/characters/portraits/%s.png" % portrait_id
	if not FileAccess.file_exists(path):
		clear_portrait()
		_reset_portrait_transitions()
		return

	var tex = load(path)
	if tex == null:
		var gpath = ProjectSettings.globalize_path(path)
		var img = Image.load_from_file(gpath)
		if img:
			tex = ImageTexture.create_from_image(img)

	set_portrait(tex)
	_update_portrait_side(portrait_id)
	_apply_portrait_layout()
	portrait_shown.emit(portrait_id)

	# 入場アニメーションの実行
	match _portrait_enter_transition:
		"fade_in":
			portrait_rect.modulate.a = 0.0
			await _fade_portrait_to(1.0, PORTRAIT_FADE_DURATION)
		"slide_in_left":
			var final_pos = portrait_rect.position
			portrait_rect.position.x -= 100
			portrait_rect.modulate.a = 0.0
			var tw = portrait_rect.create_tween().set_parallel(true)
			tw.tween_property(portrait_rect, "position:x", final_pos.x, 0.3).set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_OUT)
			tw.tween_property(portrait_rect, "modulate:a", 1.0, 0.2)
			await tw.finished
		"slide_in_right":
			var final_pos = portrait_rect.position
			portrait_rect.position.x += 100
			portrait_rect.modulate.a = 0.0
			var tw = portrait_rect.create_tween().set_parallel(true)
			tw.tween_property(portrait_rect, "position:x", final_pos.x, 0.3).set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_OUT)
			tw.tween_property(portrait_rect, "modulate:a", 1.0, 0.2)
			await tw.finished
		"zoom_in":
			portrait_rect.modulate.a = 0.0
			portrait_rect.scale = Vector2(0.8, 0.8)
			# PortraitRectは中心基準ではないため、スケールアニメーションは工夫が必要だが一旦簡易的に
			var tw = portrait_rect.create_tween().set_parallel(true)
			tw.tween_property(portrait_rect, "scale", Vector2(1.0, 1.0), 0.3).set_trans(Tween.TRANS_BACK).set_ease(Tween.EASE_OUT)
			tw.tween_property(portrait_rect, "modulate:a", 1.0, 0.2)
			await tw.finished
		"shake":
			portrait_rect.modulate.a = 1.0
			await _shake_portrait()
		_:
			portrait_rect.modulate.a = 1.0

	_reset_portrait_transitions()

func _fade_portrait_to(alpha: float, duration: float) -> void:
	if not portrait_rect:
		return
	if _portrait_tween and _portrait_tween.is_valid():
		_portrait_tween.kill()
	_portrait_tween = portrait_rect.create_tween()
	_portrait_tween.tween_property(portrait_rect, "modulate:a", alpha, duration).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	await _portrait_tween.finished

func _shake_portrait() -> void:
	if not portrait_rect: return
	var original_pos = portrait_rect.position
	var tw = portrait_rect.create_tween()
	for i in range(5):
		tw.tween_property(portrait_rect, "position:x", original_pos.x + 10, 0.05)
		tw.tween_property(portrait_rect, "position:x", original_pos.x - 10, 0.05)
	tw.tween_property(portrait_rect, "position:x", original_pos.x, 0.05)
	await tw.finished

func _reset_portrait_transitions() -> void:
	_portrait_enter_transition = "none"
	_portrait_exit_transition = "none"

func _update_portrait_side(portrait_id: String) -> void:
	if _portrait_side_override != "auto":
		_portrait_side = _portrait_side_override
		return
	match portrait_id:
		"boss", "rat_witness":
			_portrait_side = "right"
		_:
			_portrait_side = "left"

func _apply_portrait_layout() -> void:
	if not portrait_rect:
		return

	# ダイアログのシフト処理を廃止し、定位置に固定
	offset_left = 0
	offset_right = 0
	
	var screen_size = get_viewport_rect().size
	var screen_w = screen_size.x
	var screen_h = screen_size.y
	if screen_w <= 0: screen_w = 1152.0
	if screen_h <= 0: screen_h = 648.0
	
	# 水平位置: 画面を4等分した境界 (25%, 50%, 75%)
	var target_x_ratio = 0.5
	match _portrait_side:
		"left": target_x_ratio = 0.25   # No0
		"center": target_x_ratio = 0.5   # No1
		"right": target_x_ratio = 0.75  # No2
	
	var center_x = screen_w * target_x_ratio
	
	# 元のテクスチャサイズを取得
	var tex_size = Vector2(300, 400) # デフォルト
	if portrait_rect.texture:
		tex_size = portrait_rect.texture.get_size()

	# スケールはリセット（offsets でサイズを完全制御するため、scale 変換を掛けない）
	portrait_rect.scale = Vector2(1.0, 1.0)

	# 画面高の75%を目標高としてスケーリング係数を算出し、表示サイズを決定
	var target_h_ratio = 0.75
	var dynamic_scale = (screen_h * target_h_ratio) / tex_size.y
	var scaled_w = tex_size.x * dynamic_scale
	var scaled_h = tex_size.y * dynamic_scale

	# アンカーをリセットしてトップレフト基準で計算
	portrait_rect.anchor_left = 0
	portrait_rect.anchor_top = 0
	portrait_rect.anchor_right = 0
	portrait_rect.anchor_bottom = 0

	# 水平: 指定位置を中心に配置
	portrait_rect.offset_left = center_x - scaled_w * 0.5
	portrait_rect.offset_right = portrait_rect.offset_left + scaled_w

	# 垂直: スケーリング後サイズで中心Y を計算
	# charaimg_center_y = (screen_h - scaled_h) + 5 + scaled_h * 0.5
	# → 画像下端が画面下端より 5px 下にはみ出す位置に中心を設定
	var center_y = (screen_h - scaled_h) + 5.0 + scaled_h * 0.5
	portrait_rect.offset_top = center_y - scaled_h * 0.5
	portrait_rect.offset_bottom = center_y + scaled_h * 0.5

func _on_dialogue_resized() -> void:
	if portrait_rect and portrait_rect.visible:
		_apply_portrait_layout()

func set_portrait_side(side: String) -> void:
	var normalized := side.strip_edges().to_lower()
	if normalized == "":
		normalized = "auto"
	match normalized:
		"left", "right", "center", "auto":
			_portrait_side_override = normalized
		_:
			_portrait_side_override = "auto"
	if portrait_rect and portrait_rect.visible:
		_apply_portrait_layout()

func set_portrait_enter(mode: String) -> void:
	var normalized := mode.strip_edges().to_lower()
	match normalized:
		"fade", "fade_in", "in": _portrait_enter_transition = "fade_in"
		"slide_left", "slide_in_left": _portrait_enter_transition = "slide_in_left"
		"slide_right", "slide_in_right": _portrait_enter_transition = "slide_in_right"
		"zoom", "zoom_in": _portrait_enter_transition = "zoom_in"
		"shake": _portrait_enter_transition = "shake"
		_: _portrait_enter_transition = "none"

func set_portrait_exit(mode: String) -> void:
	var normalized := mode.strip_edges().to_lower()
	match normalized:
		"fade", "fade_out", "out": _portrait_exit_transition = "fade_out"
		"slide_left", "slide_out_left": _portrait_exit_transition = "slide_out_left"
		"slide_right", "slide_out_right": _portrait_exit_transition = "slide_out_right"
		_: _portrait_exit_transition = "none"

func skip_typing() -> void:
	if not is_typing:
		return
	if text_label:
		text_label.text = current_text
	is_typing = false
	_waiting_for_click = true

func _type_text(text: String) -> void:
	if is_typing:
		return
	print("[DialogueUI] Starting typing: ", text)
	is_typing = true
	current_text = text
	if text_label:
		text_label.modulate = text_color
		text_label.text = ""

	if typing_speed <= 0.0:
		if text_label:
			text_label.text = text
		is_typing = false
		_waiting_for_click = true
		print("[DialogueUI] Typing finished instantly (speed=0)")
		return

	for i in range(len(text)):
		if not is_typing:
			print("[DialogueUI] Typing interrupted")
			return
		if text_label:
			text_label.text += str(text[i])
		await get_tree().create_timer(typing_speed).timeout

	if is_typing:
		is_typing = false
		_waiting_for_click = true
		print("[DialogueUI] Typing finished naturally")

func _on_choice_pressed(index: int, text: String) -> void:
	print("[DialogueUI] _on_choice_pressed called with index %d text %s mode %s" % [index, text, _mode_input_enabled])
	if not _mode_input_enabled:
		return
	_clear_choices()
	_choice_defs.clear()
	choice_selected.emit(index, text)

func _rebuild_choices() -> void:
	var cc = _get_choices_container()
	print("[DialogueUI] Rebuilding choices. Container exists: ", cc != null)
	_clear_choices()
	for i in range(_choice_defs.size()):
		var def: Dictionary = _choice_defs[i]
		var choice_btn := Button.new()
		var choice_text := _resolve_text(str(def.get("text_key", "")), str(def.get("text", "")))
		choice_btn.text = "▶ " + choice_text
		choice_btn.disabled = not _mode_input_enabled
		choice_btn.alignment = HORIZONTAL_ALIGNMENT_LEFT
		choice_btn.custom_minimum_size = Vector2(0, 40)
		choice_btn.add_theme_font_size_override("font_size", 18)
		choice_btn.pressed.connect(_on_choice_pressed.bind(i, choice_text))
		if cc:
			cc.add_child(choice_btn)
			print("[DialogueUI] Added choice button: ", choice_text)
		else:
			print("[DialogueUI] ERROR: Cannot add choice button, container is NULL")

func _apply_message_name() -> void:
	if not name_label:
		return
	name_label.modulate = name_color
	name_label.text = _resolve_text(_message_speaker_key, _message_speaker_text)

func _resolve_text(key: String, fallback: String) -> String:
	if key.is_empty():
		return fallback
	var localized := tr(key)
	if localized != key:
		return localized
	# If tr(key) returned key, it's likely not translated OR the translation is the key itself.
	# We fallback to fallback if available.
	if fallback != "":
		return fallback
	return key

func _refresh_locale() -> void:
	if ui_part_label:
		ui_part_label.text = tr("mystery.ui.dialogue_panel")
	if _message_speaker_key != "" or _message_speaker_text != "":
		_apply_message_name()
	if _message_text_key != "":
		current_text = _resolve_text(_message_text_key, _message_text)
		if is_typing:
			skip_typing()
		if text_label:
			text_label.text = current_text
	if _choice_defs.size() > 0:
		_rebuild_choices()

func on_mode_enter(_mode_id: String, _scene_id: String) -> void:
	_refresh_locale()

func on_mode_exit(_mode_id: String, _next_scene_id: String) -> void:
	_clear_choices()

func set_mode_input_enabled(enabled: bool) -> void:
	_mode_input_enabled = enabled
	# フルリビルドを避けて既存ボタンのdisabled状態だけ更新する
	var cc = _get_choices_container()
	if cc:
		for child in cc.get_children():
			if child is Button:
				child.disabled = not enabled

func flash_screen(color_hex: String, duration: float = 0.5) -> void:
	if not screen_flash:
		return
	var color = Color.from_string(color_hex, Color.WHITE)
	screen_flash.visible = true
	screen_flash.color = color
	screen_flash.color.a = 0.5
	var tw = screen_flash.create_tween()
	tw.tween_property(screen_flash, "color:a", 0.0, duration)
	await tw.finished
	screen_flash.visible = false

func shake_screen(intensity: float = 10.0, duration: float = 0.5) -> void:
	if not main_ui_layer:
		return
	var original_offset = main_ui_layer.offset
	var tw = main_ui_layer.create_tween()
	var elapsed = 0.0
	var step = 0.05
	while elapsed < duration:
		var offset = Vector2(randf_range(-intensity, intensity), randf_range(-intensity, intensity))
		tw.tween_property(main_ui_layer, "offset", original_offset + offset, step)
		await tw.finished
		elapsed += step
	main_ui_layer.offset = original_offset

func _connect_localization_service() -> void:
	var service := get_tree().root.get_node_or_null("KarakuriLocalization")
	if service == null:
		return
	if service.has_signal("locale_changed"):
		var cb := Callable(self, "_on_locale_changed")
		if not service.is_connected("locale_changed", cb):
			service.connect("locale_changed", cb)

func _process(_delta: float) -> void:
	if proceed_indicator:
		if _waiting_for_click and not is_typing:
			proceed_indicator.visible = true
			# 0.5Hz pulse
			var alpha = 0.5 + 0.5 * sin(Time.get_ticks_msec() * 0.008)
			proceed_indicator.modulate.a = alpha
		else:
			proceed_indicator.visible = false

func _on_locale_changed(_locale: String) -> void:
	_refresh_locale()

func _input(event: InputEvent) -> void:
	if not visible:
		return
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		print("[DialogueUI] Mouse click detected. is_typing=", is_typing, " _waiting_for_click=", _waiting_for_click)
		if is_typing:
			print("[DialogueUI] Skipping typing")
			skip_typing()
			get_viewport().set_input_as_handled()
		elif _waiting_for_click:
			print("[DialogueUI] Dialogue finished, emitting signal")
			_waiting_for_click = false
			dialogue_finished.emit()
			get_viewport().set_input_as_handled()
		else:
			print("[DialogueUI] Click ignored (not typing and not waiting)")

func _clear_choices() -> void:
	var cc = _get_choices_container()
	if not cc:
		return
	for child in cc.get_children():
		child.queue_free()
