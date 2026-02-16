class_name EvidenceInventoryUI extends Control

signal item_selected(item: EvidenceItem)

@export var all_evidence: Array[EvidenceItem] = []

# Nodes (Must be set up in scene)
@onready var grid_container = $Panel/GridContainer
@onready var description_label = $Panel/DescriptionLabel
@onready var name_label = $Panel/NameLabel
@onready var large_icon = $Panel/LargeIcon
@onready var present_button = $Panel/PresentButton

var selected_item: EvidenceItem = null
var is_present_mode: boolean = false

func _ready():
    visible = false
    present_button.visible = false
    present_button.pressed.connect(_on_present_pressed)
    refresh()

func toggle():
    visible = !visible
    if visible:
        refresh()

func set_present_mode(enabled: bool):
    is_present_mode = enabled
    present_button.visible = enabled
    if enabled:
        visible = true
        refresh()

func refresh():
    # Clear existing items
    for child in grid_container.get_children():
        child.queue_free()
    
    # Get inventory IDs from AdventureGameState
    var game_state = AdventureGameState.get_singleton()
    if not game_state:
        print("AdventureGameState not found")
        return
        
    var collected_ids = game_state.get_inventory()
    
    # Populate grid
    for item in all_evidence:
        if collected_ids.has(item.id):
            var btn = Button.new()
            btn.custom_minimum_size = Vector2(64, 64)
            btn.icon = item.icon
            btn.icon_alignment = HORIZONTAL_ALIGNMENT_CENTER
            btn.expand_icon = true
            btn.pressed.connect(_on_item_clicked.bind(item))
            grid_container.add_child(btn)

    # Clear details
    _update_details(null)

func _on_item_clicked(item: EvidenceItem):
    selected_item = item
    _update_details(item)

func _update_details(item: EvidenceItem):
    if item:
        name_label.text = tr(item.display_name)
        description_label.text = tr(item.description)
        large_icon.texture = item.icon
        present_button.disabled = false
    else:
        name_label.text = ""
        description_label.text = "Select an item..."
        large_icon.texture = null
        present_button.disabled = true

func _on_present_pressed():
    if selected_item:
        item_selected.emit(selected_item)
        if is_present_mode:
            visible = false
