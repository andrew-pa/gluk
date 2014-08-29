#version 410

in vec3 pos_w;
in vec3 norm_w;
in vec2 texc;

out vec4 color;

uniform sampler2D tex;

void main()
{
  color = /*mix(vec4(.5, 0., 1., 1.),
              vec4(1., .5, 0., 1.),
              dot(norm_w, vec3(0., 1., 0.))) */ texture2D(tex, texc);
}
