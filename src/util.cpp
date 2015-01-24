#include "util.h"

namespace gluk
{
	namespace util
	{
		char fsq_vertex_shader_src[] =
			"#version 410\n\n"
			"layout(location=0) in vec3 pos;"
			"void main() { gl_Position = vec4(pos.xy, 0.5f, 1.f); }";

		full_screen_quad_shader::full_screen_quad_shader(const filedatasp ps)
			: shader(make_shared<constmem_filedata>(fsq_vertex_shader_src, sizeof(fsq_vertex_shader_src)), ps)
		{
			fsq = new interleaved_mesh<vertex_position, uint16>(generate_screen_quad<vertex_position, uint16>(vec2(0.f), vec2(2.f)), "");
		}
		
		full_screen_quad_shader::~full_screen_quad_shader()
		{
			delete fsq;
		}

		void full_screen_quad_shader::draw(device* dev, float t)
		{
			set_uniform("time", t);
			set_uniform("resolution", dev->size());
			fsq->draw();
		}
	

		camera::camera(vec2 ss, vec3 p, vec3 t, vec3 u, float fov)
			: _pos(p), _look(t - p), _up(u), _fov(fov), _nz(0.01f), _fz(1000.f)
		{
			_right = cross(_up, _look);
			update_view();
			update_proj(ss);
		}

		void camera::update_proj(vec2 size)
		{
			_proj = perspectiveFov(_fov, size.x, size.y, _nz, _fz);
		}

		void camera::update_view()
		{
			_look = normalize(_look);
			_up = normalize(_up);
			_right = cross(_look, _up);
			_up = cross(_right, _look);

			float px = -dot(_right, _pos);
			float py = -dot(_up, _pos);
			float pz = dot(_look, _pos);

			_view = mat4(1);
			_view[0][0] = _right.x;
			_view[1][0] = _right.y;
			_view[2][0] = _right.z;
			_view[0][1] = _up.x;
			_view[1][1] = _up.y;
			_view[2][1] = _up.z;
			_view[0][2] = -_look.x;
			_view[1][2] = -_look.y;
			_view[2][2] = -_look.z;
			_view[3][0] = px;
			_view[3][1] = py;
			_view[3][2] = pz;
		}

		void camera::transform(const mat4& t)
		{
			_look = vec3(vec4(_look, 0)*t);
			_up = vec3(vec4(_up, 0)*t);
			_right = vec3(vec4(_right, 0)*t);
		}
	}
}