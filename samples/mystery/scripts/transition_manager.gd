extends Node
class_name KarakuriTransitionManager

## アドベンチャーシーンやUIにおける汎用トランジションを管理するマネージャー。
## ShaderMaterialを使ったワイプやTweenによるスライド/フェードなどを提供します。

# シェーダーリソース群
const SHADER_DIR = "res://src/karakuri/shaders/"
var shaders = {
	"wipe": null,
	"radial": null,
	"split": null,
	"page_turn": null
}
var is_headless_runtime := false

func _ready() -> void:
	is_headless_runtime = DisplayServer.get_name() == "headless"

	# 利用可能なシェーダーを事前ロード
	var paths = {
		"wipe": SHADER_DIR + "transition_wipe.gdshader",
		"radial": SHADER_DIR + "transition_radial.gdshader",
		"split": SHADER_DIR + "transition_split.gdshader",
		"page_turn": SHADER_DIR + "transition_page_turn.gdshader"
	}
	
	for key in paths:
		if ResourceLoader.exists(paths[key]):
			shaders[key] = load(paths[key]) as Shader

## 指定ノードに対してフェード（透明度のAnimation）を行います。
## @param node: 対象の CanvasItem (Control, Node2D 等)
## @param duration: なめらかな遷移にかける秒数
## @param is_in: trueなら出現(明転), falseなら消滅(暗転)
## @return: Tweenオブジェクト。完了を待機したい場合は `await fade(...).finished`
func fade(node: CanvasItem, duration: float, is_in: bool) -> Tween:
	if not node: return null
	var tween = node.create_tween()
	
	# 初期の透明度をセット
	var start_alpha = 0.0 if is_in else 1.0
	var target_alpha = 1.0 if is_in else 0.0
	
	# すでに表示中（または非表示中）の場合は、現状のアルファ値を開始に使うと自然になる
	start_alpha = node.modulate.a
	
	tween.tween_property(node, "modulate:a", target_alpha, duration).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	return tween

## 指定ノードに対してスライドイン/アウトを行います。対象の現在位置を原点（出現完了位置）とみなします。
## @param node: 対象の Control ノード
## @param direction: スライドしてくる（または消えていく）方向のベクトル (例: Vector2(-1, 0) で左)
## @param distance: 移動距離ピクセル
## @param duration: 遷移にかける秒数
## @param is_in: trueなら出現(画面外から元の位置へ), falseなら消滅(元の位置から画面外へ)
func slide(node: Control, direction: Vector2, distance: float, duration: float, is_in: bool) -> Tween:
	if not node: return null
	var tween = node.create_tween()
	
	# 対象のベース位置（元の位置）
	if not node.has_meta("original_pos"):
		node.set_meta("original_pos", node.position)
	var base_pos: Vector2 = node.get_meta("original_pos")
	
	var offset = direction.normalized() * distance
	
	if is_in:
		# 出現の場合：ベース位置からoffsetだけズレた位置が開始点
		node.position = base_pos + offset
		node.modulate.a = 0.0
		tween.parallel().tween_property(node, "position", base_pos, duration).set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_OUT)
		tween.parallel().tween_property(node, "modulate:a", 1.0, duration * 0.8)
	else:
		# 消滅の場合：現在のベース位置からoffsetだけズレた位置へ移動
		node.position = base_pos
		tween.parallel().tween_property(node, "position", base_pos + offset, duration).set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_IN)
		tween.parallel().tween_property(node, "modulate:a", 0.0, duration * 0.8).set_delay(duration * 0.2)
		
	return tween

# --- Cocos2d-x Style Transitions Dispatcher --- #

