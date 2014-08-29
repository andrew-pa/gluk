#pragma once
#include "cmmn.h"

namespace gluk
{
	namespace detail
	{
		template <int D>
		struct texture_target_enum_from_dim
		{
			static const GLenum value = GL_INVALID_ENUM;
		};

		template<>
		struct texture_target_enum_from_dim<1>
		{
			static const GLenum value = GL_TEXTURE_1D;
		};
		template<>
		struct texture_target_enum_from_dim<2>
		{
			static const GLenum value = GL_TEXTURE_2D;
		};
		template<>
		struct texture_target_enum_from_dim<3>
		{
			static const GLenum value = GL_TEXTURE_3D;
		};
	}

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

		void bind(int slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(detail::texture_target_enum_from_dim<Dim>::value, _txid);
		}
		void unbind(int slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(detail::texture_target_enum_from_dim<Dim>::value, 0);
		}

		//these functions assume the function is already bound
		void min_filter(GLenum f)
		{
			glTexParameteri(detail::texture_target_enum_from_dim<Dim>::value, GL_TEXTURE_MIN_FILTER, f);
		}
		void mag_filter(GLenum f)
		{
			glTexParameteri(detail::texture_target_enum_from_dim<Dim>::value, GL_TEXTURE_MAG_FILTER, f);
		}
		void ansiotropic_filter(float max_ansio)
		{
			if (!GL_EXT_texture_filter_anisotropic) return;
			glTexParameterf(detail::texture_target_enum_from_dim<Dim>::value, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_ansio);
		}
		void wrap(GLenum S, GLenum T, GLenum R)
		{
			glTexParameteri(detail::texture_target_enum_from_dim<Dim>::value, GL_TEXTURE_WRAP_S, S);
			glTexParameteri(detail::texture_target_enum_from_dim<Dim>::value, GL_TEXTURE_WRAP_T, T);
			glTexParameteri(detail::texture_target_enum_from_dim<Dim>::value, GL_TEXTURE_WRAP_R, R);
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