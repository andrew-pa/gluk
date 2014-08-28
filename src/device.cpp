#include "device.h"
#include "render_target.h"

namespace gluk
{
	static void debug_gl_callback(unsigned int source, unsigned int type,
		unsigned int id, unsigned int sev, int len, const char* msg, void* usr)
	{
		ostringstream oss;
		string ssource, stype, ssev;
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             ssource = "GL"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ssource = "GUI"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: ssource = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     ssource = "3rd Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     ssource = "App"; break;
		case GL_DEBUG_SOURCE_OTHER:           ssource = "Other"; break;
		}

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: stype = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: stype = "Decaprecated"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: stype = "Undefined"; break;
		case GL_DEBUG_TYPE_PORTABILITY: stype = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: stype = "Perf"; break;
		case GL_DEBUG_TYPE_OTHER: stype = "Other"; break;
		}

		switch (sev)
		{
		case GL_DEBUG_SEVERITY_HIGH: ssev = "H"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: ssev = "M"; break;
		case GL_DEBUG_SEVERITY_LOW: ssev = "L"; break;
		}

		oss << ssource << "> " << ssev << "[" << stype << "] (" << id << ") " << msg;
		if (sev == GL_DEBUG_SEVERITY_HIGH)
		{
			OutputDebugStringA(("GL error: " + oss.str()).c_str());
			throw exception(("GL error: " + oss.str()).c_str());
		}
		else
			OutputDebugStringA(oss.str().c_str());
	}

	device::device(vec2 s_, GLFWwindow* w_, uint aal)
		: _rtsize(s_), next_uniform_buffer_bind_index(0),
		wnd(w_)
	{
		glfwMakeContextCurrent(wnd);
		glewInit();

#ifdef _DEBUG
		const GLubyte* glVersionString = glGetString(GL_VERSION);
		int glVersion[2] = { -1, -1 };
		glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
		glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
		ostringstream l;
		l << "GL Version: " << glVersion[0] << "." << glVersion[1] << endl;
		l << "GL Vendor: " << glGetString(GL_VENDOR) << endl;
		l << "GL Version String: " << glVersionString << endl;
		OutputDebugStringA(l.str().c_str());
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback((GLDEBUGPROC)&debug_gl_callback, this);
#endif
		rtsk.push(drt = new default_render_target(viewport(_rtsize)));

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(1.f, .5f, 0.f, 1.f);
	}

	void device::resize(vec2 s)
	{
		_rtsize = s;
		drt->mviewport() = viewport(s);
	}

	void device::present()
	{
		glfwSwapBuffers(wnd);
	}

	render_target* device::current_render_target() const
	{
		return rtsk.top();
	}

	void device::pop_render_target()
	{
		if (rtsk.size() == 1) return;
		rtsk.pop();
		update_render_target();
	}

	void device::update_render_target()
	{
		rtsk.top()->ombind(this);
	}

	uint device::alloc_ubbi()
	{
		if(!previously_alloced_ubbi.empty())
		{
			auto x = previously_alloced_ubbi.front();
			previously_alloced_ubbi.pop();
			return x;
		}
		return next_uniform_buffer_bind_index++;
	}

	void device::free_ubbi(uint& ubbi)
	{
		previously_alloced_ubbi.push(ubbi);
		ubbi = -1;
	}
}