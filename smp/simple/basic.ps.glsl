#version 410

in vec3 pos_w;
in vec3 norm_w;
in vec2 texc;

out vec4 color;

uniform vec3 diffuse_color;

uniform vec3 light_direction;
uniform vec3 camera_position;

float pdp(vec3 a, vec3 b) {
	return max(0.0, dot(a,b));
}

void main() {
	float ndl = pdp(norm_w, light_direction);
	vec3 v = normalize(camera_position-pos_w);
	float ndv = pdp(norm_w, v);
	vec3 h = normalize(light_direction+v);
	float ndh = pdp(norm_w, h);
	vec3 Kd = diffuse_color * ndl + diffuse_color*pow(1.-ndv, 5.);
	vec3 Ks = vec3(1.f) * pow(ndh, 60.f);
	color = vec4(Kd+Ks, 1.f);
}
