#pragma once
#include "cmmn.h"
//#define GLI //is for scrubs
#ifdef GLI
#include <gli/gli.hpp>
#endif

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

		void get_size_from_opengl()
		{
			bind(16);
			glGetTexLevelParameteriv(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, 0, GL_TEXTURE_WIDTH, (GLint*)&_size[0]);
			if (_size.length() > 1) glGetTexLevelParameteriv(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, 0, GL_TEXTURE_HEIGHT, (GLint*)&_size[1]);
			if (_size.length() > 2) glGetTexLevelParameteriv(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, 0, GL_TEXTURE_DEPTH, (GLint*)&_size[2]);
		}
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
		}

		texture(GLuint glid, size_vec_t s)
			: _size(s), _txid(glid) 
		{
			bind(7);
			min_filter(GL_LINEAR);
			mag_filter(GL_LINEAR);
			ansiotropic_filter(1.f);
			wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}

		texture(GLuint glid)
			: _txid(glid)
		{
			get_size_from_opengl();
			bind(7);
			min_filter(GL_LINEAR);
			mag_filter(GL_LINEAR);
			ansiotropic_filter(1.f);
			wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}

		void bind(int slot) const
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(detail::texture_target_enum_from_dim<Dim, ArraySize>::value, _txid);
		}
		static void unbind(int slot)
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
		GLuint load_texture_with_SOIL(const filedatasp data)
		{
/*			unsigned char* d = new unsigned char[data->length()];
			memcpy(d, data->data<void>(), data->length());
			auto r = SOIL_load_OGL_texture_from_memory(d, data->length(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,0);
			delete d;
			return r;*/
		}
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

#ifdef GLI
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
#endif

		texture2d(const filedatasp data)
			: texture(vec2(0))
		{
			int chnl = 0;
			
			auto b = SOIL_load_image_from_memory(data->data<unsigned char>(), data->length(), 
				(int*)&_size[0], (int*)&_size[1], &chnl, 0);
			GLint fmt = GL_R;
			switch (chnl)
			{
			case 1: fmt = GL_R;		break;
			case 2: fmt = GL_RG;	break;
			case 3: fmt = GL_RGB;	break;
			case 4: fmt = GL_RGBA;	break;
			}

			glBindTexture(GL_TEXTURE_2D, _txid);
			glTexImage2D(GL_TEXTURE_2D, 0, fmt, _size.x, _size.y, 0,
				fmt, GL_UNSIGNED_BYTE, b);
			glBindTexture(GL_TEXTURE_2D, 0);

			//_txid = load_texture_with_SOIL(data);
			//get_size_from_opengl();
		}

		texture2d(GLuint id)
			: texture(id) {}
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
			wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		texture_cube(const package& pak, initializer_list<string> imgs)
			: texture(vec2(0))
		{
			int chnl = 0;

			glBindTexture(GL_TEXTURE_CUBE_MAP, _txid);
			auto img = imgs.begin();
			for (int i = 0; i < 6; ++i) {
				auto data = pak.open(*img++);
				auto b = SOIL_load_image_from_memory(data->data<unsigned char>(), data->length(),
					(int*)&_size[0], (int*)&_size[1], &chnl, 0);
				GLint fmt = GL_R;
				switch (chnl)
				{
				case 1: fmt = GL_R;		break;
				case 2: fmt = GL_RG;	break;
				case 3: fmt = GL_RGB;	break;
				case 4: fmt = GL_RGBA;	break;
				}

				glTexImage2D(detail::gl_cube_map_faces[i], 0, fmt, _size.x, _size.y, 0,
					fmt, GL_UNSIGNED_BYTE, b);
			}
			wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		}
#ifdef GLI
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
#endif
	};


}