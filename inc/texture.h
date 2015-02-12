#pragma once
#include "cmmn.h"
#include <gli/gli.hpp>

namespace gluk
{
	namespace detail
	{
		template <int D, int AS>
		struct texture_target_enum_from_dim
		{
			static const GLenum value = GL_INVALID_ENUM;
		};

		template<>
		struct texture_target_enum_from_dim<1, 1>
		{
			static const GLenum value = GL_TEXTURE_1D;
		};
		template<>
		struct texture_target_enum_from_dim<2, 1>
		{
			static const GLenum value = GL_TEXTURE_2D;
		};
		template<>
		struct texture_target_enum_from_dim<3, 1>
		{
			static const GLenum value = GL_TEXTURE_3D;
		};


		template<>
		struct texture_target_enum_from_dim<2, 6>
		{
			static const GLenum value = GL_TEXTURE_CUBE_MAP;
		};
	}

	template <int Dim, int ArraySize = 1>
	class texture
	{
	public:
		typedef typename vec_of<Dim, mediump_uint>::x size_vec_t;
		
	protected:
		size_vec_t _size;
		GLuint _txid;
	public:
		texture(size_vec_t s)
			: _size(s)
		{
			glGenTextures(1, &_txid);
			bind(7);
			min_filter(GL_LINEAR);
			mag_filter(GL_LINEAR);
			ansiotropic_filter(1.f);
			wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
			unbind(7);
		}

		texture(GLuint glid, size_vec_t s)
			: _size(s), _txid(glid) 
		{
		}

		void bind(int slot) const
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, _txid);
		}
		void unbind(int slot) const
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, 0);
		}

		//these functions assume the function is already bound
		void min_filter(GLenum f) const
		{
			glTexParameteri(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, GL_TEXTURE_MIN_FILTER, f);
		}
		void mag_filter(GLenum f) const
		{
			glTexParameteri(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, GL_TEXTURE_MAG_FILTER, f);
		}
		void ansiotropic_filter(float max_ansio) const
		{
			if (!GL_EXT_texture_filter_anisotropic) return;
			glTexParameterf(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_ansio);
		}
		void wrap(GLenum S, GLenum T, GLenum R) const
		{
			glTexParameteri(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, GL_TEXTURE_WRAP_S, S);
			glTexParameteri(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, GL_TEXTURE_WRAP_T, T);
			glTexParameteri(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, GL_TEXTURE_WRAP_R, R);
		}

		operator GLuint()
		{
			return _txid;
		}

		virtual ~texture() 
		{
			glDeleteTextures(1, &_txid);
		}

		propr(GLuint, id, { return _txid; });
		propr(size_vec_t, size, { return _size; });
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
			glerr
		}

		texture2d(const gli::texture2D& tex)
			: texture(tex.dimensions())
		{
			glBindTexture(GL_TEXTURE_2D, _txid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(tex.levels() - 1));
			glTexStorage2D(GL_TEXTURE_2D, GLint(tex.levels()),
				GLenum(gli::internal_format(tex.format())),
				GLsizei(tex.dimensions().x),
				GLsizei(tex.dimensions().y));

			if(gli::is_compressed(tex.format()))
			{
				for (uint lvl = 0; lvl < tex.levels(); ++lvl)
				{
					glCompressedTexSubImage2D(GL_TEXTURE_2D,
						GLint(lvl), 0, 0,
						GLsizei(tex[lvl].dimensions().x),
						GLsizei(tex[lvl].dimensions().y),
						GLenum(gli::internal_format(tex.format())),
						GLsizei(tex[lvl].size()),
						tex[lvl].data());
				}
			}
			else
			{
				for (uint lvl = 0; lvl < tex.levels(); ++lvl)
				{
					glTexSubImage2D(GL_TEXTURE_2D,
						GLint(lvl), 0, 0,
						GLsizei(tex[lvl].dimensions().x),
						GLsizei(tex[lvl].dimensions().y),
						GLenum(gli::external_format(tex.format())),
						GLenum(gli::type_format(tex.format())),
						tex[lvl].data());
				}
			}
			glerr
		}
	};

	namespace detail
	{
		const static GLenum gl_cube_map_faces[] =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};
	}
	class texture_cube : public texture<2, 6> /*this dim template is weird here*/
	{
		
	public:
		texture_cube(const pixel_format& fmt, size_vec_t size_, vector<void*> data = vector<void*>())
			: texture(size_)
		{
			
			glBindTexture(GL_TEXTURE_CUBE_MAP, _txid);
			for (uint i = 0; i < 6; ++i)
			{
				glTexImage2D(gluk::detail::gl_cube_map_faces[i], 0, fmt.get_gl_format_internal(), size_.x, size_.y, 0,
					fmt.get_gl_format(), fmt.get_gl_type(), data.empty() ? nullptr : data[i]);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}

		texture_cube(const gli::textureCube& ctex)
			: texture(ctex.dimensions())
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, _txid);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, GLint(ctex.levels() - 1));

			for (uint i = 0; i < 6; ++i)
			{
				auto& tex = ctex[i];
				glTexStorage2D(gluk::detail::gl_cube_map_faces[i], GLint(tex.levels()),
					GLenum(gli::internal_format(tex.format())),
					GLsizei(tex.dimensions().x),
					GLsizei(tex.dimensions().y));
			

				if (gli::is_compressed(tex.format()))
				{
					for (uint lvl = 0; lvl < tex.levels(); ++lvl)
					{
						glCompressedTexSubImage2D(gluk::detail::gl_cube_map_faces[i],
							GLint(lvl), 0, 0,
							GLsizei(tex[lvl].dimensions().x),
							GLsizei(tex[lvl].dimensions().y),
							GLenum(gli::internal_format(tex.format())),
							GLsizei(tex[lvl].size()),
							tex[lvl].data());
					}
				}
				else
				{
					for (uint lvl = 0; lvl < tex.levels(); ++lvl)
					{
						glTexSubImage2D(gluk::detail::gl_cube_map_faces[i],
							GLint(lvl), 0, 0,
							GLsizei(tex[lvl].dimensions().x),
							GLsizei(tex[lvl].dimensions().y),
							GLenum(gli::external_format(tex.format())),
							GLenum(gli::type_format(tex.format())),
							tex[lvl].data());
					}
				}
			}
		}
	};


}