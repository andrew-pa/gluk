#include "device.h"
#include "render_target.h"
#include "shader.h"

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
			throw rexception(("GL error: " + oss.str()).c_str());
		}
		else
			OutputDebugStringA(oss.str().c_str());
	}

	device::device(vec2 s_, GLFWwindow* w_, uint aal)
		: _rtsize(s_), next_uniform_buffer_bind_index(0),
		wnd(w_)
	{
		glerr
		glfwMakeContextCurrent(wnd);
		glerr
		GLenum err = glewInit();
		if(err != GLEW_OK)
		{
			OutputDebugString(L"GLEW Error: ");
			OutputDebugStringA((char*)glewGetErrorString(err));
			OutputDebugString(L"\n");
		}
		glerr
#ifdef _DEBUG
		const GLubyte* glVersionString = glGetString(GL_VERSION);
		int glVersion[2] = { -1, -1 };
		glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
		glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
		glerr
		ostringstream l;
		l << "GL Version: " << glVersion[0] << "." << glVersion[1] << endl;
		l << "GL Vendor: " << glGetString(GL_VENDOR) << endl;
		l << "GL Version String: " << glVersionString << endl;
		OutputDebugStringA(l.str().c_str());
		glerr
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback((GLDEBUGPROC)&debug_gl_callback, this);
		glerr
#endif
		rtsk.push(drt = new default_render_target(viewport(_rtsize)));
		glerr
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glerr
	}

	void device::resize(vec2 s)
	{
		_rtsize = s;
		drt->mviewport() = viewport(s);
		update_render_target(false);
	}

	void device::present()
	{
		glfwSwapBuffers(wnd);
	}

	render_target* device::current_render_target() const
	{
		return rtsk.top();
	}

	void device::push_render_target(render_target* rt, bool clear)
	{
		rtsk.push(rt);
		update_render_target(clear);
	}

	void device::pop_render_target(bool clear)
	{
		if (rtsk.size() == 1) return;
		rtsk.pop();
		update_render_target(clear);
	}

	void device::update_render_target(bool c)
	{
		rtsk.top()->ombind(this);
		if (c) rtsk.top()->clear();
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

	static GLuint compile_shader(const string& path, GLenum type, const package& pak)
	{
		GLuint sh = glCreateShader(type);
		auto fd = pak.open(path);
		string vssd = string(fd->data<char>(), fd->data<char>() + fd->length());
		//preprocessor
		auto ppd = vssd.find_first_of("#");
		while(ppd != string::npos) {
			string cmd = "";
			auto line_begin = ppd;
			ppd++;
			while (vssd[ppd] != ' ') cmd += vssd[ppd++];
			auto line_end = vssd.find_first_of('\n', line_begin);
			if(cmd == "include") {
				auto first_quote = vssd.find_first_of('"', ppd);
				auto last_quote = vssd.find_first_of('"', first_quote + 1);
				if(first_quote == string::npos || last_quote == string::npos)
					throw;
				string path = vssd.substr(first_quote + 1, last_quote-first_quote-1);
				ostringstream oss;
				oss << "Including: " << path << endl;
				OutputDebugStringA(oss.str().c_str());
				auto nfd = pak.open(path);
				string incssd = string(nfd->data<char>(), nfd->data<char>() + nfd->length());
				incssd.append("\n#line 0");
				vssd.replace(line_begin, line_end - line_begin, incssd);
			}
			ppd = vssd.find_first_of("#", ppd);
		}
		//------------
	
		const GLchar* vsd = (GLchar*)vssd.data();
		const GLint vsl = vssd.size();
		glShaderSource(sh, 1, &vsd, &vsl);
		glCompileShader(sh);
		GLint success = 0;
		glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
		if(success == GL_FALSE) {
			GLint log_size = 0;
			glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &log_size);
			string log(log_size, ' ');
			glGetShaderInfoLog(sh, log_size, &log_size, (GLchar*)log.data());
			ostringstream oss;
			oss << "Shader " << sh << " [@ " << path << "]" << " failed to compile!" << endl;
			oss << "Compilation Log:" << endl << log << endl;
			OutputDebugStringA(oss.str().c_str());
			cout << oss.str();
	//		glDeleteShader(sh);
	//		throw rexception(oss.str().c_str());
		}
		return sh;
	}

	GLint device::load_shader(const string& path, const package& pak, GLenum type)
	{
		auto shc = shader_cashe.find(path);
		if(shc == shader_cashe.end())
		{
			shader_cashe[path] = tuple<GLint, int>(compile_shader(path, type, pak), 1);
			return get<0>(shader_cashe[path]);
		}
		else
		{
			get<1>(shc->second)++;
			return get<0>(shc->second);
		}
	}

	void device::delete_shader(GLint id)
	{
		auto s = find_if(shader_cashe.begin(), shader_cashe.end(), [&](pair<string,tuple<GLint, int>> v)
		{
			return get<0>(v.second) == id;
		});
		if(s != shader_cashe.end())
		{
			get<1>(s->second)--;
			if(get<1>(s->second) <= 0)
			{
				glDeleteShader(get<0>(s->second));
				shader_cashe.erase(s);
			}
		}
		else
		{
			glDeleteShader(id);//was not in cashe
		}
	}
}
