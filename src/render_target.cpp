#include "render_target.h"

namespace gluk
{
	render_texture2d::render_texture2d(size_vec_t size, const pixel_format& f, const pixel_format& df)
		: _vp(size), render_target(df.comp == pixel_components::depth_stencil), texture2d(f, size),
		depthtx(new texture2d(df, size))
	{
		glGenFramebuffers(1, &_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		
		glBindTexture(GL_TEXTURE_2D, _txid);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txid, 0);

		glBindTexture(GL_TEXTURE_2D, depthtx->id());
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, _wstencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtx->id(), 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	render_texture2d::render_texture2d(const viewport& vp, const pixel_format& f, const pixel_format& df)
		: _vp(vp), render_target(df.comp == pixel_components::depth_stencil), texture2d(f, vp.size),
		depthtx(new texture2d(df, vp.size))
	{
		glGenFramebuffers(1, &_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

		glBindTexture(GL_TEXTURE_2D, _txid);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txid, 0);

		glBindTexture(GL_TEXTURE_2D, depthtx->id());
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, _wstencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtx->id(), 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void render_texture_cube::render_face::ombind(device* dev)
	{
		const auto& _vp = rtc->_vp;	
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rtc->_fbo[idx]);
		glViewport(_vp.offset.x, _vp.offset.y,
			_vp.size.x < 0 ? dev->size().x : _vp.size.x,
			_vp.size.y < 0 ? dev->size().y : _vp.size.y);
		glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
			_vp.max_depth < 0 ? 1.f : _vp.max_depth);

	}

	render_texture_cube::render_texture_cube(viewport vp,
		pixel_format pf,
		pixel_format dpf)
		: texture_cube(pf, vp.size), _vp(vp), _db(dpf, vp.size), _wstencil(dpf.comp == pixel_components::depth_stencil)
	{
		static GLuint face[] = 
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};
		glGenFramebuffers(6, _fbo);
		for (uint i = 0; i < 6; ++i)
		{
			rtx[i] = render_face(this, i);
			glBindFramebuffer(GL_FRAMEBUFFER, _fbo[i]);
			glFramebufferTextureFaceEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->id(), 0,
				face[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, dpf.comp == pixel_components::depth_stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _db.id(), 0);
		}
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	render_texture_cube::~render_texture_cube()
	{
		glDeleteFramebuffers(6, _fbo);
	}

}