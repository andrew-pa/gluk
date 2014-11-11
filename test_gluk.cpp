#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "render_target.h"
#include <gli/gli.hpp>
using namespace gluk;

vec4 texture_data[] =
{
	vec4(1.f, 1.f, 1.f, 1.f),
	vec4(1.f, 0.f, 1.f, 1.f),
	vec4(1.f, 1.f, 0.f, 1.f),
	vec4(0.f, 1.f, 1.f, 1.f),
};

class test_app : public app
{
	mesh* torus; mat4 torus_world;
	mesh* screen;
	shader s;
	texture2d tex;
	render_texture2d rtx;
	mat4 view, proj;
public:
	test_app() 
		: app("test", vec2(640, 480), 8),
		s(default_package.open("basic.vs.glsl"), default_package.open("basic.ps.glsl")),
		//s(read_data_from_package(L"basic.vs.glsl"), read_data_from_package(L"basic.ps.glsl")),
		tex(gli::texture2D(gli::load_dds("test.dds"))),
		rtx(uvec2(1024))
	{
		tex.bind(0);
		glGenerateMipmap(GL_TEXTURE_2D);
		tex.mag_filter(GL_LINEAR);
		tex.min_filter(GL_LINEAR_MIPMAP_LINEAR);
		tex.ansiotropic_filter(16.f);
		tex.wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		tex.unbind(0);

		
		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_torus<vertex_position_normal_texture, uint16>(vec2(1., .5), 64), string("torus1"));

		screen = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_plane<vertex_position_normal_texture, uint16>(vec2(4), vec2(16), vec3(0,0,1)), string("video_screen"));
		
		view = lookAt(vec3(0.01f,3.f,-7.f), vec3(0.f), vec3(0.f, 1.f, 0.f));
		proj = perspectiveFov(radians(45.f), dev->size().x, dev->size().y, 0.01f, 1000.f);
	}

	void resize() override
	{
		proj = perspectiveFov(radians(45.f), dev->size().x, dev->size().y, 0.01f, 1000.f);
	}

	void update(float t, float dt) override
	{
		torus_world = rotate(translate(mat4(1), vec3(0.f, 1.4f, 0.f)), t, vec3(.6f, .5f, .4f));
	}

	void render(float t, float dt) override
	{
		dev->push_render_target(&rtx);
		s.bind();
		s.set_uniform("world", torus_world);
		s.set_uniform("itworld", inverse(transpose(torus_world)));
		s.set_uniform("view_proj", perspectiveFov(radians(45.f), 1024.f, 1024.f, .01f, 1000.f)*lookAt(vec3(0.f, 10.f, -15.f), vec3(0.f), vec3(0.f, 1.f, 0.f)));
		tex.bind(0);
		s.set_uniform("tex", 0);
		torus->draw();
		tex.unbind(0);
		s.unbind();
		dev->pop_render_target();

		s.bind();
		
		s.set_uniform("world", torus_world);
		s.set_uniform("itworld", inverse(transpose(torus_world)));
		s.set_uniform("view_proj", proj*view);
		tex.bind(0);
		s.set_uniform("tex", 0);
		torus->draw();
		tex.unbind(0);
		
		s.set_uniform("world", mat4(1));
		s.set_uniform("itworld", mat4(1));
		rtx.bind(0);
		s.set_uniform("tex", 0);
		screen->draw();
		rtx.unbind(0);

		s.unbind();
	}
};

int main()
{
	test_app a;
	a.run();
}