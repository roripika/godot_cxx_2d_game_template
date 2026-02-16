extends Control

# Signals used by KarakuriScenarioRunner
signal all_rounds_complete(success: bool)

# Dependencies
var dialogue_ui: Control
var inventory_ui: Control

# Testimony Data (Array of Dictionaries)
# {
#   speaker: String,
#   text: String,
#   evidence: String (ID needed to break),
#   shake: String (Text to show on Shake/Press? Or is shake effect?)
# }
var testimonies: Array = []
var current_statement_index: int = 0
var max_rounds: int = 1
var current_round: int = 0 # Not used yet, maybe for loops? Actually HP is better.

# State
var is_active: bool = false
var waiting_for_advance: bool = false

# UI Elements (Bound in _ready)
var btn_press: Button
var btn_present: Button
var health_bar: ProgressBar
var label_mode: Label

func _ready():
    visible = false
    # Find UI components if scene structure is known
    # Assuming this script is on the root Control of testimony_ui.tscn
    btn_press = find_child("ButtonPress", true, false)
    btn_present = find_child("ButtonPresent", true, false)
    health_bar = find_child("HealthBar", true, false)
    label_mode = find_child("LabelMode", true, false)
    
    if btn_press:
        btn_press.pressed.connect(_on_press_pressed)
    if btn_present:
        btn_present.pressed.connect(_on_present_pressed)
        
    set_process_input(false)

# --- API for KarakuriScenarioRunner ---

func set_dialogue_ui(ui: Control):
    dialogue_ui = ui

func set_inventory_ui(ui: Control):
    inventory_ui = ui
    # Connect to InventoryUI signal
    if inventory_ui:
        if inventory_ui.has_signal("item_selected"):
             if not inventory_ui.is_connected("item_selected", _on_evidence_selected):
                 inventory_ui.connect("item_selected", _on_evidence_selected)
        elif inventory_ui.has_signal("evidence_selected"):
             if not inventory_ui.is_connected("evidence_selected", _on_evidence_selected):
                 inventory_ui.connect("evidence_selected", _on_evidence_selected)

func show_current_statement():
    if current_statement_index >= testimonies.size():
        current_statement_index = 0 # Loop back

    var t = testimonies[current_statement_index]
    
    if dialogue_ui:
        dialogue_ui.show_message(t.speaker, t.text)
        
    waiting_for_advance = true

func _on_press_pressed():
    if not is_active: return
    
    print("Hold It!")
    var t = testimonies[current_statement_index]
    
    var press_text = t.shake if t.shake != "" else "Please elaborate on that point."
    if dialogue_ui:
        dialogue_ui.show_message("Detective", press_text)

func _on_present_pressed():
    if not is_active: return
    
    # Open inventory to select item
    if inventory_ui:
         if inventory_ui.has_method("set_present_mode"):
             inventory_ui.call("set_present_mode", true)
         else:
             inventory_ui.visible = !inventory_ui.visible

func _on_evidence_selected(item):
    # Callback from Inventory UI
    if not is_active: return
    
    var item_id = ""
    if typeof(item) == TYPE_OBJECT and item.get("id"):
        item_id = item.id
    elif typeof(item) == TYPE_STRING:
        item_id = item
    
    print("Presenting: " + item_id)
    
    var t = testimonies[current_statement_index]
    if t.evidence == item_id:
        # Correct!
        print("OBJECTION!")
        if dialogue_ui:
            dialogue_ui.show_message("Detective", "This evidence (" + item_id + ") contradicts your claim!")
        
        # End confrontation with Success
        is_active = false
        waiting_for_advance = false
        visible = false
        all_rounds_complete.emit(true)
    else:
        # Incorrect!
        print("TAKE THAT... Wait, no.")
        if dialogue_ui:
            dialogue_ui.show_message("System", "That evidence is irrelevant! (-20% Health)")
            
        # Penalty
        if health_bar:
            health_bar.value -= 20
            if health_bar.value <= 0:
                # Game Over logic
                is_active = false
                visible = false
                all_rounds_complete.emit(false)
                
    # Close inventory if it was open
    if inventory_ui and inventory_ui.has_method("set_present_mode"):
        inventory_ui.call("set_present_mode", false)


func _input(event):
    if not is_active or not visible: return
    
    if event.is_action_pressed("ui_accept") or (event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT):
         # Logic to advance statement on click if not in menu
         # Checks if dialogue is finished typing?
         if dialogue_ui and dialogue_ui.has_method("is_typing_active") and dialogue_ui.is_typing_active():
             return # Let dialogue UI handle skip
         
         # If statement finished, move to next
         if waiting_for_advance:
             # Delay slightly to not conflict with dialogue skip? 
             # For now, simple advance.
             current_statement_index += 1
             show_current_statement()

