#include "app.h"
#include "timer.h"

namespace gluk
{
	app::app(const string& title, vec2 winsize, uint aa_samples, uvec4 colorbits, uvec2 dsbits, function<void()> apply_window_hints)
	{
		if (!glfwInit()) throw exception("GLFW init failed!");
		if(aa_samples >= 1) glfwWindowHint(GLFW_SAMPLES, aa_samples);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		apply_window_hints();
		
		glfwWindowHint(GLFW_RED_BITS, colorbits.r);
		glfwWindowHint(GLFW_GREEN_BITS, colorbits.g);
		glfwWindowHint(GLFW_BLUE_BITS, colorbits.b);
		glfwWindowHint(GLFW_ALPHA_BITS, colorbits.a);
		
		glfwWindowHint(GLFW_DEPTH_BITS, dsbits.x);
		glfwWindowHint(GLFW_STENCIL_BITS, dsbits.y);		
	
		wnd = glfwCreateWindow((int)floor(winsize.x), (int)floor(winsize.y), title.c_str(), nullptr, nullptr);
		if(!wnd)
		{
			glfwTerminate();
			throw exception("GLFW window creation failed");
		}

		glfwSwapInterval(1);
	
		glfwSetWindowUserPointer(wnd, this);

		glfwSetWindowSizeCallback(wnd, [](GLFWwindow* wnd, int w, int h)
		{
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			t->dev->resize(vec2(w, h));
			t->resize();
		});

		glfwSetKeyCallback(wnd, [](GLFWwindow* wnd, int key, int scancode, int action, int mods) 
		{
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			t->key_down(key, (key_action)action, (key_mod)mods);
		});
		
		dev = new device(winsize, wnd, aa_samples);
	}

	void app::run()
	{
		const float target_delta_time = 1.f / 120.f;

		uint fc = 0;
		float ft = 0.f;
		tm.reset();
		while(!glfwWindowShouldClose(wnd))
		{
			tm.update();
			update(tm.time(), tm.delta_time());
			dev->update_render_target();
			render(tm.time(), tm.delta_time());
			dev->present();
			fc++;
			ft += tm.delta_time();
			if(ft >= 1.f)
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
					this_thread::sleep_for(chrono::milliseconds((long)ceil(missing_delta)));
			}
		}
	}

	app::~app()
	{
		glfwDestroyWindow(wnd);
		glfwTerminate();
	}
}
