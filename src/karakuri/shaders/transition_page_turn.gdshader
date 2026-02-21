shader_type canvas_item;

// Defines if the page turns from left-to-right (0) or right-to-left (1)
// or top-to-bottom (2) or bottom-to-top (3)
uniform int mode = 1;

// Progress from 0.0 (fully flat) to 1.0 (fully curled and moved away)
uniform float progress : hint_range(0.0, 1.0) = 0.0;

// Size of the curling cylinder (larger = flatter curl)
uniform float curl_radius : hint_range(0.01, 1.0) = 0.2;

const float PI = 3.14159265359;

void fragment() {
	vec2 uv = UV;
	
	// Normalize progress so 1.0 completely clears the screen
	// We need to move the curl center from 1.0 + curl_radius to -curl_radius
	float total_distance = 1.0 + (curl_radius * PI);
	
	float curl_center;
	float current_pos;
	
	if (mode == 1) { // Right to Left
		curl_center = 1.0 + curl_radius - (progress * total_distance);
		current_pos = uv.x;
	} else if (mode == 0) { // Left to Right
		curl_center = -curl_radius + (progress * total_distance);
		current_pos = 1.0 - uv.x;
	} else if (mode == 2) { // Top to Bottom
		curl_center = -curl_radius + (progress * total_distance);
		current_pos = 1.0 - uv.y;
	} else { // Bottom to Top
		curl_center = 1.0 + curl_radius - (progress * total_distance);
		current_pos = uv.y;
	}
	
	vec4 color = texture(TEXTURE, uv);
	
	// If the pixel is behind the curl center, it's flat on the page
	if (current_pos < curl_center) {
		COLOR = color;
	} else {
		// The pixel is in the curled section
		float dist = current_pos - curl_center;
		
		// If the distance is greater than half the cylinder circumference,
		// it's wrapped entirely around and hidden
		float half_circ = curl_radius * PI;
		
		if (dist > half_circ) {
			COLOR = vec4(0.0);
		} else {
			// Calculate the angle on the cylinder
			float angle = dist / curl_radius;
			
			// Calculate how much light hits the curve
			// Sin of angle gives us a normal pointing towards the viewer,
			// adding a slight shadow as it curves away.
			float shadow = max(0.0, sin(angle));
			
			// Map the UV back to the flat texture to see the back of the page
			// The distance along the curve maps back to the flat page distance
			// To see the "back" of the image correctly mirrored, we subtract 
			// the distance from the curl center instead of adding it.
			// But for simplicity, we just show a darkened version of the front or a solid color.
			// We'll just show the front texture with lighting, curving away.
			
			// Find the actual texture UV at this curved point (this is a simplified 2D approximation)
			float flat_dist = curl_radius * sin(angle);
			vec2 mapped_uv = uv;
			
			if (mode == 1) {
				mapped_uv.x = curl_center + flat_dist;
			} else if (mode == 0) {
				mapped_uv.x = 1.0 - (curl_center + flat_dist);
			} else if (mode == 2) {
				mapped_uv.y = 1.0 - (curl_center + flat_dist);
			} else {
				mapped_uv.y = curl_center + flat_dist;
			}
			
			vec4 curled_color = texture(TEXTURE, mapped_uv);
			
			// Determine if we are looking at the back of the page
			// If angle > PI/2, we're on the back half of the cylinder
			if (angle > PI / 2.0) {
				// Back of the page (darkened)
				curled_color.rgb *= 0.5;
				// Add a hard shadow right on the fold
				curled_color.rgb *= smoothstep(PI/2.0, PI, angle);
			} else {
				// Front of the page, add highlight on the curve
				float highlight = pow(cos(angle - PI/4.0), 10.0) * 0.3;
				curled_color.rgb += vec3(highlight);
			}
			
			COLOR = curled_color;
		}
	}
}
