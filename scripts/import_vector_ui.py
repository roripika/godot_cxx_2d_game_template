import os
import shutil
import json
import subprocess
from pathlib import Path

# Paths
GENERATOR_ROOT = Path("/Users/ooharayukio/ai-vector-ui-generator")
GENERATED_DIR = GENERATOR_ROOT / "generated"
OUT_DIR = GENERATOR_ROOT / "out"
CLI_PY = GENERATOR_ROOT / "src" / "cli.py"

GODOT_PROJECT_ROOT = Path(__file__).resolve().parents[1]
ASSETS_UI_DIR = GODOT_PROJECT_ROOT / "assets" / "ui"

def run_cli_render(json_path):
    """ai-vector-ui-generator の CLI を呼び出して PNG を生成する"""
    print(f"Rendering {json_path.name}...")
    try:
        # .venv の python を使用
        venv_python = GENERATOR_ROOT / ".venv" / "bin" / "python"
        if not venv_python.exists():
            venv_python = "python3"
            
        cmd = [
            str(venv_python),
            "-m", "src.cli",
            "render",
            "--in", str(json_path),
            "--out", str(OUT_DIR),
            "--only", "png",
            "--backend", "inkscape"
        ]
        subprocess.run(cmd, check=True, cwd=str(GENERATOR_ROOT))
    except Exception as e:
        print(f"Failed to render {json_path}: {e}")

def calculate_9slice(data):
    """JSON アセットから 9-slice マージンを推論する"""
    viewBox = data.get("viewBox", [0, 0, 100, 100])
    vw, vh = viewBox[2], viewBox[3]
    
    # RoundedRect を持つ最初のレイヤーを検索
    for layer in data.get("layers", []):
        if layer.get("shape") == "roundedRect":
            rect = layer.get("rect", {})
            x = rect.get("x", 0)
            y = rect.get("y", 0)
            radius = rect.get("radius", 0)
            
            # マージンの推論 (微調整が必要な場合もあるが、基本は x + radius)
            margin_l = int(x + radius)
            margin_t = int(y + radius)
            margin_r = int(vw - (x + rect.get("width", 0) - radius))
            margin_b = int(vh - (y + rect.get("height", 0) - radius))
            
            return {
                "left": margin_l,
                "top": margin_t,
                "right": margin_r,
                "bottom": margin_b
            }
    return None

def import_assets():
    if not ASSETS_UI_DIR.exists():
        ASSETS_UI_DIR.mkdir(parents=True, exist_ok=True)

    json_files = list(GENERATED_DIR.glob("*.json"))
    if not json_files:
        print("No generated JSON files found in ai-vector-ui-generator/generated")
        return

    for json_path in json_files:
        asset_name = json_path.stem
        png_filename = f"{asset_name}.png"
        png_out_path = OUT_DIR / png_filename
        
        # PNG がまだないか、JSON より古い場合はレンダリング
        if not png_out_path.exists() or png_out_path.stat().st_mtime < json_path.stat().st_mtime:
            run_cli_render(json_path)
            
        if png_out_path.exists():
            dest_png = ASSETS_UI_DIR / png_filename
            shutil.copy2(png_out_path, dest_png)
            
            # JSON データの読み込みと 9-slice 計算
            with open(json_path, 'r') as f:
                data = json.load(f)
            
            nslice = calculate_9slice(data)
            if nslice:
                if "metadata" not in data:
                    data["metadata"] = {}
                data["metadata"]["godot_9slice"] = nslice
                print(f"Inferred 9-slice: {nslice}")

            # 拡張した JSON をコピー
            dest_json = ASSETS_UI_DIR / f"{asset_name}.ui.json"
            with open(dest_json, 'w') as f:
                json.dump(data, f, indent=2)
            
            print(f"Imported: {dest_png.name} and metadata.")
        else:
            print(f"Warning: PNG not found for {asset_name} after rendering attempt.")

if __name__ == "__main__":
    import_assets()
