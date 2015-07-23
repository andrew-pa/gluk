#pragma once
#include "cmmn.h"
#include "device.h"
#include "texture.h"

namespace gluk
{

	struct viewport
	{
		ivec2 offset;
		uvec2 size;
		float min_depth;
		float max_depth;
		viewport(uvec2 si, ivec2 os = ivec2(0), float md = 0.f, float xd = 1.f)
			: offset(os), size(si), min_depth(md), max_depth(xd){}
		
	};

	class render_target
	{
	protected:
		bool _wstencil;
		render_target(bool _ws) : _wstencil(_ws) {}
	public:
		virtual void ombind(device* dev) = 0;

		virtual void bind_for_read() = 0;

		virtual void clear()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (_wstencil) glClear(GL_STENCIL_BITS);
		}
		virtual viewport& mviewport() = 0;
	};

	class default_render_target : public render_target
	{
		viewport vp;
	public:
		default_render_target(const viewport& _vp) //viewport(vec2(-1)) makes a viewport that renders to the whole screen with the default settings
			: vp(_vp), render_target(false) 
		{
			glerr
		}
		void ombind(device* dev) override
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport((GLint)vp.offset.x, (GLint)vp.offset.y, (GLsizei)vp.size.x, (GLsizei)vp.size.y);
			glDepthRange(vp.min_depth, vp.max_depth);
		}
		void bind_for_read() override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		}
		inline viewport& mviewport() override { return vp; }
	};

	class external_fbo_render_target : public render_target
	{
		viewport vp;
	public:
		GLuint* fbo;
		
		external_fbo_render_target() : render_target(false), vp(vec2(0)), fbo(nullptr) {}
		external_fbo_render_target(const viewport& _vp, GLuint& _fbo, bool has_stencil = false)
			: render_target(has_stencil), vp(_vp), fbo(&_fbo)
		{}

		void ombind(device* dev) override
		{
			glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
			glViewport((GLint)vp.offset.x, (GLint)vp.offset.y, (GLsizei)vp.size.x, (GLsizei)vp.size.y);
			glDepthRange(vp.min_depth, vp.max_depth);
		}

		void bind_for_read() override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, *fbo);
		}
		
		inline viewport& mviewport() override { return vp; }
	};

	class render_texture2d : public texture2d, public render_target
	{
	protected:
		GLuint _fbo;
		GLuint rbo;
		texture2d* depthtx;
		viewport _vp;
	public:
		//render_texture2d(size_vec_t size, const pixel_format& f = pixel_format(pixel_components::rgba, pixel_type::floatp, 32),
		//	const pixel_format& df = pixel_format(pixel_components::depth, pixel_type::floatp, 32));
		render_texture2d(const viewport& vp, const pixel_format& f = pixel_format(pixel_components::rgba, pixel_type::floatp, 32),
			const pixel_format& df = pixel_format(pixel_components::depth, pixel_type::floatp, 32), bool use_rbo_dsb = true);

		void ombind(device* dev) override
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
			glViewport((GLint)_vp.offset.x, (GLint)_vp.offset.y,
				(GLsizei)(_vp.size.x < 0 ? dev->size().x : _vp.size.x),
				(GLsizei)(_vp.size.y < 0 ? dev->size().y : _vp.size.y));
			glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
				_vp.max_depth < 0 ? 1.f : _vp.max_depth);
		}

		void bind_for_read() override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
		}

		inline viewport& mviewport() override { return _vp; }

		propr(GLuint, frame_buffer, { return _fbo; })
		propr(texture2d*, depth_buffer, { return depthtx; })

		~render_texture2d() 
		{
			glDeleteFramebuffers(1, &_fbo);
			if (depthtx) { delete depthtx; depthtx = nullptr; } 
		}
	};

	class depth_render_texture2d : public texture2d, public render_target {
	protected:
		GLuint _fbo;
		viewport _vp;
	public:
		depth_render_texture2d(const viewport& vp, const pixel_format& df = pixel_format(pixel_components::depth, pixel_type::floatp, 32));

		void ombind(device* dev) override
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
			glViewport((GLint)_vp.offset.x, (GLint)_vp.offset.y,
				(GLsizei)(_vp.size.x < 0 ? dev->size().x : _vp.size.x),
				(GLsizei)(_vp.size.y < 0 ? dev->size().y : _vp.size.y));
			glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
				_vp.max_depth < 0 ? 1.f : _vp.max_depth);
		}
		virtual void clear()
		{
			glClear(GL_DEPTH_BUFFER_BIT);
			if (_wstencil) glClear(GL_STENCIL_BITS);
		}

		void bind_for_read() override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
		}

		void set_comparison_mode(GLenum f, GLenum mode = GL_COMPARE_REF_TO_TEXTURE) {
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, mode);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, f);
		}

		void disable_comparison_mode() {
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, 0);
		}

		inline viewport& mviewport() override { return _vp; }

		~depth_render_texture2d()
		{
			glDeleteFramebuffers(1, &_fbo);
		}
	};

	class render_texture_cube : public texture_cube
	{
		viewport _vp;
		struct render_face : public render_target
		{
			render_texture_cube* rtc;
			uint idx;
			render_face():render_target(false){}
			render_face(render_texture_cube* r, uint i)
				: idx(i), rtc(r), render_target(r->_wstencil) {}
			void ombind(device* dev) override;
			viewport& mviewport() override { return rtc->_vp; }
			void bind_for_read() override
			{
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, rtc->_fbo);
			}
		};
		render_face rtx[6];
		GLuint _fbo;//[6];
		texture2d _db;
		bool _wstencil;
	public:
		render_texture_cube(viewport vp,
			gluk::pixel_format pf = gluk::pixel_format(gluk::pixel_components::rgba, gluk::pixel_type::floatp, 32),
			gluk::pixel_format dpf = gluk::pixel_format(gluk::pixel_components::depth, gluk::pixel_type::floatp, 32));

		inline render_face& target_for_face(uint i)
		{
			return rtx[i];
		}

		proprw(viewport, mviewport, { return _vp; });
		proprw(texture2d, depth_buffer, { return _db; });

		~render_texture_cube();
	};

	template <int N>
	class multi_render_texture2d : public gluk::render_target
	{
		GLuint _fbo;
		GLuint _tex[N];
		texture<2>* _mtex[N];
		texture2d _dtx;
		gluk::viewport _vp;
	public:
		multi_render_texture2d(gluk::viewport vp,
			gluk::pixel_format pf = gluk::pixel_format(gluk::pixel_components::rgba, gluk::pixel_type::floatp, 32),
			gluk::pixel_format dpf = gluk::pixel_format(gluk::pixel_components::depth, gluk::pixel_type::floatp, 32))
			: _vp(vp), render_target(dpf.comp == pixel_components::depth_stencil),
			_dtx(dpf, vp.size)
		{
			glGenFramebuffers(1, &_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
			glGenTextures(N, _tex);

			for (int i = 0; i < N; ++i)
			{
				glBindTexture(GL_TEXTURE_2D, _tex[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, pf.get_gl_format_internal(),
					(GLsizei)vp.size.x, (GLsizei)vp.size.y, 0, pf.get_gl_format(), pf.get_gl_type(), nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _tex[i], 0);

				_mtex[i] = new texture<2>(_tex[i], vp.size);
			}

			glBindTexture(GL_TEXTURE_2D, _dtx);
			glFramebufferTexture2D(GL_FRAMEBUFFER, _wstencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _dtx, 0);

			vector<GLenum> db;
			for (int i = 0; i < N; ++i) db.push_back(GL_COLOR_ATTACHMENT0 + i);
			glDrawBuffers(db.size(), db.data());

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void current_buffers(initializer_list<GLenum> buffers) {
			glDrawBuffers(buffers.size(), buffers.begin());
		}

		void ombind(gluk::device* dev) override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glViewport((GLint)_vp.offset.x, (GLint)_vp.offset.y,
				(GLsizei)(_vp.size.x < 0 ? dev->size().x : _vp.size.x),
				(GLsizei)(_vp.size.y < 0 ? dev->size().y : _vp.size.y));
			glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
				_vp.max_depth < 0 ? 1.f : _vp.max_depth);
		}

		void bind_for_read() override
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
		}
		
		gluk::viewport& mviewport() { return _vp; }

		propr(GLuint, fbo, { return _fbo; });
		proprw(texture2d, depth_texture, { return _dtx; });

		inline GLuint get_gl_texture(int i)
		{
			return _tex[i];
		}

		inline texture<2>& get_texture(int i)
		{
			return *_mtex[i];
		}

		~multi_render_texture2d()
		{
			glDeleteTextures(N, _tex);
			glDeleteFramebuffers(1, &_fbo);
		}
	};
}