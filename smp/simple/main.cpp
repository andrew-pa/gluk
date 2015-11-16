#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "render_target.h"
#include "util.h"
#include "renderer2d.h"

using namespace gluk;

static package sample_package = package(SMP_PATH);

struct simple_app : public app {
	package media_package;

	mesh *torus, *sphere, *floor;
		
	shader s;
	util::perspective_camera cam;
	util::fps_camera_controller* fpscamctrl;


	simple_app() : 
		app("simple app", vec2(800, 600), 1),
		media_package(sample_package, "simple/"),
		s(dev, media_package, "basic.vs.glsl", "basic.ps.glsl"),
		cam(dev->size(), vec3(0.01, 3.f, -7.f), vec3(0.f), vec3(0.f, 1.f, 0.f))
	{
		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			       generate_torus<vertex_position_normal_texture,uint16>(vec2(1.f, 0.5f), 64));
		sphere = new interleaved_mesh<vertex_position_normal_texture, uint32>(
				generate_sphere<vertex_position_normal_texture, uint32>(1.f, 32, 32));
		floor = new interleaved_mesh<vertex_position_normal_texture, uint32>(
				generate_plane<vertex_position_normal_texture, uint32>(vec2(16.f), vec2(8.f), vec3(0.f, -1.f, 0.f)));	
		fpscamctrl = new util::fps_camera_controller(cam);
		input_handlers.push_back(fpscamctrl);
	}

	void resize() override {
		cam.update_proj(dev->size());
	}
	void update(float t, float dt) override { 
		fpscamctrl->update(t, dt);
	}
	
	void render(float t, float dt) override { 
		s.bind();
		s.set_uniform("view_proj", cam.proj()*cam.view());
		s.set_uniform("camera_position", cam.position());
		s.set_uniform("light_direction", vec3(0.f, 1.f, 0.f));
		
		s.set_uniform("world", mat4(1));
		s.set_uniform("diffuse_color", vec3(0.2f, 0.2f, 0.1f));
		floor->draw();

		s.set_uniform("world", rotate(translate(mat4(1), vec3(-2.f, 2.f, 0.f)), t, vec3(1.f, 0.3f, 0.6f)));
		s.set_uniform("diffuse_color", vec3(0.8f, 0.6f, 0.1f));
		torus->draw();
		
		s.set_uniform("world", translate(mat4(1), vec3(2.f, 1.f, 0.f)));
		s.set_uniform("diffuse_color", vec3(0.3f, 0.6f, 0.8f));
		sphere->draw();
	}	
};

int main() {
	simple_app a;
	a.run();
}
