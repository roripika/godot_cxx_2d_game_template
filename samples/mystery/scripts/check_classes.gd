extends SceneTree

func _initialize():
	print("Checking global classes...")
	var classes = ClassDB.get_class_list()
	var targets = ["InputService", "MysteryGameState", "ActionRegistry", "ConditionEvaluator"]
	for t in targets:
		if classes.has(t):
			print("[FOUND] ", t)
		else:
			print("[MISSING] ", t)
	
	quit(0)
