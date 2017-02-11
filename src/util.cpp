#include "util.h"

namespace gluk
{
	namespace util
	{
		char fsq_vertex_shader_src[] =
			"#version 410\n\n"
			"layout(location=0) in vec3 pos;"
			"void main() { gl_Position = vec4(pos.xy, 0.5f, 1.f); }";

		full_screen_quad_shader::full_screen_quad_shader(device* dev, const filedatasp ps, const filedatasp vs)
			: shader(dev, vs ? vs : make_shared<constmem_filedata>(fsq_vertex_shader_src, sizeof(fsq_vertex_shader_src)), ps)
		{
			fsq = new interleaved_mesh<vertex_position, uint16>(generate_screen_quad<vertex_position, uint16>(vec2(0.f), vec2(2.f)));
		}
		
		full_screen_quad_shader::~full_screen_quad_shader()
		{
			delete fsq;
		}

		void full_screen_quad_shader::draw(float t)
		{
			set_uniform("time", t, false);
			set_uniform("resolution", _dev->size(), false);
			auto x = glIsEnabled(GL_CULL_FACE);
			glDisable(GL_CULL_FACE); //fsq generator is not quite right, so it produces a back facing quad 
			fsq->draw();
			if(x) glEnable(GL_CULL_FACE);
		}
	

		perspective_camera::perspective_camera(vec2 ss, vec3 p, vec3 t, vec3 u, float fov, float nz, float fz)
			: camera(p,t,u), _fov(fov), _nz(nz), _fz(fz)
		{
			update_view();
			update_proj(ss);
		}


		void perspective_camera::update_proj(vec2 size)
		{
			_proj = perspectiveFov(_fov, size.x, size.y, _nz, _fz);
			_ar = size.x / size.y;
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

		void pls3p(vec3 a, vec3 b, vec3 c, vec3& n, float& d) {
			n = normalize(cross(c - b, a - b));
			d = -dot(n, b);
		}

		math::frustrum::frustrum(float fov, float aspect, float nz, float fz, vec3 pos, mat3 basis) {
			float tan_hfov = tan(fov*1.2f),
				nh = nz * tan_hfov,
				nw = nh * aspect,
				fh = fz * tan_hfov,
				fw = fh * aspect;
			
			vec3 Z = -basis[0];
			vec3 X = basis[2];
			vec3 Y = basis[1];

			vec3 nc = pos + Z*nz;
			vec3 fc = pos - Z*fz;
			
			vec3 ntl = nc + Y * nh - X * nw;
			vec3 ntr = nc + Y * nh + X * nw;
			vec3 nbl = nc - Y * nh - X * nw;
			vec3 nbr = nc - Y * nh + X * nw;

			vec3 ftl = fc + Y * fh - X * fw;
			vec3 ftr = fc + Y * fh + X * fw;
			vec3 fbl = fc - Y * fh - X * fw;
			vec3 fbr = fc - Y * fh + X * fw;

			pls3p(ntr, ntl, ftl, normal[0], dist[0]);
			pls3p(nbl, nbr, fbr, normal[1], dist[1]);
			pls3p(ntl, nbl, fbl, normal[2], dist[2]);
			pls3p(nbr, ntr, fbr, normal[3], dist[3]);
			pls3p(ntl, ntr, nbl, normal[4], dist[4]);
			pls3p(ftr, ftl, fbl, normal[5], dist[5]);
			
			/*mat4 vp = cam.proj()*cam.view();
			vec4 plns[6];
			plns[0] =  vp[0]  + vp[3];
			plns[1] = -vp[0]  + vp[3];
			plns[2] =  vp[1]  + vp[3];
			plns[3] = -vp[1]  + vp[3];
			plns[4] =  vp[2]  + vp[3];
			plns[5] = -vp[2]  + vp[3];
			for (uint i = 0; i < 6; ++i) {
				normal[i] = plns[i].xyz;
				float l = normal[i].length();
				normal[i] /= l;
				dist[i] = plns[i].w / l;
				//normal[i] = normalize(vec3(plns[i]));
				//dist[i] = plns[i].w;
			}*/
		}
		inline float xovery(vec2 v) { return v.x / v.y; }
		math::frustrum::frustrum(const vec4& ortho_lrtb, vec2 ortho_nf, vec3 pos, mat3 basis) {
			vec2 np = ortho_lrtb.zw - ortho_nf.xy;
			float 
				nh = np.x,
				nw = np.y,
				fh = np.x,
				fw = np.y;

			vec3 Z = -basis[0];
			vec3 X = basis[2];
			vec3 Y = basis[1];

			vec3 nc = pos + Z*ortho_nf.x;
			vec3 fc = pos - Z*ortho_nf.y;

			vec3 ntl = nc + Y * nh - X * nw;
			vec3 ntr = nc + Y * nh + X * nw;
			vec3 nbl = nc - Y * nh - X * nw;
			vec3 nbr = nc - Y * nh + X * nw;

			vec3 ftl = fc + Y * fh - X * fw;
			vec3 ftr = fc + Y * fh + X * fw;
			vec3 fbl = fc - Y * fh - X * fw;
			vec3 fbr = fc - Y * fh + X * fw;

			pls3p(ntr, ntl, ftl, normal[0], dist[0]);
			pls3p(nbl, nbr, fbr, normal[1], dist[1]);
			pls3p(ntl, nbl, fbl, normal[2], dist[2]);
			pls3p(nbr, ntr, fbr, normal[3], dist[3]);
			pls3p(ntl, ntr, nbl, normal[4], dist[4]);
			pls3p(ftr, ftl, fbl, normal[5], dist[5]);
		}
	}
}
