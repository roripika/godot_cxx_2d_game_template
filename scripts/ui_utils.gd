extends Node

# Generation Tool が出力した .ui.json を読み込み、対象の Node に設定を反映する
static func apply_ui_metadata(node: Control, json_path: String):
    var file = FileAccess.open(json_path, FileAccess.READ)
    if not file:
        push_error("Failed to open UI metadata: " + json_path)
        return
        
    var json = JSON.new()
    var error = json.parse(file.get_as_text())
    if error != OK:
        push_error("Failed to parse UI metadata JSON")
        return
        
    var data = json.data
    var metadata = data.get("metadata", {})
    var nslice = metadata.get("godot_9slice", {})
    
    if node is NinePatchRect and nslice:
        node.patch_margin_left = nslice.get("left", 0)
        node.patch_margin_top = nslice.get("top", 0)
        node.patch_margin_right = nslice.get("right", 0)
        node.patch_margin_bottom = nslice.get("bottom", 0)
        print("Applied 9-slice margins to ", node.name)
    elif node is TextureRect and nslice:
        # TextureRect では直接 9-slice はできないが、情報として保持
        node.set_meta("ui_metadata", data)