## エフェクト名を指定して各種トランジションを実行します。
## @param node: 対象ノード
## @param type_name: エフェクト名 (例: "fade", "slide_left", "split_rows", "page_turn")
## @param duration: なめらかな遷移にかける秒数
## @param is_in: trueなら出現, falseなら消滅
func apply_transition(node: CanvasItem, type_name: String, duration: float, is_in: bool) -> Tween:
	if not node: return null
	
	match type_name:
		"fade":
			return fade(node, duration, is_in)

		# Shader: Wipe (Linear/CrossFade/TurnOffTiles)
		"cross_fade":
			return fade(node, duration, is_in) if is_headless_runtime else wipe_tile(node, Vector2(1, 1), Vector2(0, 0), 0.0, duration, is_in)
		"turn_off_tiles":
			return fade(node, duration, is_in) if is_headless_runtime else wipe_tile(node, Vector2(12, 12), Vector2(0, 0), 1.0, duration, is_in)
		"fade_tr":
			return fade(node, duration, is_in) if is_headless_runtime else wipe_tile(node, Vector2(1, 1), Vector2(1, -1), 0.0, duration, is_in)
		"fade_bl":
			return fade(node, duration, is_in) if is_headless_runtime else wipe_tile(node, Vector2(1, 1), Vector2(-1, 1), 0.0, duration, is_in)
		"fade_up":
			return fade(node, duration, is_in) if is_headless_runtime else wipe_tile(node, Vector2(1, 1), Vector2(0, -1), 0.0, duration, is_in)
		"fade_down":
			return fade(node, duration, is_in) if is_headless_runtime else wipe_tile(node, Vector2(1, 1), Vector2(0, 1), 0.0, duration, is_in)

		# Shader: Radial / Linear
		"wipe_radial_cw":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "radial", {"mode": 0}, duration, is_in)
		"wipe_radial_ccw":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "radial", {"mode": 1}, duration, is_in)
		"wipe_linear_h":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "radial", {"mode": 2}, duration, is_in)
		"wipe_linear_v":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "radial", {"mode": 3}, duration, is_in)
		"wipe_center_out":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "radial", {"mode": 4}, duration, is_in)
		"wipe_center_in":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "radial", {"mode": 5}, duration, is_in)

		# Shader: Split
		"split_rows":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "split", {"mode": 0, "splits": 10.0}, duration, is_in)
		"split_cols":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "split", {"mode": 1, "splits": 10.0}, duration, is_in)

		# Shader: Page Turn
		"page_turn":
			return fade(node, duration, is_in) if is_headless_runtime else _apply_shader_transition(node, "page_turn", {"mode": 1}, duration, is_in)
		
		
		# Tween: Move & Slide (requires Control node for size/position manipulation)
		"slide_left": return slide(node as Control, Vector2(-1, 0), 100.0, duration, is_in) if node is Control else fade(node, duration, is_in)
		"slide_right": return slide(node as Control, Vector2(1, 0), 100.0, duration, is_in) if node is Control else fade(node, duration, is_in)
		"slide_up": return slide(node as Control, Vector2(0, -1), 100.0, duration, is_in) if node is Control else fade(node, duration, is_in)
		"slide_down": return slide(node as Control, Vector2(0, 1), 100.0, duration, is_in) if node is Control else fade(node, duration, is_in)
		
		# Tween: Scale, Rotate, Flip (require pivot adjustments)
		"shrink_grow": return _animate_scale_rotate(node as Control, false, false, duration, is_in) if node is Control else fade(node, duration, is_in)
		"roto_zoom": return _animate_scale_rotate(node as Control, true, false, duration, is_in) if node is Control else fade(node, duration, is_in)
		"jump_zoom": return _animate_scale_rotate(node as Control, false, true, duration, is_in) if node is Control else fade(node, duration, is_in)
		"flip_x": return _animate_flip(node as Control, true, false, false, duration, is_in) if node is Control else fade(node, duration, is_in)
		"flip_y": return _animate_flip(node as Control, false, true, false, duration, is_in) if node is Control else fade(node, duration, is_in)
		"zoom_flip": return _animate_flip(node as Control, true, false, true, duration, is_in) if node is Control else fade(node, duration, is_in)
		"flip_angular": return _animate_flip(node as Control, true, true, false, duration, is_in) if node is Control else fade(node, duration, is_in)
		
		_: # デフォルトフォールバック
			push_warning("Transition type '%s' unknown, falling back to fade." % type_name)
			return fade(node, duration, is_in)

## シェーダーを利用したタイル状ワイプ/グラデーションワイプエフェクトを適用します。
## @param node: 対象の CanvasItem
## @param tile_count: タイル分割数。Vector2(1,1)ならプレーンなグラデーション
## @param wipe_dir: ワイプの進行方向 (例: Vector2(1,0) で左から右へ消える/現れる)
## @param scale_effect: 1.0ならタイルごとに小さくなりながら消える。0.0ならアルファのみ
## @param duration: 遷移にかける秒数
## @param is_in: trueなら出現, falseなら消滅
func wipe_tile(node: CanvasItem, tile_count: Vector2, wipe_dir: Vector2, scale_effect: float, duration: float, is_in: bool) -> Tween:
	var actual_dir = -wipe_dir if is_in else wipe_dir
	var params = {
		"tile_count": tile_count,
		"wipe_direction": actual_dir,
		"smooth_size": 0.1,
		"scale_effect": scale_effect
	}
	return _apply_shader_transition(node, "wipe", params, duration, is_in)

