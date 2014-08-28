#pragma once
#include "cmmn.h"

namespace gluk
{
	class render_target;
	class default_render_target;

	class device
	{
		stack<render_target*> rtsk;
		default_render_target* drt;
	protected:
		GLFWwindow* wnd;
		vec2 _rtsize;
		uint next_uniform_buffer_bind_index;
		queue<uint> previously_alloced_ubbi;
		uint alloc_ubbi();
		void free_ubbi(uint& ubbi);
	public:
		device(vec2 s_, GLFWwindow* w_, uint aal);
		~device();
		render_target* current_render_target() const;
		void pop_render_target();
		void push_render_target(render_target* rt);
		void update_render_target();
		void present();
		void resize(vec2 ns);

		propr(vec2, size, { return _rtsize; })
	};
}