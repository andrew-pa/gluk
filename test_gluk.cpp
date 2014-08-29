#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
using namespace gluk;

class test_app : public app
{
	mesh* torus;
	shader s;

	mat4 view, proj;
public:
	test_app() 
		: app("test", vec2(640, 480), 16),
		s(read_data_from_package(L"basic.vs.glsl"), read_data_from_package(L"basic.ps.glsl"))
	{
		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_torus<vertex_position_normal_texture, uint16>(vec2(1., .5), 16), string("torus1"));
		
		view = lookAt(vec3(0.f,3.f,-5.f), vec3(0.f), vec3(0.f, 1.f, 0.f));
		proj = perspectiveFov(radians(45.f), dev->size().x, dev->size().y, 0.01f, 1000.f);
	}

	void resize() override
	{
		proj = perspectiveFov(radians(45.f), dev->size().x, dev->size().y, 0.01f, 1000.f);
	}

	void update(float t, float dt) override
	{
	}

	void render(float t, float dt) override
	{
		s.bind();
		s.set_uniform("world", mat4(1));
		s.set_uniform("view_proj", view*proj);
		torus->draw();
		s.unbind();
	}
};

int main()
{
	test_app a;
	a.run();
}