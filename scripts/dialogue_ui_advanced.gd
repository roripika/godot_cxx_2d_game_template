extends DialogueUI

signal choice_selected(index: int)

# Nodes for choices and portraits
var choice_container: VBoxContainer
var portrait_rect: TextureRect

func _ready():
    # super._ready() # Not needed for C++ base unless virtual exposed
    
    # Setup Portrait (Left side usually)
    portrait_rect = TextureRect.new()
    portrait_rect.name = "Portrait"
    portrait_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
    portrait_rect.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
    portrait_rect.custom_minimum_size = Vector2(256, 300)
    # Position manually or use anchors? 
    # DialogueUI likely takes up bottom. Portrait usually sits above it or to the side.
    # Let's assume DialogueUI fills bottom 200px. Portrait can be anchored bottom-left of screen, simply added as child.
    # Since DialogueUI might be Control centered, we might need to adjust.
    # For now, simplistic positioning.
    portrait_rect.position = Vector2(50, -320) # Above the dialogue box?
    add_child(portrait_rect)
    
    # Setup Choices (Center screen usually)
    choice_container = VBoxContainer.new()
    choice_container.name = "ChoiceContainer"
    choice_container.set_anchors_preset(Control.PRESET_CENTER)
    # Since child of DialogueUI, anchors relative to it.
    # If DialogueUI is bottom of screen, we want choices in middle of screen?
    # Maybe better to add choice_container to a higher layer or just offset heavily y-axis negative.
    choice_container.position = Vector2(300, -400) 
    choice_container.custom_minimum_size = Vector2(400, 0)
    add_child(choice_container)
    choice_container.visible = false

func show_choices(choices: Array):
    choice_container.visible = true
    # Clear old choices
    for child in choice_container.get_children():
        child.queue_free()
        
    for i in range(choices.size()):
        var btn = Button.new()
        btn.text = choices[i]
        btn.custom_minimum_size = Vector2(0, 40)
        btn.pressed.connect(_on_choice_pressed.bind(i))
        choice_container.add_child(btn)

func _on_choice_pressed(index: int):
    choice_container.visible = false
    choice_selected.emit(index)

func set_portrait(texture: Texture2D):
    portrait_rect.texture = texture
    portrait_rect.visible = (texture != null)
