#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "render_target.h"
#include "util.h"
#include <gli/gli.hpp>

#include "ft2build.h"
#include FT_FREETYPE_H

using namespace gluk;

vec4 texture_data[] =
{
	vec4(1.f, 1.f, 1.f, 1.f),
	vec4(1.f, 0.f, 1.f, 1.f),
	vec4(1.f, 1.f, 0.f, 1.f),
	vec4(0.f, 1.f, 1.f, 1.f),
};


class test_app : public app, public input_handler
{
	mesh* torus; mat4 torus_world;
	mesh* screen;
	shader s;
	texture2d tex;
	render_texture2d rtx;
	//mat4 view, proj;
	util::camera cam;
	shared_ptr<util::fps_camera_controller> fpscamcntrl;
	GLuint fbo;
	GLuint rbo, frx;
public:
	test_app()
		: app("test", vec2(640, 480), 1),
		s(dev, default_package, "basic.vs.glsl", "basic.ps.glsl"),//default_package.open("basic.vs.glsl"), default_package.open("basic.ps.glsl")),
		//s(read_data_from_package(L"basic.vs.glsl"), read_data_from_package(L"basic.ps.glsl")),
		tex(default_package.open("test.tga"))//gli::texture2D(gli::load_dds("test.dds")))
		, rtx(vec2(1024)), clear_color(0.1f, 0.3f, 0.8f, 1.f), cam(dev->size(), vec3(0.01f, 3.f, -7.f), vec3(0.f), vec3(0.f, 1.f, 0.f)),
		fpscamcntrl(make_shared<util::fps_camera_controller>(cam))
	{
		glerr
		tex.bind(0);
		glGenerateMipmap(GL_TEXTURE_2D);
		tex.mag_filter(GL_LINEAR_MIPMAP_LINEAR);
		tex.min_filter(GL_LINEAR_MIPMAP_LINEAR);
		tex.ansiotropic_filter(16.f);
		tex.wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		tex.unbind(0);

		/*rtx.bind(0);
		rtx.mag_filter(GL_LINEAR);
		rtx.min_filter(GL_LINEAR);
		rtx.ansiotropic_filter(16.f);
		rtx.wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		rtx.unbind(0);
		*/
		
		glGenFramebuffers(1, &fbo);
		glerr
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glerr;
		glGenTextures(1, &frx);
		glBindTexture(GL_TEXTURE_2D, frx);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frx, 0);
		glerr;
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, pixel_format(pixel_components::depth, pixel_type::floatp, 32).get_gl_format_internal(), 1024, 1024);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fdrtx.id(), 0);
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			throw;
		glerr


		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_torus<vertex_position_normal_texture, uint16>(vec2(1., .5), 64), string("torus1"));

		screen = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_plane<vertex_position_normal_texture, uint16>(vec2(3), vec2(16), vec3(0,0,1)), string("video_screen"));
		
		input_handlers.push_back(this);
		input_handlers.push_back(fpscamcntrl.get());

		FT_Library lib;
		FT_Init_FreeType(&lib);
	}

	vec4 clear_color;
	void mouse_enterleave_handler(app* _app, bool entered)
	{
		auto t = (test_app*)_app;
		if(entered) {
			t->clear_color = vec4(0.1f, 0.3f, 0.8f, 1.f);
		}
		else {
			t->clear_color = vec4(0.8f, 0.1f, 0.3f, 1.f);
		}
	}

	void resize() override
	{
		cam.update_proj(dev->size());
	}

	void update(float t, float dt) override
	{
		torus_world = rotate(translate(mat4(1), vec3(0.f, 1.4f, 0.f)), t, vec3(.6f, .5f, .4f));
		fpscamcntrl->update(t, dt);
	}

	void render(float t, float dt) override
	{
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, 1024, 1024);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		s.bind();
		s.set_uniform("world", torus_world);
		s.set_uniform("itworld", inverse(transpose(torus_world)));
		s.set_uniform("view_proj", perspectiveFov(radians(45.f), 1024.f, 1024.f, .01f, 1000.f)*lookAt(vec3(0.f, 4.f, -10.f), vec3(0.f), vec3(0.f, 1.f, 0.f)));
		tex.bind(0);
		s.set_uniform("tex", 0);
		torus->draw();
		tex.unbind(0);
		s.unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.f, 0.f, 0.f, 1.f);
		dev->push_render_target(&rtx);
		s.bind();
		s.set_uniform("world", torus_world);
		s.set_uniform("itworld", inverse(transpose(torus_world)));
		s.set_uniform("view_proj", perspectiveFov(radians(45.f), 1024.f, 1024.f, .01f, 1000.f)*lookAt(vec3(0.f, 4.f, -10.f), vec3(0.f), vec3(0.f, 1.f, 0.f)));
		tex.bind(0);
		s.set_uniform("tex", 0);
		torus->draw();
		tex.unbind(0);
		s.unbind();
		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		dev->pop_render_target();
		
		
		//glViewport(0, 0, dev->size().x, dev->size().y);
		s.bind();
		
		s.set_uniform("world", translate(mat4(1), vec3(1.7f, 0.f, 0.f)));
		s.set_uniform("itworld", inverse(transpose(translate(mat4(1), vec3(1.7f, 0.f, 0.f)))));
		s.set_uniform("view_proj", cam.proj()*cam.view());
		//tex.bind(0);
		//s.set_uniform("tex", 0);
		//s.set_texture("tex", tex, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frx);
		screen->draw();
		glBindTexture(GL_TEXTURE_2D, 0);
		//tex.unbind(0);
		
		s.set_uniform("world", translate(mat4(1), vec3(-1.7f, 0.f, 0.f)));
		s.set_uniform("itworld", inverse(transpose(translate(mat4(1), vec3(-1.7f, 0.f, 0.f)))));
		//rtx.bind(0);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, frx);
		//s.set_uniform("tex", 0);
		s.set_texture("tex", rtx, 0);
		screen->draw();
		rtx.unbind(0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		s.unbind();

	}
};

int main()
{
	test_app a;
	a.run();
}