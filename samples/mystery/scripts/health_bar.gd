# HealthBar.gd
# HP表示UI（ハート形式）
extends HBoxContainer
class_name HealthBar

@export var heart_scene: PackedScene  # ハートのシーン（オプション）
@export var max_health: int = 3
@export var heart_color: Color = Color.RED
@export var empty_color: Color = Color.GRAY

var current_health: int = 3
var heart_icons: Array[ColorRect] = []

func _ready():
	current_health = max_health
	_create_hearts()
	
	# AdventureGameStateの変更を監視
	if AdventureGameState and AdventureGameState.has_signal("health_changed"):
		if not AdventureGameState.is_connected("health_changed", Callable(self, "_on_health_changed")):
			AdventureGameState.health_changed.connect(_on_health_changed)
		# 初期値を反映
		if AdventureGameState.has_method("get_health"):
			update_health(AdventureGameState.get_health())

func _create_hearts():
	"""ハートUI を作成"""
	# 既存の子を削除
	for child in get_children():
		child.queue_free()
	heart_icons.clear()
	
	for i in range(max_health):
		var heart = ColorRect.new()
		heart.custom_minimum_size = Vector2(32, 32)
		heart.color = heart_color
		
		add_child(heart)
		heart_icons.append(heart)

func update_health(new_health: int):
	"""HPを更新"""
	current_health = max(0, min(new_health, max_health))
	_refresh_display()

func take_damage():
	"""ダメージを受ける"""
	update_health(current_health - 1)
	
	# ダメージアニメーション
	_play_damage_animation()

func heal(amount: int):
	"""回復"""
	update_health(current_health + amount)

func _refresh_display():
	"""表示を更新"""
	for i in range(heart_icons.size()):
		if i < current_health:
			heart_icons[i].color = heart_color
		else:
			heart_icons[i].color = empty_color

func _play_damage_animation():
	"""ダメージアニメーション"""
	var tween = create_tween()
	tween.set_trans(Tween.TRANS_BOUNCE)
	tween.set_ease(Tween.EASE_OUT)
	tween.tween_property(self, "scale", Vector2(1.1, 1.1), 0.1)
	tween.tween_property(self, "scale", Vector2(1.0, 1.0), 0.1)

func get_current_health() -> int:
	"""現在のHP取得"""
	return current_health

func is_dead() -> bool:
	"""HP0か確認"""
	return current_health <= 0

func _on_health_changed(new_health: int):
	"""HP変更時のコールバック"""
	update_health(new_health)
