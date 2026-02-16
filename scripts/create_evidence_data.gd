@tool
extends SceneTree

func _init():
    print("Creating Evidence Data...")
    
    var dir = DirAccess.open("res://samples/mystery")
    if not dir.dir_exists("data"):
        dir.make_dir("data")
    var data_dir = DirAccess.open("res://samples/mystery/data")
    if not data_dir.dir_exists("evidence"):
        data_dir.make_dir("evidence")

    # Create placeholder icon
    var placeholder_icon = GradientTexture2D.new()
    placeholder_icon.width = 64
    placeholder_icon.height = 64
    placeholder_icon.fill = GradientTexture2D.FILL_SQUARE
    placeholder_icon.fill_from = Vector2(0.5, 0.5)
    placeholder_icon.fill_to = Vector2(1, 1)

    var items = [
        {
            "id": "ectoplasm",
            "name": "Ectoplasm",
            "desc": "A glowing substance found on the floor. It feels cold to the touch.",
            "color": Color(0.2, 0.8, 0.2)
        },
        {
            "id": "footprint",
            "name": "Bloody Footprint",
            "desc": "A non-human footprint leading deeper inside. It seems fresh.",
            "color": Color(0.8, 0.2, 0.2)
        },
        {
            "id": "torn_memo",
            "name": "Torn Memo",
            "desc": "A piece of paper with '3 AM' written on it. The rest is missing.",
            "color": Color(0.8, 0.8, 0.8)
        }
    ]
    
    for item_data in items:
        var item = EvidenceItem.new()
        item.id = item_data.id
        item.display_name = item_data.name
        item.description = item_data.desc
        
        # Create unique icon color
        var icon = placeholder_icon.duplicate()
        var grad = Gradient.new()
        grad.set_color(0, item_data.color)
        grad.set_color(1, Color.BLACK)
        icon.gradient = grad
        item.icon = icon
        
        var path = "res://samples/mystery/data/evidence/" + item_data.id + ".tres"
        ResourceSaver.save(item, path)
        print("Saved: " + path)

    print("Evidence data creation complete.")
    quit()
