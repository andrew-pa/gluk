#version 410
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 i_texc;

layout(location = 3) in vec4 screen_rect;
layout(location = 4) in vec4 texture_rect;
layout(location = 5) in vec4 vcolor;
layout(location = 6) in vec4 voptions;

uniform vec2 invres;
uniform vec2 screen_offset;

flat out vec4 color;
flat out float texid;
flat out float istext;
out vec2 texc;

void main() {
    vec2 itexcv = voptions.y > 0. ? vec2(1.-i_texc.x,i_texc.y) : i_texc;
    texc = itexcv*texture_rect.zw + texture_rect.xy;
    color = vcolor;
    texid = voptions.x;
    istext = voptions.y;
    vec4 p = vec4(pos, 1.f);
    p.xy *= screen_rect.zw;
    p.xy += screen_rect.xy+(voptions.y > 0. ? screen_rect.zw : vec2(0.)) + screen_offset;
    p.xy *= invres;
    //p *= trnf;
    gl_Position = p;
}
