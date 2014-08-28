#pragma once
#include "cmmn.h"

namespace gluk
{

	template <int Dim>
	class texture
	{
	public:
		typedef typename vec_of<Dim, uint, highp>::x size_vec_t;
	protected:
		size_vec_t _size;
		GLuint _txid;
	public:
		texture(size_vec_t s)
			: _size(s)
		{
			glGenTextures(1, &_txid);
		}
		virtual ~texture() 
		{
			glDeleteTextures(1, &_txid);
		}

		propr(GLuint, id, { return _txid; })
	};

	class texture2d : public texture<2>
	{
	public:
		texture2d(const pixel_format& fmt, size_vec_t size_, void* data = nullptr)
			: texture(size_)
		{
			glBindTexture(GL_TEXTURE_2D, _txid);
			glTexImage2D(GL_TEXTURE_2D, 0, fmt.get_gl_format_internal(), size_.x, size_.y, 0,
				fmt.get_gl_format(), fmt.get_gl_type(), data);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	};
}