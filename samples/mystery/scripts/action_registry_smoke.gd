# action_registry_smoke.gd
# ActionRegistry + ScenarioRunner + TaskAddEvidence E2E smoke test.
#
# Usage:
#   godot --headless --path . \
#     --script res://samples/mystery/scripts/action_registry_smoke.gd
extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _ok(msg: String) -> void:
	print("[SMOKE] PASS: " + msg)

func _fail(msg: String) -> void:
	push_error("[SMOKE] FAIL: " + msg)
	quit(1)

func _chk(cond: bool, msg: String) -> void:
	if cond:
		_ok(msg)
	else:
		_fail(msg)

func _run() -> void:
	print("[SMOKE] === ActionRegistry E2E smoke test ===")

	# 1. ActionRegistry Engine singleton
	var reg = Engine.get_singleton("ActionRegistry")
	_chk(reg != null, "ActionRegistry Engine singleton exists")
	_chk(reg.has_action("add_evidence"),
		"add_evidence registered via MysteryGameState._ready()")

	# 2. MysteryGameState Autoload
	var mgs = root.get_node_or_null("AdventureGameState")
	_chk(mgs != null, "Autoload /root/AdventureGameState exists")
	_chk(not mgs.has_evidence("bloody_knife"), "initial: bloody_knife not acquired")

	# 3. ScenarioRunner.execute_single_action
	_chk(ClassDB.class_exists("ScenarioRunner"), "ScenarioRunner in ClassDB")
	var runner: Node = ClassDB.instantiate("ScenarioRunner")
	root.add_child(runner)
	await process_frame

	runner.execute_single_action({"action": "add_evidence", "evidence_id": "bloody_knife"})
	await process_frame

	# 4. Victory line
	_chk(mgs.has_evidence("bloody_knife"),
		"[VICTORY] bloody_knife added to MysteryGameState!")

	print("[SMOKE] === All tests passed! ===")
	quit(0)
