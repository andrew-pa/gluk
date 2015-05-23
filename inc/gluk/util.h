#pragma once
#include "cmmn.h"
#include "shader.h"
#include "mesh.h"
#include "app.h"
#include "texture.h"
namespace gluk
{
	namespace util 
	{
		namespace math {
			struct aabb {
				vec3 min, max;
				aabb() : min(), max() {}
				aabb(vec3 m, vec3 x) : min(m), max(x) {}

				aabb(const aabb& a, const aabb& b) : min(), max() {
					add_point(a.min);
					add_point(a.max);
					add_point(b.min);
					add_point(b.max);
				}

				inline void add_point(vec3 p) {
					if (p.x > max.x) max.x = p.x;
					if (p.y > max.y) max.y = p.y;
					if (p.z > max.z) max.z = p.z;

					if (p.x < min.x) min.x = p.x;
					if (p.y < min.y) min.y = p.y;
					if (p.z < min.z) min.z = p.z;
					//min = glm::min(min, p);
					//max = glm::max(max, p);
				}

				inline aabb transform(const mat4& m) const {
					vec3 nmin, nmax;
					nmin = vec3(m[3][0], m[3][1], m[3][2]);
					nmax = nmin;

					for (int i = 0; i < 3; ++i)
						for (int j = 0; j < 3; ++j) {
							if (m[i][j] > 0) {
								nmin[i] += m[i][j] * min[j];
								nmax[i] += m[i][j] * max[j];
							}
							else {
								nmin[i] += m[i][j] * max[j];
								nmax[i] += m[i][j] * min[j];
							}
						}
					return aabb(nmin, nmax);
				}

				inline vec3 p_vertex(vec3 n) const {
					vec3 r = min;
					if (n.x > 0) r.x += max.x;
					if (n.y > 0) r.y += max.y;
					if (n.z > 0) r.z += max.z;
					return r;
				}

				inline vec3 n_vertex(vec3 n) const {
					vec3 r = min;
					if (n.x < 0) r.x += max.x;
					if (n.y < 0) r.y += max.y;
					if (n.z < 0) r.z += max.z;
					return r;
				}

				inline vec3 center() const {
					return (min + max) * 0.5f;
				}
				inline vec3 extents() const {
					return (max - min);
				}

				inline void operator +=(const aabb& b) {
					add_point(b.min);
					add_point(b.max);
				}

				inline int max_extent() const
				{
					vec3 diag = max - min;
					if (diag.x > diag.y && diag.x > diag.z)
						return 0;
					else if (diag.y > diag.z)
						return 1;
					else
						return 2;
				}

				inline float surface_area() const
				{
					vec3 d = max - min;
					return 2.f*(d.x*d.y + d.x*d.z + d.y*d.z);
				}
			};
		
			struct sphere {
				vec3 pos; float radius;
				sphere(vec3 p, float r) : pos(p), radius(r) {} 
			};

			struct frustrum {
				vec3 normal[6];
				float dist[6];
				frustrum(const mat4& view, const mat4& proj) {
					mat4 vp = proj*view;
					vec4 plns[6];
					plns[0] =  vp[0]  + vp[3];
					plns[1] = -vp[0]  + vp[3];
					plns[2] =  vp[1]  + vp[3];
					plns[3] = -vp[1]  + vp[3];
					plns[4] =  vp[2]  + vp[3];
					plns[5] = -vp[2]  + vp[3];
					for (uint i = 0; i < 6; ++i) {
						normal[i] = normalize(vec3(plns[i]));
						dist[i] = plns[i].w;
					}
				}

				//'contains' functions are containment || intersection 
				
				inline bool contains(const sphere& s) const {
					for (uint i = 0; i < 6; ++i) {
						float d = dot(normal[i], s.pos) + dist[i];
						if (d < -s.radius)
							return false;
					}
					return true;
				}

				inline bool contains(const aabb& b) const {
					for (uint i = 0; i < 6; ++i) {
						if ((dist[i] + dot(normal[i], b.p_vertex(normal[i]))) < 0) return false;
						if ((dist[i] + dot(normal[i], b.n_vertex(normal[i]))) < 0) return true;
					}
					return true;
				}
			};
		}
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

			inline math::frustrum make_frustrum() const {
				return math::frustrum(_view, _proj);
			}
		};

		struct fps_camera_controller : public input_handler {
		protected:
			vec3 cam_pos_v; vec2 cam_rot_v; vec2 tot_cam_rot; 
		public:
			camera& cam;
			vec3 linear_speed;
			vec2 rotational_speed;
			bool mouse_disabled;
			uint normal_cursor_mode;
			fps_camera_controller(camera& c, vec3 lin_speed = vec3(7.f, 10.f, 5.f), vec2 rot_speed = vec2(1.f)) 
				: cam(c), linear_speed(lin_speed), rotational_speed(rot_speed), mouse_disabled(false), normal_cursor_mode(GLFW_CURSOR_NORMAL) {
			}

			virtual void key_handler(app* _app, uint key, input_action action, input_mod mods) {
				if(key == GLFW_KEY_F1 && action == input_action::release) {
					mouse_disabled = !mouse_disabled;
					if (glfwGetInputMode(_app->wnd, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
						glfwSetInputMode(_app->wnd, GLFW_CURSOR, normal_cursor_mode);
				}

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
				if(!mouse_disabled) {
					if (glfwGetInputMode(_app->wnd, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
						glfwSetInputMode(_app->wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					auto smid = _app->dev->size();
					vec2 np = (_p/smid)*2.f - vec2(1.f);
					cam_rot_v = np*60.f;
					glfwSetCursorPos(_app->wnd, smid.x*0.5f, smid.y*0.5f);
				}
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

		class texture_cashe {
		protected:
			const package& pak;
			shared_ptr<texture2d> blank_texture;
		public:
			map<size_t, shared_ptr<texture2d>> textures;

			texture_cashe(const package& _pak) : pak(_pak) {
				vec4* v = new vec4(1.f);
				blank_texture = make_shared<texture2d>(pixel_format(pixel_components::rgba, pixel_type::floatp, 32),
					vec2(1), v);
				delete v;
				textures[hash<string>()("")] = blank_texture;
			}

			pair<size_t, shared_ptr<texture2d>> load_texture(const string& path) {
				auto hpath = hash<string>()(path);
				auto t = textures.find(hpath);
				if(t == textures.end()) {
					return make_pair(hpath, textures[hpath] = make_shared<texture2d>(pak.open(path)));
				}
				else return *t;
			}


		};
	}
}