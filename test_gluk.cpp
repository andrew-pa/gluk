#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
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
	shader s;
	texture2d tex;
	mat4 view, proj;
public:
	test_app() 
		: app("test", vec2(640, 480), 16),
		s(read_data_from_package(L"basic.vs.glsl"), read_data_from_package(L"basic.ps.glsl")),
		tex(pixel_format(pixel_components::rgba, pixel_type::floatp, 32), uvec2(2, 2), texture_data)
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
		
		view = lookAt(vec3(0.1f,5.f,-5.f), vec3(0.f), vec3(0.f, 1.f, 0.f));
		proj = perspectiveFov(radians(45.f), dev->size().x, dev->size().y, 0.01f, 1000.f);
	}

	void resize() override
	{
		proj = perspectiveFov(radians(45.f), dev->size().x, dev->size().y, 0.01f, 1000.f);
	}

	void update(float t, float dt) override
	{
		torus_world = rotate(mat4(1), t, vec3(.6f, .5f, .4f));
	}

	void render(float t, float dt) override
	{
		s.bind();
		s.set_uniform("world", torus_world);
		s.set_uniform("itworld", inverse(transpose(torus_world)));
		s.set_uniform("view_proj", proj*view);
		tex.bind(0);
		s.set_uniform("tex", 0);
		torus->draw();
		tex.unbind(0);
		s.unbind();
	}
};

int main()
{
	test_app a;
	a.run();
}