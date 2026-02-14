extends SceneTree

func _init():
    var common_script = load("res://samples/common/common_demo_ui.gd")
    if not common_script:
        print("Error: Could not load common_demo_ui.gd")
        quit(1)
        return

    var targets = [
        "res://samples/rhythm/rhythm_demo.tscn",
        "res://samples/rhythm/rhythm_demo_type_a.tscn",
        "res://samples/rhythm/rhythm_demo_type_b.tscn",
        "res://samples/fighting/fighting_demo.tscn",
        "res://samples/sandbox/sandbox_demo.tscn",
        "res://samples/mystery/demo_adv.tscn",
        "res://samples/gallery/gallery_demo.tscn"
    ]

    for path in targets:
        if not FileAccess.file_exists(path):
            print("Skipping missing file: " + path)
            continue
            
        var scene = load(path)
        if not scene:
            print("Error loading scene: " + path)
            continue
            
        var root = scene.instantiate()
        if not root:
            print("Error instantiating scene: " + path)
            continue
            
        # check if already added
        if root.has_node("CommonDemoUI"):
            print("CommonDemoUI already exists in: " + path)
            # Optionally remove and re-add if updating script? 
            # root.get_node("CommonDemoUI").queue_free()
            # For now, assume it's fine.
        else:
            print("Injecting CommonDemoUI into: " + path)
            var node = Node.new()
            node.name = "CommonDemoUI"
            node.set_script(common_script)
            root.add_child(node)
            node.set_owner(root)
            
            var packed = PackedScene.new()
            packed.pack(root)
            ResourceSaver.save(packed, path)
            
    print("Injection complete.")
    quit(0)
