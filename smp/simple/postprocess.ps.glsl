#version 410

uniform float time;
uniform vec2 resolution;

uniform sampler2D backbuffer;

out vec4 color;

void main() {
	vec2 uv = gl_FragCoord.xy / resolution; 
	float v = .1 + 1.0*pow(12.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.2);
	vec4 rc = texture(backbuffer, uv);
	color = pow(rc*v, vec4(1.0/2.2)) + vec4(uv, sin(time), 1.);
}
