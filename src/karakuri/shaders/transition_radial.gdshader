shader_type canvas_item;

// Defines how the wipe progresses:
// 0: Radial (Clockwise)
// 1: Radial (Counter-Clockwise)
// 2: Linear Horizontal (Left to Right)
// 3: Linear Vertical (Top to Bottom)
// 4: Center Out (Expand from center)
// 5: Center In (Shrink to center)
uniform int mode = 0;

// Progress from 0.0 (fully visible) to 1.0 (fully hidden)
uniform float progress : hint_range(0.0, 1.0) = 0.0;
uniform float smoothness : hint_range(0.0, 1.0) = 0.05;

void fragment() {
	vec4 color = texture(TEXTURE, UV);
	
	// Default alpha modifier based on progress
	float alpha = 1.0;
	
	if (mode == 0 || mode == 1) {
		// Radial Wipe
		vec2 center = vec2(0.5, 0.5);
		vec2 uv_centered = UV - center;
		
		// Calculate angle from -PI to PI
		float angle = atan(uv_centered.y, uv_centered.x);
		
		// Normalize angle to 0.0 -> 1.0
		// atan returns 0 at right (3 o'clock). Let's make 12 o'clock the 0 progress point.
		// Rotate by 90 degrees (PI/2)
		angle += PI / 2.0;
		if (angle < 0.0) {
			angle += 2.0 * PI;
		}
		float normalized_angle = angle / (2.0 * PI);
		
		if (mode == 1) { // Counter-Clockwise
			normalized_angle = 1.0 - normalized_angle;
		}
		
		alpha = smoothstep(progress - smoothness, progress + smoothness, normalized_angle);
		
	} else if (mode == 2) {
		// Linear Horizontal (Left to Right)
		alpha = smoothstep(progress - smoothness, progress + smoothness, UV.x);
		
	} else if (mode == 3) {
		// Linear Vertical (Top to Bottom)
		alpha = smoothstep(progress - smoothness, progress + smoothness, UV.y);
		
	} else if (mode == 4 || mode == 5) {
		// Center Out / In (Box or Circle)
		// Let's use distance from center for a circular expand/shrink
		float dist = distance(UV, vec2(0.5, 0.5)) * 2.0; // 0.0 at center, ~1.414 at corners
		
		// Adjust progress to cover the corners
		float adjusted_progress = progress * 1.5; 
		
		if (mode == 4) { // Center Out (Hides from center to edges)
			alpha = smoothstep(adjusted_progress - smoothness, adjusted_progress + smoothness, dist);
		} else { // Center In (Hides from edges to center)
			alpha = 1.0 - smoothstep(1.0 - adjusted_progress - smoothness, 1.0 - adjusted_progress + smoothness, dist);
		}
	}
	
	// Apply alpha to color
	COLOR = vec4(color.rgb, color.a * alpha);
}
