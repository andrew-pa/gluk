#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "render_target.h"
#include "util.h"
#include "renderer2d.h"

using namespace gluk;

#include <memory>

static package sample_package = package(SMP_PATH);

struct simple_app : public app, public input_handler {
	package media_package;

	mesh *torus, *sphere, *floor;
		
	unique_ptr<shader> s;
	util::perspective_camera cam;
	util::fps_camera_controller* fpscamctrl;

	render_texture2d* intrm;

	unique_ptr<util::full_screen_quad_shader> postprocess_s;

	simple_app() : 
		app("simple app", vec2(800, 600), 1),
		media_package(sample_package, "simple/"),
		s(new shader(dev, media_package, "basic.vs.glsl", "basic.ps.glsl")),
		postprocess_s(new util::full_screen_quad_shader(dev, media_package.open("postprocess.ps.glsl"))),
		cam(dev->size(), vec3(0.01, 3.f, -7.f), vec3(0.f), vec3(0.f, 1.f, 0.f)),
		intrm(nullptr)
	{
		create_size_dpe_res();
		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			       generate_torus<vertex_position_normal_texture,uint16>(vec2(1.f, 0.5f), 64));
		sphere = new interleaved_mesh<vertex_position_normal_texture, uint32>(
				generate_sphere<vertex_position_normal_texture, uint32>(1.f, 32, 32));
		floor = new interleaved_mesh<vertex_position_normal_texture, uint32>(
				generate_plane<vertex_position_normal_texture, uint32>(vec2(16.f), vec2(8.f), vec3(0.f, -1.f, 0.f)));	
		fpscamctrl = new util::fps_camera_controller(cam);
		input_handlers.push_back(fpscamctrl);
		input_handlers.push_back(this);
	}

	void resize() override {
		cam.update_proj(dev->size());
		create_size_dpe_res();
	}
	void create_size_dpe_res() {
		if(intrm != nullptr) delete intrm;
		intrm = new render_texture2d(uvec2(dev->size()));
	}


	void update(float t, float dt) override { 
		fpscamctrl->update(t, dt);
	}
	void key_handler(app* a, uint kc, input_action ac, input_mod md) {
		if(ac == input_action::release) {
			if(kc == GLFW_KEY_F3) {
				s.reset();
				s = unique_ptr<shader>(
					new shader(dev, media_package, "basic.vs.glsl", "basic.ps.glsl"));
				postprocess_s.reset();
				postprocess_s = unique_ptr<util::full_screen_quad_shader>(new util::full_screen_quad_shader(dev, media_package.open("postprocess.ps.glsl")));
			}
		}
	}
	void render(float t, float dt) override { 
		dev->push_render_target(intrm);
		s->bind();
		s->set_uniform("view_proj", cam.proj()*cam.view());
		s->set_uniform("camera_position", cam.position());
		s->set_uniform("light_direction", vec3(0.f, 1.f, 0.f));
		
		s->set_uniform("world", mat4(1));
		s->set_uniform("diffuse_color", vec3(0.2f, 0.2f, 0.1f));
		floor->draw();

		s->set_uniform("world", 
			rotate(translate(mat4(1), vec3(-2.f, 2.f, 0.f)), t, vec3(1.f, 0.3f, 0.6f)));
		s->set_uniform("diffuse_color", vec3(0.8f, 0.6f, 0.1f));
		torus->draw();
		
		s->set_uniform("world", translate(mat4(1), vec3(2.f, 1.f, 0.f)));
		s->set_uniform("diffuse_color", vec3(0.3f, 0.6f, 0.8f));
		sphere->draw();
		dev->pop_render_target();
		postprocess_s->bind();
		postprocess_s->set_texture("backbuffer", *intrm, 0);
		postprocess_s->draw(t);
	}	
};

int main() {
	simple_app a;
	a.run();
}
