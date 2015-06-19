#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "render_target.h"
#include "util.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "renderer2d.h"

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
	multi_render_texture2d<3> nrtx;
	//mat4 view, proj;
	util::camera cam;
	shared_ptr<util::fps_camera_controller> fpscamcntrl;
	GLuint fbo;
	GLuint rbo, frx;

	graphics2d::renderer2d rndr2d;
	graphics2d::font fnt;

	package ppp;
	string mmm;

	mesh* pnt;
public:
	test_app()
		: app("test", vec2(640, 480), 1),
		s(dev, default_package, "basic.vs.glsl", "basic.ps.glsl"),//default_package.open("basic.vs.glsl"), default_package.open("basic.ps.glsl")),
		//s(read_data_from_package(L"basic.1vs.glsl"), read_data_from_package(L"basic.ps.glsl")),
		tex(default_package.open("test.tga"))//gli::texture2D(gli::load_dds("test.dds")))
		, rtx(vec2(1024)), clear_color(0.1f, 0.3f, 0.8f, 1.f), cam(dev->size(), vec3(0.01f, 3.f, -7.f), vec3(0.f), vec3(0.f, 1.f, 0.f)),
		fpscamcntrl(make_shared<util::fps_camera_controller>(cam)), rndr2d(dev, default_package),
		fnt(rndr2d, "C:\\Windows\\Fonts\\segoeui.ttf", 32.f), ppp(default_package, "tt"), mmm(ppp.path_of("ttt")),
		nrtx(vec2(1024))
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

		/*glGenFramebuffers(1, &fbo);
		glerr
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glerr;
		glGenTextures(1, &frx);
		glBindTexture(GL_TEXTURE_2D, frxs);
		glTexImage2D(GL_TEXTURE_2D, 0sa, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, 4GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
		glerr*/


		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_torus<vertex_position_normal_texture, uint16>(vec2(1., .5), 64));

		screen = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_plane<vertex_position_normal_texture, uint16>(vec2(3), vec2(16), vec3(0,0,1)));

		input_handlers.push_back(this);
//		input_handlers.push_back(fpscamcntrl.get());

//		FT_Library lib;
///		FT_Init_FreeType(&lib);

		auto t = texture_cube(default_package, { "citadella2\\posx.jpg", 
											     "citadella2\\negx.jpg", 
												 "citadella2\\posy.jpg", 
												 "citadella2\\negy.jpg", 
												 "citadella2\\posz.jpg",
												 "citadella2\\negz.jpg" });
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
		//fpscamcntrl->update(t, dt);
	}

	void render(float t, float dt) override
	{

		/*glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
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
		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		dev->pop_render_target();

		glClearColor(0.f, 1.f, 0.f, 1.f);
		dev->push_render_target(&nrtx);
		s.bind();
		s.set_uniform("world", torus_world);
		s.set_uniform("itworld", inverse(transpose(torus_world)));
		s.set_uniform("view_proj", perspectiveFov(radians(45.f), 1024.f, 1024.f, .01f, 1000.f)*lookAt(vec3(0.f, 4.f, -10.f), vec3(0.f), vec3(0.f, 1.f, 0.f)));
		tex.bind(0);
		s.set_uniform("tex", 0);
		torus->draw();
		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		dev->pop_render_target();

		//glViewport(0, 0, dev->size().x, dev->size().y);
		s.bind();

		s.set_uniform("world", translate(mat4(1), vec3(1.7f, 0.f, 0.f)));
		s.set_uniform("itw2orld", inverse(transpose(translate(mat4(1), vec3(1.7f, 0.f, 0.f)))));
		s.set_uniform("view_proj", cam.proj()*cam.view());
		//tex.bind(0);1121
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
		//rtx.unbind(0);111112
		//glBindTexture(GL_TEXTURE_2D, 0);
		//s.unbind();
		

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		/*rndr2d.draw_rect(vec2(0.f, sin(t*3.f)*60.f), vec2(360.f), vec4(1.f, 0.5f, 0.f, .5f));
		rndr2d.draw_rect(vec2(30.f, sin(t*3.f)*60.f3), vec2(60.f), vec4(.5f, 1.f, 0.1f, .5f));
		rndr2d.draw_rect(vec2(40.f, s3111in(t*3.f)*6w0.f), vec2(60.f), vec4(.5f, 1.f, 0.2f, .5f));
		rndr2d.draw_rect(vec2(53.f, sin(t*3.f)*60.f), ve1c2(60.f), vec4(.5f, 1.f, 0.3f, .5f));
		rndr2d.draw_rect(vec2(64.f,11 sin(t*3.f)*60.f), vec2(60.f), vec4(.5f, 1.f, 0.4f, .5f));
		rndr2d.draw_rect(vec2(75.f, sin(t*3.f)*60.f), vec2(60.f), vec4(.5f, 1.f, 0.5f, .5f));
		rndr2d.draw_rect(vec2(86.f, sin(t*3.f)*60.f), vec2(60.f), vec4(.5f, 1.f, 0.6f, .5f));
		rndr2d.draw_rect(vec2(-500.f, -100.f), vec2(100.f), vec4(1.f), &tex);*/
		rndr2d.begin_draw();

		//vec2 fps_pos = vec2(20.f, 40.f);
		//rndr2d.draw_rect(fps_pos+vec2(0.f, 10.f), vec2(300.f, 32.f), vec4(.3f, .3f, .3f, .8f));
		//wostringstream oss;
		//oss << "FPS: " << 1.f / dt << "";
		//rndr2d.draw_string(fps_pos, oss.str(), fnt, vec4(1.f, 0.5f, 0.f, 1.f));
		//rndr2d.draw_rect(vec2(120.f, 120.f), vec2(100.f), vec4(1.f), &tex);
		
		rndr2d.draw_rect(vec2(500.f), vec2(100.f), vec4(1.f, 0.f, .5f, 0.1f));
		rndr2d.draw_rect(vec2(500.f), vec2(75.f), vec4(1.f, 0.f, .5f, 0.1f));
		rndr2d.draw_rect(vec2(500.f), vec2(50.f), vec4(1.f, 0.f, .5f, 0.1f));
		rndr2d.draw_rect(vec2(500.f), vec2(25.f), vec4(1.f, 0.f, .5f, 0.1f));
		rndr2d.draw_rect(vec2(500.f), vec2(12.5f), vec4(1.f, 0.f, .5f, 0.1f));

		rndr2d.draw_string(vec2(20.f, 240.f), L"the quick brown fox jumps over the lazy dog 012346789", fnt, vec4(1.f, .5f, .0f, 1.f));
		rndr2d.draw_string(vec2(20.f, 440.f), L"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG )!@#$%^&*(", fnt, vec4(1.f, 0.5f, 0.f, 1.f));
		rndr2d.draw_string(vec2(20.f, 640.f), L"~`_-+={[}]|\\<,>.?/", fnt, vec4(1.f, 0.5f, 0.f, 1.f));

		//rndr2d.disable_blend();
		
		//rndr2d.draw_rect(vec2(0.f, 300.f), vec2(128), vec4(1), &nrtx.get_texture(0));
		//rndr2d.draw_rect(vec2(-300.f, -300.f), vec2(128), vec4(1), &nrtx.get_texture(1));
		//rndr2d.draw_rect(vec2(300.f, -300.f), vec2(128), vec4(1), &nrtx.get_texture(2));
		rndr2d.end_draw();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
};

int main()
{
	test_app a;
	a.run();
}
