#version 410

in vec3 pos_w;
in vec3 norm_w;
in vec2 texc;

out vec4 color;

uniform vec3 diffuse_color;

uniform vec3 light_direction;
uniform mat4 light_view_proj;
uniform sampler2D light_shadow_map;


uniform vec3 camera_position;

float pdp(vec3 a, vec3 b) {
	return max(0.0, dot(a,b));
}

vec3 calc_shadow() {
	vec4 lp = light_view_proj * vec4(pos_w,1.f);
	lp /= lp.w; lp = lp*0.5 + 0.5;
	float ld = texture(light_shadow_map, lp.xy  ).g;
	return vec3(lp.xy, 10000.f/ld);
}

void main() {
	float ndl = pdp(norm_w, light_direction);
	vec3 v = normalize(camera_position-pos_w);
	float ndv = pdp(norm_w, v);
	vec3 Kd = diffuse_color * ndl + diffuse_color*pow(1.-ndv, 5.);
	color = vec4(calc_shadow(), 1.f);
}
