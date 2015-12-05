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

struct shadow_smp : public app {
	package media_package;

	mesh *torus, *sphere, *floor;
		
	shader s;
	util::perspective_camera cam;
	util::fps_camera_controller* fpscamctrl;

	vec3 light_dir;
	depth_render_texture2d shadow_map;
	shader shadow_depth_render;

	graphics2d::renderer2d rndr2d;	

	shadow_smp() : 
		app("simple app with shadows", vec2(800, 600), 1),
		rndr2d(dev, default_package),
		media_package(sample_package, "shadows/"),
		s(dev, media_package, "basic.vs.glsl", "basic.ps.glsl"),
		shadow_depth_render(dev, media_package, "basic.vs.glsl"),
		shadow_map(uvec2(1024)), 
		light_dir(normalize(vec3(0.5f, 0.8f, 0.f))),
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

	mat4 calc_light_shadow_vp(vec3 ld) {
		const float r = 5.f;
		return ortho(-r, r, -r, r, r*0.5f, r+2.f) * lookAt(ld*-r, vec3(0.f), vec3(0.f,1.f,0.f));
	}

	void render_geometry(float t, float dt, shader& sh) {
		sh.set_uniform("world", mat4(1));
		sh.set_uniform("diffuse_color", vec3(0.2f, 0.2f, 0.1f));
		floor->draw();

		sh.set_uniform("world", rotate(translate(mat4(1), vec3(-2.f, 2.f, 0.f)), t, vec3(1.f, 0.3f, 0.6f)));
		sh.set_uniform("diffuse_color", vec3(0.8f, 0.6f, 0.1f));
		torus->draw();
		
		sh.set_uniform("world", translate(mat4(1), vec3(2.f, 1.f, 0.f)));
		sh.set_uniform("diffuse_color", vec3(0.3f, 0.6f, 0.8f));
		sphere->draw();
	}	
	void render(float t, float dt) override { 
		mat4 lvp = calc_light_shadow_vp(light_dir); 
		dev->push_render_target(&shadow_map);
		shadow_depth_render.bind();
		shadow_depth_render.set_uniform("view_proj", lvp);
		render_geometry(t, dt, shadow_depth_render);
		dev->pop_render_target();

		s.bind();
		s.set_uniform("view_proj", cam.proj()*cam.view());
		s.set_uniform("camera_position", cam.position());
		s.set_uniform("light_direction", light_dir);
		s.set_uniform("light_view_proj", lvp);
		s.set_texture("light_shadow_map", shadow_map, 0);
		render_geometry(t, dt, s);

		rndr2d.begin_draw();
		rndr2d.draw_rect(vec2(256.f), vec2(256.f), vec4(1.f), &shadow_map);
		rndr2d.end_draw();
	}	
};

int main() {
	shadow_smp a;
	a.run();
}
