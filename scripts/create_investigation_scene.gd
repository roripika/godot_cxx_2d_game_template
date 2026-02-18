@tool
extends SceneTree

func _init():
    print("Creating Investigation Scene...")
    
    var hotspot_script = load("res://scripts/hotspot.gd")
    if not hotspot_script:
        print("Script not found: res://scripts/hotspot.gd")
        quit(1)
        return

    # Root Node
    var root = Node2D.new()
    root.name = "WarehouseInvestigation"

    # Background
    var bg = ColorRect.new()
    bg.name = "Background"
    bg.color = Color(0.15, 0.15, 0.2)
    bg.set_anchors_preset(Control.PRESET_FULL_RECT)
    root.add_child(bg)
    bg.set_owner(root)

    # Hotspots Container
    var hotspots = Node2D.new()
    hotspots.name = "Hotspots"
    root.add_child(hotspots)
    hotspots.set_owner(root)

    # Create 3 Hotspots
    var items = [
        {
            "id": "ectoplasm_spot",
            "desc": "A weird glowing puddle.",
            "evidence": "ectoplasm",
            "pos": Vector2(300, 400),
            "size": Vector2(100, 60),
            "color": Color(0.2, 0.8, 0.2, 0.5)
        },
        {
            "id": "footprint_spot",
            "desc": "This looks like a paw print, but huge.",
            "evidence": "footprint",
            "pos": Vector2(600, 500),
            "size": Vector2(80, 80),
            "color": Color(0.8, 0.2, 0.2, 0.5)
        },
        {
            "id": "memo_spot",
            "desc": "A torn piece of paper stuck under a crate.",
            "evidence": "torn_memo",
            "pos": Vector2(800, 300),
            "size": Vector2(50, 50),
            "color": Color(0.9, 0.9, 0.9, 0.5)
        }
    ]

    for item in items:
        var spot = Area2D.new()
        spot.name = item.id.capitalize().replace(" ", "")
        spot.position = item.pos
        spot.set_script(hotspot_script)
        spot.hotspot_id = item.id
        # Note: logic moved to YAML
        
        var shape = CollisionShape2D.new()
        shape.name = "CollisionShape2D"
        var rect = RectangleShape2D.new()
        rect.size = item.size
        shape.shape = rect
        spot.add_child(shape)
        shape.set_owner(root)
        
        # Visual Helper (ColorRect)
        var viz = ColorRect.new()
        viz.color = item.color
        viz.size = item.size
        viz.position = -item.size / 2
        spot.add_child(viz)
        viz.set_owner(root)
        
        hotspots.add_child(spot)
        spot.set_owner(root)

    # Save Scene
    var scene = PackedScene.new()
    scene.pack(root)
    var path = "res://samples/mystery/warehouse_investigation.tscn"
    ResourceSaver.save(scene, path)
    
    print("Investigation Scene saved to: " + path)
    quit()
