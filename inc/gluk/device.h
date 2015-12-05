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
		template <typename T>
		friend class uniform_buffer;

		map<string, tuple<GLint, int>> shader_cashe;
		GLint load_shader(const string& path, const package& pak, GLenum type);
		void delete_shader(GLint id);
		friend class shader;
	public:
		device(vec2 s_, GLFWwindow* w_, uint aal, int frames_before_swap = 0);
		~device();
		render_target* current_render_target() const;
		void pop_render_target(bool clear = true);
		void push_render_target(render_target* rt, bool clear = true);
		void update_render_target(bool c);
		void present();
		void resize(vec2 ns);


		propr(vec2, size, { return _rtsize; })
	};
}