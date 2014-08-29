#pragma once
#pragma once
#include "cmmn.h"
#include "device.h"

namespace gluk
{

	template <typename T>
	struct glSetUniformResolve { static void set(GLint l, const T& v) {} };
#pragma region glSetUniformResolve<X>
	template<>
	struct glSetUniformResolve<float>
	{
		static void set(GLint l, const float& v)
		{
			glUniform1f(l, v);
		}
	};
	template<>
	struct glSetUniformResolve<vec2>
	{
		static void set(GLint l, const vec2& v)
		{
			glUniform2f(l, v.x, v.y);
		}
	};
	template<>
	struct glSetUniformResolve<vec3>
	{
		static void set(GLint l, const vec3& v)
		{
			glUniform3f(l, v.x, v.y, v.z);
		}
	};
	template<>
	struct glSetUniformResolve<vec4>
	{
		static void set(GLint l, const vec4& v)
		{
			glUniform4fv(l, 4, &v[0]);
		}
	};

	template<>
	struct glSetUniformResolve<int>
	{
		static void set(GLint l, const int& v)
		{
			glUniform1i(l, v);
		}
	};
	template<>
	struct glSetUniformResolve<ivec2>
	{
		static void set(GLint l, const ivec2& v)
		{
			glUniform2i(l, v.x, v.y);
		}
	};
	template<>
	struct glSetUniformResolve<ivec3>
	{
		static void set(GLint l, const ivec3& v)
		{
			glUniform3i(l, v.x, v.y, v.z);
		}
	};
	template<>
	struct glSetUniformResolve<ivec4>
	{
		static void set(GLint l, const ivec4& v)
		{
			glUniform4iv(l, 4, &v[0]);
		}
	};

	template<>
	struct glSetUniformResolve<mat2>
	{
		static void set(GLint l, const mat2& v)
		{
			glUniformMatrix2fv(l, 4, false, &v[0][0]);
		}
	};
	template<>
	struct glSetUniformResolve<mat3>
	{
		static void set(GLint l, const mat3& v)
		{
			glUniformMatrix3fv(l, 9, false, &v[0][0]);
		}
	};
	template<>
	struct glSetUniformResolve<mat4>
	{
		static void set(GLint l, const mat4& v)
		{
			glUniformMatrix4fv(l, 16, false, &v[0][0]);
		}
	};
#pragma endregion

	template <typename T>
	class uniform_buffer
	{
		GLuint _buf, _ix, _gi;
		device* _ad;
	public:
		uniform_buffer(device* dev, GLint pid, GLuint block_index)
			: _ix(block_index), _ad(dev)
		{
			if (_ix == GL_INVALID_INDEX)
				throw exception("Invalid uniform buffer index");
			glGenBuffers(1, &_buf);
			glBindBuffer(GL_UNIFORM_BUFFER, _buf);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STREAM_DRAW);
			_gi = dev->alloc_ubbi();
			glBindBufferRange(GL_UNIFORM_BUFFER, _gi, _buf, 0, sizeof(T));
			glUniformBlockBinding(pid, _ix, _gi);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void set_data(const T& value)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, _buf);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &value, GL_DYNAMIC_DRAW);
		}

		propr(GLuint, buffer, { return _buf; });
		propr(GLuint, block_index, { return _ix; });

		~uniform_buffer()
		{
			_ad->free_ubbi(_gi);
			glDeleteBuffers(1, &_buf);
		}
	};

	class shader
	{
	protected:
		GLuint _id, _idvp, _idfp, _idgp;
		map<string, GLint> uniform_index_cache;
	public:
		shader( const datablob<byte>& vs_data, const datablob<byte>& ps_data = datablob<byte>(),
			const datablob<byte>& gs_data = datablob<byte>());
		~shader();

		virtual void bind();
		virtual void unbind();
		virtual void update();

		template <typename T>
		void set_uniform(const string& id, const T& value)
		{
			GLint ix = -1;
			auto ui = uniform_index_cache.find(id);
			if(ui != uniform_index_cache.end())
				ix = ui->second;
			else
			{
				ix = glGetUniformLocation(_id, id.c_str());
				uniform_index_cache[id] = ix;
			}
			glSetUniformResolve<T>::set(ix, value);
		}

		template <typename T>
		//WARNING: This function returns a new UBO each time!
		uniform_buffer<T>& get_uniform_buffer(device* dev, const string& id)
		{
			return uniform_buffer<T>(dev, _id, glGetUniformBlockIndex(_id, id.c_str()));
		}

		propr(GLuint, program_id, { return _id; });
		propr(GLuint, vertex_shader_id, { return _idvp; });
		propr(GLuint, pixel_shader_id, { return _idfp; });
		propr(GLuint, geometry_shader_id, { return _idgp; });
	};
}