## 汎用のシェーダートランジション実行処理
func _apply_shader_transition(node: CanvasItem, shader_key: String, params: Dictionary, duration: float, is_in: bool) -> Tween:
	if not node or not shaders.get(shader_key): return null
	
	var mat = ShaderMaterial.new()
	mat.shader = shaders[shader_key]
	
	for k in params:
		mat.set_shader_parameter(k, params[k])
	
	node.material = mat
	node.modulate.a = 1.0 # Shader effects fully rely on the shader progress, not node alpha
	
	var start_progress = 1.0 if is_in else 0.0
	var target_progress = 0.0 if is_in else 1.0
	
	mat.set_shader_parameter("progress", start_progress)
	
	var tween = node.create_tween()
	tween.tween_property(mat, "shader_parameter/progress", target_progress, duration).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	
	tween.tween_callback(func():
		node.material = null
		if not is_in:
			node.modulate.a = 0.0
	)
	
	return tween

# --- Tween Based Cocos2d-x Transition Implementations --- #

func _animate_scale_rotate(node: Control, do_rotation: bool, do_jump: bool, duration: float, is_in: bool) -> Tween:
	var tween = node.create_tween()
	node.pivot_offset = node.size / 2.0
	
	var base_scale = Vector2.ONE
	var start_scale = Vector2.ZERO if is_in else base_scale
	var target_scale = base_scale if is_in else Vector2.ZERO
	
	node.scale = start_scale
	node.modulate.a = 0.0 if is_in else 1.0
	
	tween.parallel().tween_property(node, "scale", target_scale, duration).set_trans(Tween.TRANS_BACK).set_ease(Tween.EASE_OUT if is_in else Tween.EASE_IN)
	tween.parallel().tween_property(node, "modulate:a", 1.0 if is_in else 0.0, duration * 0.8).set_delay(0.0 if is_in else duration * 0.2)
	
	if do_rotation:
		var start_rot = deg_to_rad(-720.0) if is_in else 0.0
		var target_rot = 0.0 if is_in else deg_to_rad(720.0)
		node.rotation = start_rot
		tween.parallel().tween_property(node, "rotation", target_rot, duration).set_trans(Tween.TRANS_QUINT).set_ease(Tween.EASE_OUT if is_in else Tween.EASE_IN)
	
	if do_jump:
		if not node.has_meta("original_pos"):
			node.set_meta("original_pos", node.position)
		var base_pos: Vector2 = node.get_meta("original_pos")
		
		# Jump up and down
		var jump_height = 100.0
		var half_t = duration / 2.0
		if is_in:
			node.position = base_pos
			# For 'in', it usually jumps while growing
			var bounce_prop = tween.parallel().tween_property(node, "position:y", base_pos.y - jump_height, half_t).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_OUT)
			bounce_prop.finished.connect(func():
				var t2 = node.create_tween()
				t2.tween_property(node, "position:y", base_pos.y, half_t).set_trans(Tween.TRANS_BOUNCE).set_ease(Tween.EASE_OUT)
			)
		else:
			var bounce_prop = tween.parallel().tween_property(node, "position:y", base_pos.y - jump_height, half_t).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_OUT)
			bounce_prop.finished.connect(func():
				var t2 = node.create_tween()
				t2.tween_property(node, "position:y", base_pos.y, half_t).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN)
			)
			
	return tween

func _animate_flip(node: Control, do_x: bool, do_y: bool, do_zoom: bool, duration: float, is_in: bool) -> Tween:
	var tween = node.create_tween()
	node.pivot_offset = node.size / 2.0
	
	var base_scale = Vector2.ONE
	var flip_scale = Vector2(0.0 if do_x else 1.0, 0.0 if do_y else 1.0)
	
	var start_scale = flip_scale if is_in else base_scale
	var target_scale = base_scale if is_in else flip_scale
	
	node.scale = start_scale
	node.modulate.a = 0.0 if is_in else 1.0
	
	# Scale down (if zooming) then flip
	if do_zoom:
		var zoom_scale = Vector2(0.5, 0.5)
		if is_in:
			# Fast appear from flipped, then zoom to 1.0
			node.scale = flip_scale * zoom_scale
			tween.tween_property(node, "scale", base_scale, duration).set_trans(Tween.TRANS_BACK).set_ease(Tween.EASE_OUT)
		else:
			# Zoom down to half, then flip to zero
			tween.tween_property(node, "scale", zoom_scale, duration * 0.4).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_OUT)
			tween.tween_property(node, "scale", flip_scale, duration * 0.6).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN)
	else:
		# Just flip
		tween.tween_property(node, "scale", target_scale, duration).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	
	tween.parallel().tween_property(node, "modulate:a", 1.0 if is_in else 0.0, duration * 0.5).set_delay(0.0 if is_in else duration * 0.5)
	
	return tween
