#include "render_target.h"

namespace gluk
{
	render_texture2d::render_texture2d(size_vec_t size, const pixel_format& f, const pixel_format& df)
		: _vp(size), _wstencil(df.comp == pixel_components::depth_stencil), texture2d(f, size),
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
		: _vp(vp), _wstencil(df.comp == pixel_components::depth_stencil), texture2d(f, vp.size),
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
}