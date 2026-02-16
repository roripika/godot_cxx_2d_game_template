@tool
extends SceneTree

func _init():
    print("Creating Main Mystery Scene...")
    
    # Check dependencies
    if not ClassDB.class_exists("KarakuriScenarioRunner") or not ClassDB.class_exists("InteractionManager"):
        print("Error: Required C++ classes not found.")
        quit(1)
        return

    # Root
    var root = Node2D.new()
    root.name = "MainMystery"

    # Scene Container (Where investigation scenes are loaded)
    var container = Node2D.new()
    container.name = "SceneContainer"
    root.add_child(container)
    container.set_owner(root)

    # Interaction Manager (For clicks)
    var interaction = ClassDB.instantiate("InteractionManager") 
    interaction.name = "InteractionManager"
    root.add_child(interaction)
    interaction.set_owner(root)

    # UI Layer
    var canvas = CanvasLayer.new()
    canvas.name = "CanvasLayer"
    root.add_child(canvas)
    canvas.set_owner(root)

    # Dialogue UI
    var dialogue_script = load("res://scripts/dialogue_ui_advanced.gd")
    if not dialogue_script:
        print("Error: script dialogue_ui_advanced.gd not found")
        quit(1)
        return

    var dialogue = ClassDB.instantiate("DialogueUI")
    dialogue.name = "DialogueUI"
    dialogue.set_script(dialogue_script)
    dialogue.set_anchors_preset(Control.PRESET_FULL_RECT)
    canvas.add_child(dialogue)
    dialogue.set_owner(root)

    # Evidence Inventory UI
    var inv_ui_scene = load("res://samples/mystery/ui/evidence_inventory_ui.tscn")
    if inv_ui_scene:
        var inv_ui = inv_ui_scene.instantiate()
        inv_ui.name = "InventoryUI"
        inv_ui.visible = false 
        canvas.add_child(inv_ui)
        inv_ui.set_owner(root)
    else:
        print("Error: EvidenceInventoryUI scene not found!")

    # Inventory Toggle Button
    var inv_btn = Button.new()
    inv_btn.name = "InventoryButton"
    inv_btn.text = "Evidence"
    inv_btn.position = Vector2(1000, 20)
    inv_btn.size = Vector2(120, 40)
    var inv_btn_script = GDScript.new()
    inv_btn_script.source_code = """
extends Button
func _pressed():
    var ui = get_node("../InventoryUI")
    if ui:
        ui.visible = !ui.visible
        if ui.has_method("refresh") and ui.visible:
            ui.refresh()
"""
    inv_btn.set_script(inv_btn_script)
    canvas.add_child(inv_btn)
    inv_btn.set_owner(root)

    # Testimony System UI
    var testimony_ui_scene = load("res://samples/mystery/ui/testimony_ui.tscn")
    if testimony_ui_scene:
        var testimony_ui = testimony_ui_scene.instantiate()
        testimony_ui.name = "TestimonySystem"
        # It's hidden by default in its _ready or init
        canvas.add_child(testimony_ui)
        testimony_ui.set_owner(root)
    else:
        print("Error: TestimonyUI scene not found!")

    # Back to Menu Button (Injecting standard one logic manually or relying on injection script later?)
    # Let's add it manually for safety here as this is a new root.
    var back_script = load("res://samples/common/common_demo_ui.gd")
    if back_script:
        var back_node = Node.new()
        back_node.name = "CommonDemoUI"
        back_node.set_script(back_script)
        root.add_child(back_node)
        back_node.set_owner(root)

    # Scenario Runner
    var runner = ClassDB.instantiate("KarakuriScenarioRunner")
    runner.name = "KarakuriScenarioRunner"
    runner.set("scenario_path", "res://samples/mystery/scenario/mystery.yaml")
    runner.set("scene_container_path", NodePath("../SceneContainer"))
    runner.set("dialogue_ui_path", NodePath("../CanvasLayer/DialogueUI"))
    runner.set("evidence_ui_path", NodePath("../CanvasLayer/InventoryUI"))
    runner.set("testimony_system_path", NodePath("../CanvasLayer/TestimonySystem"))
    runner.set("interaction_manager_path", NodePath("../InteractionManager"))
    root.add_child(runner)
    runner.set_owner(root)

    # Save
    var scene = PackedScene.new()
    scene.pack(root)
    var path = "res://samples/mystery/main_mystery.tscn"
    ResourceSaver.save(scene, path)
    print("Main Mystery Scene saved to: " + path)
    quit()
