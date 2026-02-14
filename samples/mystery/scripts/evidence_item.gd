# EvidenceItem.gd
# 証拠品データリソース
class_name EvidenceItem extends Resource

@export var id: String       # 一意識別子
@export var display_name: String  # 表示名
@export var description: String   # 説明
@export var icon: Texture2D       # アイコン画像
@export var category: String = "物証"  # カテゴリ（物証/証言/その他）

func _to_string() -> String:
	return "EvidenceItem(id=%s, name=%s)" % [id, display_name]
