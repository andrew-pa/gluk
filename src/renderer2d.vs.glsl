#version 410
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 i_texc;

uniform vec2 invres;
uniform vec2 offset;
uniform vec2 size;

out vec2 texc;
void main() {
    texc = i_texc;
    vec4 p = vec4(pos, 1.f);
    p.xy *= size;
    p.xy += offset;
    p.xy *= invres;
    //p *= trnf;
    gl_Position = p;
}
