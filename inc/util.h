#pragma once
#include "cmmn.h"
#include "shader.h"
#include "mesh.h"
#include "app.h"

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
			full_screen_quad_shader(device* dev, const filedatasp ps, const filedatasp vs = nullptr);
			~full_screen_quad_shader();

			void draw(float t);
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

		struct fps_camera_controller : public input_handler {
		protected:
			vec3 cam_pos_v; vec2 cam_rot_v; vec2 tot_cam_rot;
		public:
			camera& cam;
			vec3 linear_speed;
			vec2 rotational_speed;
			fps_camera_controller(camera& c, vec3 lin_speed = vec3(7.f, 10.f, 5.f), vec2 rot_speed = vec2(1.f)) 
				: cam(c), linear_speed(lin_speed), rotational_speed(rot_speed) {
			}

			virtual void key_handler(app* _app, uint key, input_action action, input_mod mods) {
				if (action == key_action::press)
				{
					if (key == GLFW_KEY_W)
						cam_pos_v.x = 1;
					else if (key == GLFW_KEY_S)
						cam_pos_v.x = -1;
					else if (key == GLFW_KEY_A)
						cam_pos_v.y = -1;
					else if (key == GLFW_KEY_D)
						cam_pos_v.y = 1;
					else if (key == GLFW_KEY_Q)
						cam_pos_v.z = 1;
					else if (key == GLFW_KEY_E)
						cam_pos_v.z = -1;
				}
				else if (action == key_action::release)
				{
					if (key == GLFW_KEY_W || key == GLFW_KEY_S)
						cam_pos_v.x = 0.f;
					else if (key == GLFW_KEY_A || key == GLFW_KEY_D)
						cam_pos_v.y = 0.f;
					else if (key == GLFW_KEY_Q || key == GLFW_KEY_E)
						cam_pos_v.z = 0.f;
				}
			}

			virtual void mouse_position_handler(app* _app, vec2 _p) {
				if (glfwGetInputMode(_app->wnd, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
					glfwSetInputMode(_app->wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				auto smid = _app->dev->size();
				vec2 np = (_p/smid)*2.f - vec2(1.f);
				cam_rot_v = np*60.f;
				glfwSetCursorPos(_app->wnd, smid.x*0.5f, smid.y*0.5f);
			}

			void update(float t, float dt) {
				cam.fwd(cam_pos_v.x*dt*linear_speed.x);
				cam.straft(cam_pos_v.y*dt*linear_speed.y);
				cam.move_up(cam_pos_v.z*dt*linear_speed.z);
				cam.transform(rotate(mat4(1), cam_rot_v.x*dt*rotational_speed.x, vec3(0.f, 1.f, 0.f))); 
				cam.pitch(cam_rot_v.y*dt*rotational_speed.y);
				cam_rot_v = vec2(0.f);
				cam.update_view();
			}
		};
	}
}