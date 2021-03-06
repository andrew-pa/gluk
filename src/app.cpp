#include "app.h"
#include "render_target.h"
#include "timer.h"

namespace gluk
{
	app::app(const string& title, vec2 winsize, uint aa_samples, const uvec4& colorbits, uvec2 dsbits, function<void()> apply_window_hints)
	{
		glerr
		if (!glfwInit()) throw runtime_error("GLFW init failed!");
		glfwSetErrorCallback([](int ec, const char* em) { char s[64]; sprintf(s, "GLFW error: %s, (error code: %08X)", em, ec);  OutputDebugStringA(s); DebugBreak(); });
		if(aa_samples >= 1) glfwWindowHint(GLFW_SAMPLES, aa_samples);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		apply_window_hints();
		
		glfwWindowHint(GLFW_RED_BITS, colorbits.r);
		glfwWindowHint(GLFW_GREEN_BITS, colorbits.g);
		glfwWindowHint(GLFW_BLUE_BITS, colorbits.b);
		glfwWindowHint(GLFW_ALPHA_BITS, colorbits.a);
		
		glfwWindowHint(GLFW_DEPTH_BITS, dsbits.x);
		glfwWindowHint(GLFW_STENCIL_BITS, dsbits.y);		
		glerr

		wnd = glfwCreateWindow((int)floor(winsize.x), (int)floor(winsize.y), title.c_str(), nullptr, nullptr);
		if(!wnd)
		{
			glfwTerminate();
			throw runtime_error("GLFW window creation failed");
		}
	
		glfwSetWindowUserPointer(wnd, this);

		glfwSetWindowSizeCallback(wnd, [](GLFWwindow* wnd, int w, int h)
		{
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			t->dev->resize(vec2(w, h));
			t->resize();
			t->make_resolution_dependent_resources(t->dev->size());
		});

		glfwSetKeyCallback(wnd, [](GLFWwindow* wnd, int key, int scancode, int action, int mods) 
		{
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			t->key_down(key, (key_action)action, (key_mod)mods);
			for(auto& ih : t->input_handlers) {
				ih->key_handler(t, key, (input_action)action, (input_mod)mods);
			}
		});

		glfwSetCharModsCallback(wnd, [](GLFWwindow* wnd, uint cp, int mods) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->char_handler(t, cp, (input_mod)mods);
			}
		});
		
		glfwSetCursorPosCallback(wnd, [](GLFWwindow* wnd, double x, double y) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->mouse_position_handler(t, vec2(x, y));
			}
		});

		glfwSetCursorEnterCallback(wnd, [](GLFWwindow* wnd, int entered) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->mouse_enterleave_handler(t, entered>0);
			}
		});

		glfwSetMouseButtonCallback(wnd, [](GLFWwindow* wnd, int button, int action, int mods) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->mouse_button_handler(t, (mouse_button)button, (input_action)action, (input_mod)mods);
			}
		});

		glerr
		dev = new device(winsize, wnd, aa_samples);
		glerr
	}

	void app::run(bool pdfps)
	{
		const float target_delta_time = 1.f / 120.f;

		uint fc = 0;
		float ft = 0.f;
		tm.reset();
		while(!glfwWindowShouldClose(wnd))
		{
			tm.update();
			update(tm.time(), tm.delta_time());
			dev->current_render_target()->clear(); //dev->update_render_target(true);
			render(tm.time(), tm.delta_time());
			dev->present();
			fc++;
			ft += tm.delta_time();
			if(ft >= 1.f && pdfps)
			{
				float fps = (float)fc;
				float mpf = 1000.f / fps;
				ostringstream wos;
				wos << "FPS: " << fps << " MPF:" << mpf << endl;
				OutputDebugStringA(wos.str().c_str());
				fc = 0;
				ft = 0.f;
			}
			glfwPollEvents();
			if(tm.delta_time() < target_delta_time)
			{
				auto missing_delta = target_delta_time - tm.delta_time();
				if(missing_delta > 0.f)
					Sleep(chrono::milliseconds((long)ceil(missing_delta)).count());
			}
		}
	}

	app::~app()
	{
		glfwDestroyWindow(wnd);
		glfwTerminate();
	}
}
