import sys
from PIL import Image

def remove_magenta_bg(input_path, output_path):
    img = Image.open(input_path).convert("RGBA")
    data = img.getdata()
    
    new_data = []
    # Magenta is #FF00FF (255, 0, 255)
    # We'll use a tolerance to catch anti-aliased edges
    for item in data:
        # Check if the pixel is very close to pure magenta
        # High red, high blue, low green
        if item[0] > 200 and item[1] < 100 and item[2] > 200:
            # For anti-aliasing edges, we could make it partially transparent or just remove it
            # But maybe a simpler strict check first. Let's do a strict check for magenta dominant
            new_data.append((255, 255, 255, 0))
        else:
            new_data.append(item)
            
    img.putdata(new_data)
    img.save(output_path, "PNG")
    print(f"Saved to {output_path}")

input_img = "/Users/ooharayukio/.gemini/antigravity/brain/606fe6a0-a056-4b22-921c-664bf1586252/boss_portrait_magenta_bg_1772277715691.png"

# Target paths
target1 = "/Users/ooharayukio/godot_cxx_2d_game_template/assets/mystery/characters/portraits/boss.png"
target2 = "/Users/ooharayukio/godot_cxx_2d_game_template/assets/mystery/characters/sprites/boss.png"

remove_magenta_bg(input_img, target1)
remove_magenta_bg(input_img, target2)

