#include "cmmn.h"
#include "shader.h"
#include "mesh.h"

namespace gluk
{
	namespace util 
	{
		//full_screen_quad_shader
		//Renders a FSQ with the pixel shader given
		class full_screen_quad_shader : public shader
		{
			mesh* fsq;
		public:
			full_screen_quad_shader(const filedatasp ps, const filedatasp vs = nullptr);
			~full_screen_quad_shader();

			void draw(device* dev, float t);
		};

		class camera
		{
			mat4 _view, _proj;
			float _fov, _nz, _fz;
			vec3 _pos, _look, _up, _right;
		public:
			camera() {}
			camera(vec2 ss, vec3 p, vec3 t, vec3 u = vec3(0.f, 1.f, 0.f), float fov = radians(45.f),
				float nz = 0.5f, float fz = 500.f);

			void update_proj(vec2 size);
			void update_view();

			inline void look_at(vec3 p, vec3 t, vec3 u)
			{
				_pos = p; _look = t - _pos; _up = u;
			}

			proprw(vec3, position, { return _pos; });
			propr(vec3, position, { return _pos; });

			propr(float, fov, { return _fov; });
			proprw(float, near_z, { return _nz; });
			proprw(float, far_z, { return _fz; });

			propr(vec3, target, { return _pos + _look; });
			inline void target(vec3 p) { _look = p - _pos; }

			propr(mat4, view, { return _view; });
			propr(mat4, proj, { return _proj; });

			inline void fwd(float d) { _pos += d*_look; }
			inline void straft(float d) { _pos += d*_right; }
			inline void move_up(float d) { _pos += d*_up; }

			void transform(const mat4& t);

			inline void yaw(float a) { transform(rotate(mat4(1), a, _up)); }
			inline void pitch(float a) { transform(rotate(mat4(1), a, _right)); }
			inline void roll(float a) { transform(rotate(mat4(1), a, _look)); }

			tuple<vec3, vec3, vec3> basis()
			{
				return tuple<vec3, vec3, vec3>(_look, _up, _right);
			}
		};
	}
}