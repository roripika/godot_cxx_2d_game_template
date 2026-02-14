import os
import glob
import subprocess

def process_assets(input_dir, output_dir, sizes):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    png_files = glob.glob(os.path.join(input_dir, "*.png"))
    
    if not png_files:
        print(f"No PNG files found in {input_dir}")
        return

    print(f"Found {len(png_files)} images. Processing with sips...")

    for png_file in png_files:
        filename = os.path.basename(png_file)
        parts = filename.split('_')
        # Expecting zodiac_animal_timestamp.png
        # We want zodiac_animal.png
        if len(parts) >= 3:
            # Check if it is zodiac_pixel_animal_timestamp.png
            if parts[1] == "pixel":
                 # parts = [zodiac, pixel, animal, timestamp]
                 if len(parts) >= 4:
                     animal_name = parts[2]
                     clean_name = f"zodiac_pixel_{animal_name}.png"
                 else:
                     clean_name = filename # Fallback
            else:
                # zodiac_animal_timestamp.png
                animal_name = parts[1]
                clean_name = f"zodiac_{animal_name}.png"
        else:
            clean_name = filename

        try:
            for size in sizes:
                size_dir = os.path.join(output_dir, f"{size}x{size}")
                if not os.path.exists(size_dir):
                    os.makedirs(size_dir)
                
                output_path = os.path.join(size_dir, clean_name)
                
                # sips -z H W input --out output
                cmd = ["sips", "-z", str(size), str(size), png_file, "--out", output_path]
                subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
                print(f"Saved {output_path}")

        except subprocess.CalledProcessError as e:
            print(f"Failed to process {filename}: {e}")

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 3:
        print("Usage: python3 process_assets.py <input_dir> <output_dir> [size1,size2,...]")
        print("Example: python3 process_assets.py assets/zodiac_v2/raw assets/zodiac_v2 64,128")
        sys.exit(1)

    raw_dir = sys.argv[1]
    base_dir = sys.argv[2]
    
    if len(sys.argv) > 3:
        target_sizes = [int(s) for s in sys.argv[3].split(',')]
    else:
        target_sizes = [64, 128]
    
    process_assets(raw_dir, base_dir, target_sizes)
