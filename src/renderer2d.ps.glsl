#version 410
in vec2 texc;
uniform sampler2D tex;
uniform vec4 vcolor;
uniform bool use_texture;
uniform bool is_text;
layout(location = 0) out vec4 out_color;
void main() {
    vec4 tv = use_texture ? (is_text ?
                        vec4(1.,1.,1.,texture(tex, texc*vec2(-1.0f, 1.0f)).x) :
                        texture(tex, texc)) : vec4(1.);
    out_color = vcolor * tv;
}
