#pragma once
#include "cmmn.h"
#include "device.h"
#include "texture.h"

namespace gluk
{

	struct viewport
	{
		vec2 offset;
		vec2 size;
		float min_depth;
		float max_depth;
		viewport(vec2 si, vec2 os = vec2(0.), float md = 0.f, float xd = 1.f)
			: offset(os), size(si), min_depth(md), max_depth(xd){}
		
	};

	class render_target
	{
	public:
		virtual void ombind(device* dev) = 0;
		virtual viewport& mviewport() = 0;
	};

	class default_render_target : public render_target
	{
		viewport vp;
	public:
		default_render_target(const viewport& _vp) //viewport(vec2(-1)) makes a viewport that renders to the whole screen with the default settings
			: vp(_vp) {}
		void ombind(device* dev) override
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport((GLint)vp.offset.x, (GLint)vp.offset.y, (GLsizei)vp.size.x, (GLsizei)vp.size.y);
			glDepthRange(vp.min_depth, vp.max_depth);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		inline viewport& mviewport() override { return vp; }
	};

	class render_texture2d : public texture2d, public render_target
	{
	protected:
		GLuint _fbo;
		texture2d* depthtx;
		viewport _vp;
		bool _wstencil;
	public:
		render_texture2d(size_vec_t size, const pixel_format& f = pixel_format(pixel_components::rgba, pixel_type::floatp, 32),
			const pixel_format& df = pixel_format(pixel_components::depth, pixel_type::floatp, 32));
		render_texture2d(const viewport& vp, const pixel_format& f = pixel_format(pixel_components::rgba, pixel_type::floatp, 32),
			const pixel_format& df = pixel_format(pixel_components::depth, pixel_type::floatp, 32));

		void ombind(device* dev) override
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
			glViewport((GLint)_vp.offset.x, (GLint)_vp.offset.y,
				(GLsizei)(_vp.size.x < 0 ? dev->size().x : _vp.size.x),
				(GLsizei)(_vp.size.y < 0 ? dev->size().y : _vp.size.y));
			glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
				_vp.max_depth < 0 ? 1.f : _vp.max_depth);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (_wstencil) glClear(GL_STENCIL_BITS);
		}

		inline viewport& mviewport() override { return _vp; }

		propr(GLuint, frame_buffer, { return _fbo; })
		propr(texture2d*, depth_buffer, { return depthtx; })
	};

	template <int N>
	class multi_render_texture2d : public gluk::render_target
	{
		GLuint _fbo;
		GLuint _tex[N];
		texture<2>* _mtex[N];
		GLuint _dtx;
		gluk::viewport _vp;
	public:
		multi_render_texture2d(gluk::viewport vp,
			gluk::pixel_format pf = gluk::pixel_format(gluk::pixel_components::rgba, gluk::pixel_type::floatp, 32),
			gluk::pixel_format dpf = gluk::pixel_format(gluk::pixel_components::depth, gluk::pixel_type::floatp, 32))
			: _vp(vp)
		{
			glGenFramebuffers(1, &_fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glGenTextures(N, _tex);
			glGenTextures(1, &_dtx);

			for (int i = 0; i < N; ++i)
			{
				glBindTexture(GL_TEXTURE_2D, _tex[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, pf.get_gl_format_internal(),
					vp.size.x, vp.size.y, 0, pf.get_gl_format(), pf.get_gl_type(), nullptr);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _tex[i], 0);

				_mtex[i] = new texture<2>(_tex[i], vp.size);
			}

			glBindTexture(GL_TEXTURE_2D, _dtx);
			glTexImage2D(GL_TEXTURE_2D, 0, dpf.get_gl_format_internal(), vp.size.x, vp.size.y, 0, dpf.get_gl_format(), dpf.get_gl_type(), nullptr);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _dtx, 0);

			vector<GLenum> db;
			for (int i = 0; i < N; ++i) db.push_back(GL_COLOR_ATTACHMENT0 + i);
			glDrawBuffers(db.size(), db.data());

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		void ombind(gluk::device* dev) override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glViewport(_vp.offset.x, _vp.offset.y,
				_vp.size.x < 0 ? dev->size().x : _vp.size.x,
				_vp.size.y < 0 ? dev->size().y : _vp.size.y);
			glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
				_vp.max_depth < 0 ? 1.f : _vp.max_depth);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void bind_for_read()
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
		}

		gluk::viewport& mviewport() { return _vp; }

		propr(GLuint, fbo, { return _fbo; });
		propr(GLuint, depth_texture, { return _dtx; });

		inline GLuint get_gl_texture(int i)
		{
			return _tex[i];
		}

		inline const texture<2>& get_texture(int i)
		{
			return *_mtex[i];
		}

		~multi_render_texture2d()
		{
			glDeleteTextures(N, _tex);
			glDeleteTextures(1, &_dtx);
			glDeleteFramebuffers(1, &_fbo);
		}
	};
}