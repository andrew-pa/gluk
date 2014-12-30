#include "render_target.h"

namespace gluk
{
	/*render_texture2d::render_texture2d(size_vec_t size, const pixel_format& f, const pixel_format& df)
		: _vp((vec2)size), render_target(df.comp == pixel_components::depth_stencil), texture2d(f, (uvec2)size),
		depthtx(new texture2d(df, size))
	{
		this->min_filter(GL_LINEAR);
		this->mag_filter(GL_LINEAR);
		this->ansiotropic_filter(0);
		glGenFramebuffers(1, &_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		
		//glBindTexture(GL_TEXTURE_2D, _txid);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txid, 0);

		//glBindTexture(GL_TEXTURE_2D, depthtx->id());
		//glFramebufferTexture2D(GL_FRAMEBUFFER, _wstencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtx->id(), 0);
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, df.get_gl_format_internal(), size.x, size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		//glDrawBuffer(GL_COLOR_ATTACHMENT0);

		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			throw;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}*/

	render_texture2d::render_texture2d(const viewport& vp, const pixel_format& f, const pixel_format& df, bool use_rbo_dsb)
		: _vp(vp), render_target(df.comp == pixel_components::depth_stencil), texture2d(f, (uvec2)vp.size),
		depthtx(use_rbo_dsb ? nullptr : new texture2d(df, (uvec2)vp.size))
	{
		glGenFramebuffers(1, &_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txid, 0);

		if(use_rbo_dsb) 
		{
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, df.get_gl_format_internal(), vp.size.x, vp.size.y);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, _wstencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		}
		else
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, _wstencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtx->id(), 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void render_texture_cube::render_face::ombind(device* dev)
	{
		const auto& _vp = rtc->_vp;	
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rtc->_fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cube_map_face_for_index(idx),
			rtc->_txid, 0);
		glViewport(_vp.offset.x, _vp.offset.y,
			_vp.size.x < 0 ? dev->size().x : _vp.size.x,
			_vp.size.y < 0 ? dev->size().y : _vp.size.y);
		glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
			_vp.max_depth < 0 ? 1.f : _vp.max_depth);

	}

	render_texture_cube::render_texture_cube(viewport vp,
		pixel_format pf,
		pixel_format dpf)
		: texture_cube(pf, (uvec2)vp.size), _vp(vp), _db(dpf, (uvec2)vp.size), _wstencil(dpf.comp == pixel_components::depth_stencil)
	{
		
		glGenFramebuffers(1, &_fbo);
		for (uint i = 0; i < 6; ++i)
		{
			rtx[i] = render_face(this, i);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);;
		glFramebufferTexture2D(GL_FRAMEBUFFER, dpf.comp == pixel_components::depth_stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _db.id(), 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	render_texture_cube::~render_texture_cube()
	{
		glDeleteFramebuffers(1, &_fbo);
	}

}