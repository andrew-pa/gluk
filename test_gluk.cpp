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
	mesh* sphere;
	shader s;
	texture2d tex;
	//mat4 view, proj;
	util::perspective_camera cam;
	shared_ptr<util::fps_camera_controller> fpscamcntrl;

	shader ns;
	depth_render_texture2d* drtx;

	graphics2d::renderer2d rndr2d;
	graphics2d::font fnt;

	mesh* pnt;
public:
	test_app()
		: app("test", vec2(640, 480), 1),
		s(dev, default_package, "basic.vs.glsl", "basic.ps.glsl"), ns(dev, default_package, "basic.vs.glsl"),//default_package.open("basic.vs.glsl"), default_package.open("basic.ps.glsl")),
		//s(read_data_from_package(L"basic.1vs.glsl"), read_data_from_package(L"basic.ps.glsl")),
		tex(default_package.open("test.bmp"))//gli::texture2D(gli::load_dds("test.dds")))
		, clear_color(0.1f, 0.3f, 0.8f, 1.f), cam(dev->size(), vec3(0.01f, 3.f, -7.f), vec3(0.f), vec3(0.f, 1.f, 0.f)),
		fpscamcntrl(make_shared<util::fps_camera_controller>(cam)), rndr2d(dev, default_package),
		fnt(rndr2d, "C:\\Windows\\Fonts\\segoeui.ttf", 32.f), drtx(new depth_render_texture2d(viewport(dev->size())))
	{

		util::perspective_camera cam{ vec2(640, 480), vec3(0.f, 0.f, -5.f), vec3(0.f), vec3(0.f, 1.f, 0.f), radians(20.f) };
		auto f = cam.frustrum();
		wostringstream woss;
		for (int i = 0; i < 6; ++i) {
			woss << "vec4 p" << i << " = vec4(" << f.normal[i].x << ", " << f.normal[i].y << ", " << f.normal[i].z << ", " << f.dist[i] << ");" << endl;
		}
		OutputDebugString(woss.str().c_str());

		glerr
		tex.bind();
		glGenerateMipmap(GL_TEXTURE_2D);
		tex.mag_filter(GL_LINEAR_MIPMAP_LINEAR);
		tex.min_filter(GL_LINEAR_MIPMAP_LINEAR);
		tex.ansiotropic_filter(16.f);
		tex.wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		tex.unbind();

		auto sym = generate_sphere<vertex_position_normal_texture, uint16>(1.f, 16, 16);
		vector<vec3> pos;
		vector<vec3> nor;
		vector<vec2> tex;
		for(const auto& v : sym.vertices) {
			pos.push_back(v.pos);
			nor.push_back(v.norm);
			tex.push_back(v.tex);
		}
		sphere = new multistream_mesh<uint16, vec3, vec3, vec2>(sym.indices, {
			multistream_mesh_stream_desc(pos.data(), pos.size()*sizeof(vec3), 3, GL_FLOAT),
			multistream_mesh_stream_desc(nor.data(), nor.size()*sizeof(vec3), 3, GL_FLOAT),
			multistream_mesh_stream_desc(tex.data(), tex.size()*sizeof(vec2), 2, GL_FLOAT),
		});



		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			generate_torus<vertex_position_normal_texture, uint16>(vec2(1., .5), 64));

		input_handlers.push_back(this);
		input_handlers.push_back(fpscamcntrl.get());
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

		if (drtx) delete drtx;
		drtx = new depth_render_texture2d(viewport(dev->size()));
	}

	void update(float t, float dt) override
	{
		torus_world = rotate(translate(mat4(1), vec3(3.f, 1.4f, 0.f)), t, vec3(.6f, .5f, .4f));
		fpscamcntrl->update(t, dt);
	}

	void render(float t, float dt) override
	{
		dev->push_render_target(drtx);
		ns.bind();
		ns.set_uniform("world", torus_world);
		ns.set_uniform("view_proj", cam.proj()*cam.view());
		torus->draw();
		dev->pop_render_target();

		s.bind();
		s.set_uniform("world", torus_world);
		s.set_uniform("view_proj", cam.proj()*cam.view());
		s.set_texture("tex", tex, 2);
		torus->draw();

		s.set_uniform("world", mat4(1.f));
		sphere->draw();
		tex.unbind();

		rndr2d.begin_draw();

		const vec2 fps_pos = vec2(15.f, 30.f);
		wchar_t fps_str[15];
#ifdef _MSC_VER > 0
		swprintf(fps_str, 15, L"FPS: %f", 1.f / dt);
#else
		wprintf(fps_str, L"FPS: %f", 1.f / dt);
#endif
		rndr2d.draw_string(fps_pos, fps_str, fnt, vec4(1.f, 0.5f, 0.f, 1.f));
		rndr2d.draw_rect(vec2(500.f), dev->size()*.22f, vec4(1.f), &tex);
	
		rndr2d.draw_string(vec2(20.f, 640.f), L"the quick brown fox\njumps over the\nlazy dog", fnt, vec4(1.f, .5f, .0f, 1.f));
		//rndr2d.draw_string(vec2(20.f, 440.f), L"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG )!@#$%^&*(", fnt, vec4(1.f, 0.5f, 0.f, 1.f));
		//rndr2d.draw_string(vec2(20.f, 640.f), L"~`_-+={[}]|\\<,>.?/", fnt, vec4(1.f, 0.5f, 0.f, 1.f));

		rndr2d.end_draw();
	}
};

int main()
{
	test_app a;
	a.run();
}
