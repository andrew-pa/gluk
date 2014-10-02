#pragma once

//turn off stupid warnings
#define _CRT_SECURE_NO_WARNINGS

#include <map>
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
using namespace std;

#define proprw(t, n, gc) inline t& n() gc
#define propr(t, n, gc) inline t n() const gc

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <GL/wglew.h>
#define GLFW
#include <GLFW/glfw3.h>


//deal with stupid windows header and other fools
#undef min
#undef max

namespace gluk
{

	inline float randf() { return ((float)rand() / (float)RAND_MAX); }
	inline float randfn() { return ((randf() * 2) - 1); }

#define array_size(ar) sizeof(ar) / sizeof(ar[0])

//#define check_flag(f, v) (((uint)(v)&(uint)(f))==(uint)(f))

	template <typename flag_type>
	inline bool check_flag(flag_type check_for, flag_type in)
	{
		return ((uint)in&(uint)check_for) == (uint)check_for;
	}

#ifndef WIN32
	typedef unsigned char byte;
#endif

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
		unorm = GL_UNSIGNED_INT,
		snorm = GL_SHORT, 
		floatp = GL_FLOAT, 
		integer = GL_INT, 
		uinteger = GL_UNSIGNED_INT,
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

	//error_code_exception
	// exception the resulted from a error code that is failing
	struct error_code_exception : public exception
	{
	public:
		uint errorcode;
		error_code_exception(uint ec, const string& m = "") : errorcode(ec), exception(m.c_str()) { }
	};

	//datablob<T>
	// pointer to a T along with the T's size, usually file data
	template<typename T>
	struct datablob
	{
		typedef T ElementType;
		datablob() : data(nullptr), length(-1) { }
		datablob(T* d, size_t l) : data(/*new T[l]*/d), length(l)
		{
			//memcpy(data, d, l*sizeof(T));
		}
		datablob(size_t l) : data(new T[l]), length(l) { }
		datablob(const datablob<T>& o)
			: length(o.length)
		{
					data = new T[o.length];
					memcpy(data, o.data, o.length*sizeof(T));
		}

		~datablob()
		{
			if(data != nullptr)
				delete data;
			data = nullptr;
			length = -1;
		}

		inline bool empty() const { return data == nullptr; }

		T* data;
		size_t length;
	};


	//Read in the data contained in filename, put it in to a datablob
	const datablob<byte>& read_data(const wstring& filename);

	//Wrapper for read_data, but adds the executable path on to the file name
	const datablob<byte>& read_data_from_package(_In_ const wstring& filename);

	enum class shader_stage
	{
		pixel_shader, vertex_shader, geometry_shader //... other shaders
	};

	template <int Dim, class T, precision P>
	struct vec_of
	{
	public:
		typedef int x;
	};

	template <class T, precision P>
	struct vec_of<1, T, P>
	{
	public:
		typedef glm::detail::tvec1<T, P> x;
	};
	template <class T, precision P>
	struct vec_of<2, T, P>
	{
	public:
		typedef glm::detail::tvec2<T, P> x;
	};
	template <class T, precision P>
	struct vec_of<3, T, P>
	{
	public:
		typedef glm::detail::tvec3<T, P> x;
	};
	template <class T, precision P>
	struct vec_of<4, T, P>
	{
	public:
		typedef glm::detail::tvec4<T, P> x;
	};

};
