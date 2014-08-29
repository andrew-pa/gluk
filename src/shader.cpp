#include "shader.h"

namespace gluk
{
	static void validate_shader(GLuint shader, const string& fn)
	{
		char buf[512];
		GLsizei len = 0;
		//OutputDebugStringA(fn.c_str());
		glGetShaderInfoLog(shader, 512, &len, buf);
		if (len > 0)
		{
			ostringstream oss;
			oss << "Shader " << shader << " (" << fn << ")" << " error: " << buf;
			OutputDebugStringA(oss.str().c_str());
			throw exception(oss.str().c_str());
		}
	}

	static GLuint compile_shader(GLenum type, const datablob<byte>& fdata, const string& defs)
	{
		GLuint sh = glCreateShader(type);
		string vssd = string((char*)fdata.data, fdata.length + (char*)fdata.data);
		auto tw = vssd.find_first_of("~~~~", 0);
		if (tw != vssd.npos) 
		{
			vssd.replace(tw, 4, "");
			vssd.insert(tw, defs);
		}
		const GLchar* vsd = (GLchar*)vssd.data();
		const GLint vsl = vssd.size();
		//OutputDebugStringA(vsd);
		glShaderSource(sh, 1, &vsd, &vsl);
		glCompileShader(sh);
		validate_shader(sh, vsd);
		return sh;
	}

	shader::shader(const datablob<byte>& vs_data, const datablob<byte>& ps_data, const datablob<byte>& gs_data)
	{
		if(!vs_data.empty())
		{
			_idvp = compile_shader(GL_VERTEX_SHADER, vs_data, "");
		}

		if (!gs_data.empty())
		{
			_idgp = compile_shader(GL_GEOMETRY_SHADER, gs_data, "");
		}

		if(!ps_data.empty())
		{
			string defs = "";
			if (!gs_data.empty()) defs += "#define IN_FROM_GS";
			else if (!vs_data.empty()) defs += "#define IN_FROM_VS";
			_idfp = compile_shader(GL_FRAGMENT_SHADER, ps_data, defs);
		}

		_id = glCreateProgram();
		if(!vs_data.empty())
			glAttachShader(_id, _idvp);
		if(!ps_data.empty())
			glAttachShader(_id, _idfp);
		if (!gs_data.empty())
			glAttachShader(_id, _idgp);

		glLinkProgram(_id);

		{
			char buf[512];
			GLsizei len = 0;
			glGetProgramInfoLog(_id, 512, &len, buf);
			if (len > 0)
			{
				ostringstream oss;
				oss << "GL Program error: " << buf << endl;
				throw exception(oss.str().c_str());
			}
			glValidateProgram(_id);
			GLint sta;
			glGetProgramiv(_id, GL_VALIDATE_STATUS, &sta);
			//if (sta == GL_FALSE) throw exception("error validating shader");
		}

	}


	void shader::bind()
	{
		glUseProgram(_id);
	}

	void shader::unbind()
	{
		glUseProgram(0);
	}

	void shader::update()
	{
	}

	shader::~shader()
	{
		glDetachShader(_id, _idvp);
		glDetachShader(_id, _idgp);
		glDetachShader(_id, _idfp);

		glDeleteShader(_idfp);
		glDeleteShader(_idvp);
		glDeleteShader(_idgp);
		glDeleteProgram(_id);
	}
}
