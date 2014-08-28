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
			glViewport(vp.offset.x, vp.offset.y, vp.size.x, vp.size.y);
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
			glViewport(_vp.offset.x, _vp.offset.y,
				_vp.size.x < 0 ? dev->size().x : _vp.size.x,
				_vp.size.y < 0 ? dev->size().y : _vp.size.y);
			glDepthRange(_vp.min_depth < 0 ? 0.f : _vp.min_depth,
				_vp.max_depth < 0 ? 1.f : _vp.max_depth);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (_wstencil) glClear(GL_STENCIL_BITS);
		}

		inline viewport& mviewport() override { return _vp; }

		propr(GLuint, frame_buffer, { return _fbo; })
		propr(texture2d*, depth_buffer, { return depthtx; })
	};
}