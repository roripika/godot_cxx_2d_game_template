from PIL import Image
import os

files = [
    "assets/mystery/backgrounds/bg_ending_good.png",
    "assets/mystery/backgrounds/bg_ending_bad.png"
]

for f in files:
    if os.path.exists(f):
        print(f"Converting {f}...")
        img = Image.open(f)
        # Force conversion to PNG content
        img.save(f, "PNG")
        print(f"Successfully converted {f} to PNG.")
    else:
        print(f"File not found: {f}")
