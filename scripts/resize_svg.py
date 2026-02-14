import os
import glob
import subprocess
import sys

def resize_and_export_svg(input_dir, output_dir, size_x, size_y):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    svg_files = glob.glob(os.path.join(input_dir, "*.svg"))
    
    if not svg_files:
        print(f"No SVG files found in {input_dir}")
        return

    print(f"Found {len(svg_files)} SVG files. Converting to {size_x}x{size_y} PNG...")

    for svg_file in svg_files:
        filename = os.path.basename(svg_file)
        name_without_ext = os.path.splitext(filename)[0]
        output_png = os.path.join(output_dir, f"{name_without_ext}.png")
        
        # Use rsvg-convert (part of librsvg) if available, or just use sips/magick
        # Since we are on mac, we can try using sips again, but sips doesn't handle SVG well natively for resizing vector to raster cleanly sometimes.
        # Inkscape is best but might not be installed.
        # Let's tryqlmanage for a preview or just use a simple approach if possible.
        # Actually, since we want a specific pixel art size, maybe just keeping them as SVG is fine?
        # But Godot imports SVG as raster.
        # Let's assume the user wants them as usable assets.
        # We can use `rsvg-convert` if installed, or maybe just `qlmanage` script trick?
        # Wait, the user asked for a script to "output to same size from vector data".
        
        # We can try to use `cairo` or `wand` in python if libraries are there.
        # But safest is maybe just using `rsvg-convert` if we can install it via brew, or just use `magick` if available.
        
        # Check if imagemagick is available
        try:
             subprocess.run(["magick", "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
             cmd = ["magick", "-background", "none", "-size", f"{size_x}x{size_y}", svg_file, output_png]
             subprocess.run(cmd, check=True)
             print(f"Converted {filename} to PNG")
             continue
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass

        # Fallback to a simple text replace in SVG to force size, and then maybe QuickLook?
        # Or just rely on Godot import? Godot can import SVGs and scale them.
        # But the user specifically asked for a script.
        
        print(f"Skipping {filename}: 'magick' not found. Please install ImageMagick (brew install imagemagick) to convert SVGs.")
        return

if __name__ == "__main__":
    # Default usage: python3 resize_svg.py assets/zodiac assets/zodiac/resized 64 64
    if len(sys.argv) < 3:
        print("Usage: python3 resize_svg.py <input_dir> <output_dir> [size_x] [size_y]")
        sys.exit(1)
        
    input_d = sys.argv[1]
    output_d = sys.argv[2]
    sx = int(sys.argv[3]) if len(sys.argv) > 3 else 64
    sy = int(sys.argv[4]) if len(sys.argv) > 4 else 64
    
    resize_and_export_svg(input_d, output_d, sx, sy)
