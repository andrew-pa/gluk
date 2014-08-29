#version 410

in vec3 pos_w;
in vec3 norm_w;
in vec2 texc;

out vec4 color;

uniform sampler2D tex;

void main()
{
  vec4 tc = texture2D(tex, texc);
  vec4 itc = vec4(1.) - tc;
  color = mix(itc, tc, dot(norm_w, vec3(0., 1., 0.)));

  /*mix(vec4(.5, 0., 1., 1.),
              vec4(1., .5, 0., 1.),
              dot(norm_w, vec3(0., 1., 0.))); */
}
