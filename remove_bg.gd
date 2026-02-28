extends SceneTree

func _init():
	print("[VERIFY] Starting background removal using GDScript...")
	var img = Image.load_from_file("/Users/ooharayukio/.gemini/antigravity/brain/606fe6a0-a056-4b22-921c-664bf1586252/boss_portrait_magenta_bg_1772277715691.png")
	if not img:
		print("[VERIFY] Failed to load image!")
		quit(1)
		return
		
	# Make sure the image has an alpha channel
	if img.get_format() != Image.FORMAT_RGBA8:
		img.convert(Image.FORMAT_RGBA8)
		
	for x in range(img.get_width()):
		for y in range(img.get_height()):
			var c = img.get_pixel(x, y)
			# Magenta is high red, high blue, low green
			if c.r > 0.8 and c.b > 0.8 and c.g < 0.2:
				c.a = 0.0
				img.set_pixel(x, y, c)
				
	img.save_png("res://assets/mystery/characters/portraits/boss.png")
	img.save_png("res://assets/mystery/characters/sprites/boss.png")
	print("[VERIFY] Background removed and images saved!")
	quit(0)
