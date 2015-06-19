#version 410

in vec2 texc;
flat in vec4 color;
flat in float texid;
flat in float istext;
uniform sampler2D textures[31];

layout(location = 0) out vec4 out_color;

vec4 approtrf(vec4 v) {
    if(istext>0.) {
        if (v.r < .01) discard;
        return vec4(1., 1., 1., v.r);//vec4(1., 1., v.r, 1.);
    }
    return v;
}

#define TEXIDX(idx) (texid == (idx)) ? approtrf(texture(textures[(idx)], texc*trf /*(istext>0.?1./vec2(textureSize(textures[(idx)],0)):vec2(1.))*/)) :

void main() {
    //vec4 tv = vec4(1.);
    //if(texid == 128 || istext>0) {
    //    tv.a = texture(font_atlas, texc*vec2(-1.0f, 1.0f)).a;
    //} else if (texid < 32) {
    vec2 trf = istext > 0. ? vec2(1.,1.) : vec2(-1., -1.);
    vec4 tv =
        TEXIDX(0)
        TEXIDX(1)
        TEXIDX(2)
        TEXIDX(3)
        TEXIDX(4)
        TEXIDX(5)
        TEXIDX(6)
        TEXIDX(7)
        TEXIDX(8)
        TEXIDX(9)
        TEXIDX(10)
        TEXIDX(11)
        TEXIDX(12)
        TEXIDX(13)
        TEXIDX(14)
        TEXIDX(15)
        TEXIDX(16)
        TEXIDX(17)
        TEXIDX(18)
        TEXIDX(19)
        TEXIDX(20)
        TEXIDX(21)
        TEXIDX(22)
        TEXIDX(23)
        TEXIDX(24)
        TEXIDX(25)
        TEXIDX(26)
        TEXIDX(27)
        TEXIDX(28)
        TEXIDX(29)
        TEXIDX(30)
        vec4(1.f);
    //(texc.x > 0.5) ? texture(textures[0], texc*vec2(-1., -1.))
    //    : texture(textures[1], texc*vec2(-1., -1.));
    //}
    out_color = color*tv;//vec4(float(texid)*.5, texc, 1.);
}
