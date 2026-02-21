shader_type canvas_item;

// Defines how the screen splits and moves apart:
// 0: Horizontal Rows (Odd rows go left, even rows go right)
// 1: Vertical Columns (Odd cols go up, even cols go down)
uniform int mode = 0;

// Number of splits (e.g. 10 bands)
uniform float splits : hint_range(2.0, 50.0) = 10.0;

// Progress from 0.0 (fully visible) to 1.0 (fully hidden)
uniform float progress : hint_range(0.0, 1.0) = 0.0;

void fragment() {
	vec2 uv = UV;
	
	if (mode == 0) {
		// Split Rows
		// Calculate which horizontal band we are in
		float band = floor(uv.y * splits);
		
		// Alternate direction: even bands move right (+X), odd bands move left (-X)
		float dir = mod(band, 2.0) == 0.0 ? 1.0 : -1.0;
		
		// Offset the UV
		uv.x -= dir * progress;
		
	} else if (mode == 1) {
		// Split Cols
		// Calculate which vertical band we are in
		float band = floor(uv.x * splits);
		
		// Alternate direction: even bands move down (+Y), odd bands move up (-Y)
		float dir = mod(band, 2.0) == 0.0 ? 1.0 : -1.0;
		
		// Offset the UV
		uv.y -= dir * progress;
	}
	
	// Sample the texture with the offset UV
	vec4 color = texture(TEXTURE, uv);
	
	// Discard pixels that have moved out of the 0-1 UV space
	if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
		color.a = 0.0;
	}
	
	COLOR = color;
}
