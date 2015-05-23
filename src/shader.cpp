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
			oss << "Shader " << shader << " (" << ")" << " error: " << buf;
			OutputDebugStringA(oss.str().c_str());
			throw exception(oss.str().c_str());
		}
	}

	static GLuint compile_shader(GLenum type, const filedatasp fd, GLuint _program)
	{
		GLuint sh = glCreateShader(type);
		string vssd = string(fd->data<char>(), fd->data<char>() + fd->length());
		const GLchar* vsd = (GLchar*)vssd.data();
		const GLint vsl = vssd.size();
		glShaderSource(sh, 1, &vsd, &vsl);
		glCompileShader(sh);
		validate_shader(sh, vssd);
		glAttachShader(_program, sh);
		return sh;
	}

	shader::shader(device* dev, const filedatasp vs_data, const filedatasp ps_data, const filedatasp gs_data)
		: _dev(dev)
	{
		_id = glCreateProgram();
		if (vs_data != nullptr)
			_idvp = compile_shader(GL_VERTEX_SHADER, vs_data, _id);
		if (gs_data != nullptr)
			_idgp = compile_shader(GL_GEOMETRY_SHADER, gs_data, _id);
		if (ps_data != nullptr)
			_idfp = compile_shader(GL_FRAGMENT_SHADER, ps_data, _id);

		glLinkProgram(_id);

		char buf[512];
		GLsizei len = 0;
		glGetProgramInfoLog(_id, 512, &len, buf);
		if (len > 0)
		{
			string sbuf(buf);
			ostringstream oss;
			if(sbuf.find("warning") != sbuf.npos)
			{
				oss << "GL Program warning: " << sbuf << endl;
				OutputDebugStringA(oss.str().c_str());
			}
			else
			{
				oss << "GL Program error: " << sbuf << endl;
				OutputDebugStringA(oss.str().c_str());
				throw exception(oss.str().c_str());
			}
		}
		glerr
	}

	shader::shader(device* dev, const package& pak, const string& vs_path, const string& ps_path, const string& gs_path)
		: _dev(dev)
	{
		_id = glCreateProgram();
		if (!vs_path.empty())
		{
			_idvp = dev->load_shader(vs_path, pak, GL_VERTEX_SHADER);
			glAttachShader(_id, _idvp);
		}
		if (!gs_path.empty())
		{
			_idgp = dev->load_shader(gs_path, pak, GL_GEOMETRY_SHADER);
			glAttachShader(_id, _idgp);
		}
		if (!ps_path.empty())
		{
			_idfp = dev->load_shader(ps_path, pak, GL_FRAGMENT_SHADER);
			glAttachShader(_id, _idfp);
		}

		glLinkProgram(_id);

		char buf[512];
		GLsizei len = 0;
		glGetProgramInfoLog(_id, 512, &len, buf);
		if (len > 0)
		{
			string sbuf(buf);
			ostringstream oss;
			if (sbuf.find("warning") != sbuf.npos)
			{
				oss << "GL Program warning: " << sbuf << endl;
				OutputDebugStringA(oss.str().c_str());
			}
			else
			{
				oss << "GL Program error: " << sbuf << endl;
				OutputDebugStringA(oss.str().c_str());
				throw exception(oss.str().c_str());
			}
		}
		glerr
	}


	void shader::validate()
	{
		glValidateProgram(_id);
		GLint sta;
		glGetProgramiv(_id, GL_VALIDATE_STATUS, &sta);
		//if (sta == GL_FALSE) throw exception("error validating shader");

		char buf[512];
		GLsizei len = 0;
		glGetProgramInfoLog(_id, 512, &len, buf);
		if (len > 0 && !sta)
		{
			string sbuf(buf);
			ostringstream oss;
			if (sbuf.find("warning") != sbuf.npos)
			{
				oss << "GL Program warning: " << sbuf << endl;
				OutputDebugStringA(oss.str().c_str());
			}
			else
			{
				oss << "GL Program error: " << sbuf << endl;
				OutputDebugStringA(oss.str().c_str());
				throw exception(oss.str().c_str());
			}
		}
	}

	void shader::bind()
	{
		glUseProgram(_id);
	}

	/*unbind is stupid. 
	the new methodology is that you don't know what the GL state is so just change and forget
	void shader::unbind()
	{
		glUseProgram(0);
	}*/

	void shader::update()
	{
	}

	shader::~shader()
	{
		glDetachShader(_id, _idvp);
		glDetachShader(_id, _idgp);
		glDetachShader(_id, _idfp);

		_dev->delete_shader(_idfp);
		_dev->delete_shader(_idvp);
		_dev->delete_shader(_idgp);
		
		glDeleteProgram(_id);
	}
}
