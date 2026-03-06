extends InputService

## InputService Autoload Bridge
##
## このスクリプトは C++ karakuri::InputService を Godot の autoload として
## シーンツリーに登録するためのブリッジです。
##
## [autoload] に登録されることで:
## - C++ の InputService::singleton_ がこのノードを指す
## - MysteryPlayer や InteractionComponent が get_singleton() で
##   正常に InputService を取得できる
## - どのシーンからも InputService.get_move_direction() 等が呼べる

func _ready() -> void:
	print("[InputService] Autoload initialized (C++ singleton registered)")
