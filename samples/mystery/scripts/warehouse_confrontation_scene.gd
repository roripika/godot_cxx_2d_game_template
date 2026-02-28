# WarehouseConfrontationScene.gd
# 対決パート（倉庫 - 尋問）
# ※ロジックは C++ (KarakuriScenarioRunner) が YAML ベースで管理します。
extends Node2D

@onready var testimony_system = $CanvasLayer/TestimonySystem

func _ready():
	# UIの初期化
	if testimony_system:
		# TestimonySystem (GDScript) は信号を発火し、C++ 側がそれを受信して処理します。
		# ※実際の表示タイミングは C++ 側のアクション (testimony) によって制御されるため、
		# ここでの強制表示はレースコンディション（active=false 状態で選択される）を招くので削除します。
		# testimony_system.show_panel()
		pass

# 勝利・敗北時の演出などはシナリオ (YAML) の on_success / on_failure で制御されるため、
# このスクリプトでの直接的な終了判定は不要です。

