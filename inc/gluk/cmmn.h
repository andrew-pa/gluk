#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <stack>
#include <queue>
#include <map>
#include <iostream>
#include <fstream>
#include <functional>
#include <exception>
#include <algorithm>
#include <chrono>
#include <thread>
#include <memory>
#ifndef MINGW
#include <codecvt>
#endif
using namespace std;

#define proprw(t, n, gc) inline t& n() gc
#define propr(t, n, gc) inline t n() const gc

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/integer.hpp>
using namespace glm;

#ifdef WIN32
//this is the path delimiter (as in ~/stuff or C:\Data)
#define PATH_DELI "\\"
#define NOMINMAX
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <GL/wglew.h>
#define GLFW
#include <GLFW/glfw3.h>

extern "C"
{
	#include <SOIL.h>
}

#define glerr
//{auto err = glGetError(); if(err != 0) { char s[16]; sprintf(s, "GL Error: %08X\n", err); OutputDebugStringA(s); throw; }}

#undef min
#undef max

namespace gluk
{
	inline float randf() { return ((float)rand() / (float)RAND_MAX); }
	inline float randfn() { return ((randf() * 2) - 1); }

#define array_size(ar) sizeof(ar) / sizeof(ar[0])

	template <typename flag_type>
	inline bool check_flag(flag_type check_for, flag_type in)
	{
		return ((uint)in&(uint)check_for) == (uint)check_for;
	}

	typedef unsigned char byte;

	enum class pixel_components
	{
		r = GL_RED,
		rg = GL_RG,
		rgb = GL_RGB,
		rgba = GL_RGBA,
		depth = GL_DEPTH_COMPONENT,
		depth_stencil = GL_DEPTH_STENCIL,
	};

	enum class pixel_type
	{
		unorm,
		snorm,
		floatp,
		integer,
		uinteger,
	};

	struct pixel_format
	{
		pixel_components comp;
		pixel_type type;
		byte depth;
		pixel_format(pixel_components c, pixel_type t, byte d)
			: comp(c), type(t), depth(d) {}
		GLenum get_gl_format_internal() const;
		GLenum get_gl_type() const;
		GLenum get_gl_format() const;
	};

	enum class texture_dimension : uint
	{
		texture_1d = 0,
		texture_2d = 1,
		texture_3d = 2,
		texture_cube = 3,
	};

	inline GLuint cube_map_face_for_index(uint i)
	{
		static const GLuint face[] =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};
		return face[i];
	}
	
	//error_code_exception
	// exception the resulted from a error code that is failing
	struct error_code_exception : public runtime_error
	{
	public:
		uint errorcode;
		error_code_exception(uint ec, const string& m = "") : errorcode(ec), runtime_error(m) { }
	};

	inline vector<byte> make_data_vector(const byte* d, size_t s)
	{
		return vector<byte>(d, d + s);
	}

	inline wstring s2ws(const string& str) {
		wstring ws(str.size(), L' ');
		ws.resize(mbstowcs(&ws[0], str.c_str(), str.size()));
		return ws;
	}
	inline string ws2s(const wstring& str) {
		string s(str.size(), ' ');
		s.resize(wcstombs(&s[0], str.c_str(), str.size()));
		return s;
	}


	class filedata;
	class package
	{
		wstring _base_path;
	public:
		package(const string& p) : _base_path(s2ws(p)){}
		package(const package& p, const string& r) : _base_path(p._base_path+s2ws(r)) {}
		//gets the executable directory for the base path (system dependent)
		package(char);
		shared_ptr<filedata> open(const string& f, bool readwrite = false) const;

		string path_of(const string& p) const { return ws2s(_base_path) + p; }


		proprw(wstring, base_path, { return _base_path; })
		propr(wstring, base_path, { return _base_path; })
		friend class filedata;
	};

	//the executable directory
	const static package default_package = package(42); /*number is irreverent*/

	class filedata
	{
	protected:
		void* _b;
		long long _ln;
	public:
		inline void * const rdata() { return _b; }
		template <typename T>
		inline T * const data() { return (T*)_b; }
		inline const void * const rdata() const { return _b; }
		template <typename T>
		inline const T * const data() const { return (T*)_b; }

		inline long long length() const { return _ln; }

		inline bool empty() const { return _b != nullptr; }

		~filedata();

		friend class package;

	};
	typedef shared_ptr<filedata> filedatasp;

	class memory_filedata : public filedata
	{
	public:
		memory_filedata(void*const d, long long s)
		{
			_b = d; _ln = s;
		}
		~memory_filedata()
		{
			delete _b;
		}
	};

	class constmem_filedata : public filedata
	{
	public:
		constmem_filedata(void * const d, long long s)
		{
			_b = d;
			_ln = s;
		}
	};


	enum class shader_stage
	{
		pixel_shader, vertex_shader, geometry_shader //... other shaders
	};

	template <int Dim, class T>
	struct vec_of
	{
	public:
		typedef int x;
	};

	template <class T>
	struct vec_of<1, T>
	{
	public:
		typedef tvec1<T> x;
	};
	template <class T>
	struct vec_of<2, T>
	{
	public:
		typedef tvec2<T> x;
	};
	template <class T>
	struct vec_of<3, T>
	{
	public:
		typedef tvec3<T> x;
	};
	template <class T>
	struct vec_of<4, T>
	{
	public:
		typedef tvec4<T> x;
	};
};
