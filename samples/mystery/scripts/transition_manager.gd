extends Node
class_name KarakuriTransitionManager

## アドベンチャーシーンやUIにおける汎用トランジションを管理するマネージャー。
## ShaderMaterialを使ったワイプやTweenによるスライド/フェードなどを提供します。

# ワイプ用の共通シェーダーリソース（初期化時に一度だけロードされることを想定）
const WIPE_SHADER_PATH = "res://src/karakuri/shaders/transition_wipe.gdshader"
var wipe_shader: Shader = null

func _ready() -> void:
	if ResourceLoader.exists(WIPE_SHADER_PATH):
		wipe_shader = load(WIPE_SHADER_PATH) as Shader

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

## シェーダーを利用したタイル状ワイプ/グラデーションワイプエフェクトを適用します。
## @param node: 対象の CanvasItem
## @param tile_count: タイル分割数。Vector2(1,1)ならプレーンなグラデーション
## @param wipe_dir: ワイプの進行方向 (例: Vector2(1,0) で左から右へ消える/現れる)
## @param scale_effect: 1.0ならタイルごとに小さくなりながら消える。0.0ならアルファのみ
## @param duration: 遷移にかける秒数
## @param is_in: trueなら出現, falseなら消滅
func wipe_tile(node: CanvasItem, tile_count: Vector2, wipe_dir: Vector2, scale_effect: float, duration: float, is_in: bool) -> Tween:
	if not node or not wipe_shader: return null
	
	# シェーダーマテリアルの準備（既存のものを上書き）
	var mat = ShaderMaterial.new()
	mat.shader = wipe_shader
	mat.set_shader_parameter("tile_count", tile_count)
	node.material = mat
	
	# "出現"のときは進行方向を逆にすることで、向かってきた方向から現れるようにする
	var actual_dir = wipe_dir
	if is_in:
		actual_dir = -wipe_dir
	
	mat.set_shader_parameter("wipe_direction", actual_dir)
	mat.set_shader_parameter("smooth_size", 0.1)
	mat.set_shader_parameter("scale_effect", scale_effect)
	
	# is_in が true = progress は 1.0(消滅) から 0.0(出現) へ向かう
	# is_in が false = progress は 0.0(出現) から 1.0(消滅) へ向かう
	var start_progress = 1.0 if is_in else 0.0
	var target_progress = 0.0 if is_in else 1.0
	
	mat.set_shader_parameter("progress", start_progress)
	
	# 念のためノード自体のアルファは1に戻す（見えなくならないように）
	node.modulate.a = 1.0
	
	var tween = node.create_tween()
	tween.tween_property(mat, "shader_parameter/progress", target_progress, duration).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	
	# Tween完了後にマテリアルを掃除する（重くならないように）
	tween.tween_callback(func():
		if is_in:
			node.material = null
			# 完全に表示されている状態に戻す
		else:
			# 消滅の場合は非表示を維持するためアルファ0にしてからマテリアルを外す
			node.modulate.a = 0.0
			node.material = null
	)
	
	return tween
