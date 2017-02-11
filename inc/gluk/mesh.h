#pragma once
#include "cmmn.h"
#include "device.h"

namespace gluk
{
	template <typename index_type>
	struct gl_type_for_index_type
	{
		const static GLenum value = GL_INVALID_ENUM;
	};

	template <>
	struct gl_type_for_index_type<uint8>
	{
		const static GLenum value = GL_UNSIGNED_BYTE;
	};

	template <>
	struct gl_type_for_index_type<uint16>
	{
		const static GLenum value = GL_UNSIGNED_SHORT;
	};

	template <>
	struct gl_type_for_index_type<uint32>
	{
		const static GLenum value = GL_UNSIGNED_INT;
	};

	//vertex_attrib
	//	represents a vertex attribute, is GL equivalent to D3D11_INPUT_ELEMENT_DESC struct
	struct vertex_attrib
	{
		GLuint idx;
		GLuint count;
		GLuint offset;
		GLenum type;
		vertex_attrib(GLuint i, GLuint c, GLenum t, GLuint o)
			: idx(i), count(c), type(t), offset(o) {}
		inline void apply(GLuint stride, GLint base_idx = 0) const
		{
			glVertexAttribPointer(idx+base_idx, count, type, GL_FALSE, stride, (void*)offset);
		}
	};

#define DEF_VERTEX_MC(t) t(vec3 position, vec3 normal, vec3 tangent, vec2 texcoords)

	//vertex_position
	//	vertex with only position coord
	struct vertex_position
	{
		static vector<vertex_attrib> get_vertex_attribs();

		vec3 pos;
		vertex_position(vec3 p)
			: pos(p)
		{
		}

		DEF_VERTEX_MC(vertex_position)
			: pos(position) {}
	};

	//vertex_position_normal_texture
	// vertex with position coords, normal, and texture coords. Frequently used
	struct vertex_position_normal_texture
	{
		static vector<vertex_attrib> get_vertex_attribs();
		vec3 pos;
		vec3 norm;
		vec2 tex;
		vertex_position_normal_texture(){}
		vertex_position_normal_texture(vec3 p, vec3 n, vec2 t)
			: pos(p), norm(n), tex(t)
		{
		}
		vertex_position_normal_texture(float px, float py, float pz, float nx, float ny, float nz, float tx = 0, float ty = 0, float tz = 0, float u = 0, float v = 0)
			: pos(px, py, pz), norm(nx, ny, nz), tex(u, v){}

		DEF_VERTEX_MC(vertex_position_normal_texture)
			: pos(position), norm(normal), tex(texcoords) {}
	};

	//vertex_position_normal_tangent_texture
	// vertex with position coords, normal, tangent, and texture coords. Frequently used with bump mapping
	struct vertex_position_normal_tangent_texture
	{
		static vector<vertex_attrib> get_vertex_attribs();

		vec3 pos;
		vec3 norm;
		vec3 tang;
		vec2 tex;
		vertex_position_normal_tangent_texture(){}
		vertex_position_normal_tangent_texture(vec3 p, vec3 n, vec3 tg, vec2 t)
			: pos(p), norm(n), tang(tg), tex(t)
		{
		}

	};

	//prim_draw_type
	//	Describes how the GPU should handle the vertex data
	enum class prim_draw_type
	{
		triangle_list = GL_TRIANGLES,
		triangle_strip = GL_TRIANGLE_STRIP,
		line_list = GL_LINES,
		line_strip = GL_LINE_STRIP,
		point_list = GL_POINTS,
	};

	namespace detail {
	inline uint vertices_per_prim(prim_draw_type dt) {
		switch (dt)
		{
		case gluk::prim_draw_type::triangle_list:
			return 3;
		case gluk::prim_draw_type::triangle_strip:
			return 2;
		case gluk::prim_draw_type::line_list:
			return 2;
		case gluk::prim_draw_type::line_strip:
			return 1;
		case gluk::prim_draw_type::point_list:
			return 1;
		}
	}}

	template <typename vertex_type, typename index_type>
	struct sys_mesh
	{
		vector<vertex_type> vertices;
		vector<index_type> indices;
	};

	//mesh
	//	base class for all meshes
	class mesh
	{
	protected:
		GLuint vtx_array;
	public:
		mesh();
		mesh(GLuint vtxa)
			: vtx_array(vtxa) {}
		virtual void draw(prim_draw_type dt = prim_draw_type::triangle_list, 
			int index_offset = 0, int oindex_count = -1, int vertex_offset = 0) = 0;
		~mesh();
		propr(GLuint, vertex_array, { return vtx_array; });
	};
	
	//interleaved_mesh<vertex_type, index_type>
	//	interleaved_mesh is a mesh that uses interleaved vertices, where each vertex is a struct
	//	vertex_type: vertex data, as a struct. see vertex_position and vertex_position_normal_texture for examples of implementation. 
	//	index_type:  type for index values. uint16 or uint32 usually
	template<typename vertex_type, typename index_type> class interleaved_mesh : public mesh
	{
	protected:
		uint idx_cnt;
		uint vtx_cnt;
		GLuint vtx_buf;
		GLuint idx_buf;
	public:
		
		interleaved_mesh(const vector<vertex_type>& vs, const vector<index_type>& is)
			: mesh(), vtx_cnt(vs.size()), idx_cnt(is.size())
		{
			glBindVertexArray(vtx_array);
			glGenBuffers(1, &vtx_buf);
			glGenBuffers(1, &idx_buf);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, is.size()*sizeof(index_type), is.data(), GL_STATIC_DRAW);
			
			glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
			glBufferData(GL_ARRAY_BUFFER, vs.size()*sizeof(vertex_type), vs.data(), GL_STATIC_DRAW);

			auto vabs = vertex_type::get_vertex_attribs();
			for (const auto& v : vabs)
			{
				glEnableVertexAttribArray(v.idx);
				v.apply(sizeof(vertex_type));
			}
		}

		interleaved_mesh(uint vc, uint ic, const string& n, GLuint vb, GLuint ib, GLuint var)
			: mesh(var), idx_cnt(ic), vtx_cnt(vc), vtx_buf(vb), idx_buf(ib)
		{
		}

		interleaved_mesh(const sys_mesh<vertex_type,index_type>& gm)
			: interleaved_mesh(gm.vertices, gm.indices)
		{}
		
		void draw(prim_draw_type dt = prim_draw_type::triangle_list,
			int index_offset = 0, int oindex_count = -1, int vertex_offset = 0) override
		{
			glBindVertexArray(vtx_array);			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glDrawRangeElementsBaseVertex((GLenum)dt, index_offset, (oindex_count == -1 ? idx_cnt : index_offset+oindex_count), (oindex_count == -1 ? idx_cnt : oindex_count),
				gl_type_for_index_type<index_type>::value, (void*)0, vertex_offset); 
		}
		
		~interleaved_mesh()
		{
			glDeleteBuffers(1, &vtx_buf);
			glDeleteBuffers(1, &idx_buf);
		}

		propr(uint, index_count, { return idx_cnt; });
		propr(uint, vertex_count, { return vtx_cnt; });

		propr(GLuint, vertex_buffer, { return vtx_buf; });
		propr(GLuint, index_buffer, { return idx_buf; });

	};

	template<typename vertex_type, typename index_type, typename instance_type> class instanced_interleaved_mesh : public interleaved_mesh<vertex_type,index_type> {
		GLuint instance_buf;
	public:
		instanced_interleaved_mesh(const vector<vertex_type>& vs, const vector<index_type>& is)
			: interleaved_mesh<vertex_type,index_type>(vs, is)
		{
			glGenBuffers(1, &instance_buf);
			glBindBuffer(GL_ARRAY_BUFFER, instance_buf);
			auto idx = vertex_type::get_vertex_attribs()[vertex_type::get_vertex_attribs().size() - 1].idx + 1;
			auto vabs = instance_type::get_vertex_attribs();
			for (const auto& v : vabs)
			{
				glEnableVertexAttribArray(idx+v.idx);
				v.apply(sizeof(instance_type), idx);
				glVertexAttribDivisor(idx + v.idx, 1);
			}
		}
		instanced_interleaved_mesh(const sys_mesh<vertex_type, index_type>& gm) 
			: instanced_interleaved_mesh(gm.vertices, gm.indices)
		{
		}

		void update_instance_buffer(const vector<instance_type>& nt) {
			glBindBuffer(GL_ARRAY_BUFFER, instance_buf);
			glBufferData(GL_ARRAY_BUFFER, nt.size()*sizeof(instance_type), nt.data(), GL_DYNAMIC_DRAW);
		}

		void draw(prim_draw_type dt = prim_draw_type::triangle_list,
			int index_offset = 1, int oindex_count = -1, int vertex_offset = 0) override
		{
			glBindVertexArray(this->vtx_array);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idx_buf);
			glDrawElementsInstancedBaseVertex((GLenum)dt, oindex_count != -1 ? oindex_count : this->idx_cnt, gl_type_for_index_type<index_type>::value, (void*)0,
				index_offset, vertex_offset);
		}


		~instanced_interleaved_mesh() {
			glDeleteBuffers(1, &instance_buf);
		}
	};
	struct multistream_mesh_stream_desc {
		void* data;
		size_t size;
		uint8 element_size;
		GLenum type;
		multistream_mesh_stream_desc(void* d, size_t s, uint8 elsize, GLenum t) 
			: data(d), size(s), element_size(elsize), type(t) {}
	};

	template<typename index_type, typename... vertex_types> class multistream_mesh : public mesh {
		template<typename vertex_type, typename... rest_vertex_types>
		struct vertex_buffers {
			static const bool end = false;
			typedef vertex_type Tvertex;
			GLuint  id;
			vertex_buffers<rest_vertex_types...> rest;

			vertex_buffers() {
				glGenBuffers(1, &id);
			}

			void for_all(function<void(GLuint)> f) {
				f(id);
				rest.for_all(f);
			}

			~vertex_buffers() {
				glDeleteBuffers(1, &id);
			}
		};
		template <typename vertex_type>
		struct vertex_buffers<vertex_type> {
			static const bool end = true;
			typedef vertex_type Tvertex;
			GLuint  id;
			
			vertex_buffers() {
				glGenBuffers(1, &id);
			}

			void for_all(function<void(GLuint)> f) {
				f(id);
			}

			~vertex_buffers() {
				glDeleteBuffers(1, &id);
			}
		};
		vertex_buffers<vertex_types...> vbufs;
		GLuint idx_buf;
		GLuint idx_cnt;
	public:

		multistream_mesh() : mesh() {
			
		}


		multistream_mesh(const vector<index_type>& is, const vector<multistream_mesh_stream_desc>& datas) : mesh() {
			glBindVertexArray(vtx_array);
			glGenBuffers(1, &idx_buf);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, is.size()*sizeof(index_type), is.data(), GL_STATIC_DRAW);
			idx_cnt = is.size();
			int i = 0;
			vbufs.for_all([&](GLuint id) {
				glBindBuffer(GL_ARRAY_BUFFER, id);
				glBufferData(GL_ARRAY_BUFFER, datas[i].size, datas[i].data, GL_STATIC_DRAW);
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, datas[i].element_size, datas[i].type, GL_FALSE, 0, (void*)0);
				i++;
			});
		}

		void draw(prim_draw_type dt = prim_draw_type::triangle_list,
			int index_offset = 0, int oindex_count = -1, int vertex_offset = 0) override
		{
			glBindVertexArray(vtx_array);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glDrawRangeElementsBaseVertex((GLenum)dt, index_offset, (oindex_count == -1 ? idx_cnt : oindex_count), (oindex_count == -1 ? idx_cnt : oindex_count),
				gl_type_for_index_type<index_type>::value, (void*)0, vertex_offset);
		}

		~multistream_mesh() {
			glDeleteBuffers(1, &idx_buf);
		}
	}; 

	/*class model
	{
	protected:

		string _name;
	public:
		model(const string& n)
			: _name(n) 
		{
		}
		
		proprw(string, name, { return _name; });
		virtual void draw(function<void(uint, const mat4& m, void*)> update_shader =
			function<void(uint, const mat4& m, void*)>([&](uint, const mat4&, void*){}), const mat4& world = mat4(1),
			prim_draw_type pdt = prim_draw_type::triangle_list) = 0;
		
		~model()
		{
		}
		

	};

	//TODO: This class is bad, bad, bad! Performance should be terrible. At some point, this should be removed in favor of the now broken shared_data_model
	template <typename vertex_type, typename index_type>
	class simple_model : public model
	{
		struct model_part;
		vector<model_part> parts;
	public:
		struct sysmodel_part
		{
			sys_mesh<vertex_type, index_type> mesh;
			mat4 world;
			string name;
			void* ud;
			sysmodel_part(const sys_mesh<vertex_type, index_type>& m, const mat4& w, string n)
				: mesh(m), world(w), name(n), ud(nullptr) {}
		};
		struct model_part
		{
			GLuint vao;
			GLuint vbo;
			GLuint ibo;
			uint index_count;
			string name;
			mat4 world;
			void* ud;

			model_part() {}

			model_part(GLuint vao, GLuint v, GLuint i, uint ic, const string& n, const mat4& m)
				: vbo(v), ibo(i), index_count(ic), name(n), world(m), ud(nullptr) {}
			
			model_part(const sysmodel_part& p)
				: world(p.world), name(p.name), index_count(p.mesh.indices.size()), ud(p.ud)
			{
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);

				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, p.mesh.vertices.size()*sizeof(vertex_type), p.mesh.vertices.data(), GL_STATIC_DRAW);

				glGenBuffers(1, &ibo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, p.mesh.indices.size()*sizeof(index_type), p.mesh.indices.data(), GL_STATIC_DRAW);
				
				auto vabs = vertex_type::get_vertex_attribs();
				for (const auto& v : vabs)
				{
					glEnableVertexAttribArray(v.idx);
					glVertexAttribPointer(v.idx, v.count, v.type, GL_FALSE, sizeof(vertex_type), (void*)v.offset);
				}
			}

			~model_part()
			{
				//glDeleteBuffers(1, &vbo);
				//glDeleteBuffers(1, &ibo);
			//	glDeleteVertexArrays(1, &vao);
			}
		};

		model_part& operator[] (uint i)
		{
			return parts[i];
		}
		const model_part& operator[] (uint i) const
		{
			return parts[i];
		}

		simple_model(vector<sysmodel_part> sparts, string n)
			: model(n)
		{
			for(const auto& p : sparts)
			{
				parts.push_back(model_part(p));
			}
		}

		void draw(function<void(uint, const mat4& m, void*)> update_shader =
			function<void(uint, const mat4&, void*)>([&](uint, const mat4&, void*){}), const mat4& world = mat4(1),
			prim_draw_type pdt = prim_draw_type::triangle_list) override
		{
			for (uint i = 0; i < parts.size(); ++i)
			{
				const auto& p = parts[i];
				glBindVertexArray(p.vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p.ibo);
				update_shader(i, world*p.world, p.ud);
				glDrawElements((GLenum)pdt, p.index_count, gl_type_for_index_type<index_type>::value, (void*)0);
			}
		}

		~simple_model()
		{
			for(auto& mp : parts)
			{
				glDeleteBuffers(1, &mp.vbo);
				glDeleteBuffers(1, &mp.ibo);
				glDeleteVertexArrays(1, &mp.vao);
			}
		}

	};

	//TODO: Fix this!
	//TODO: This class is broken, no idea where
	template <typename vertex_type, typename index_type>
	class shared_data_model : public model
	{
		struct model_part;
		GLuint vtx_array;
		GLuint vtx_buf;
		GLuint idx_buf;
		vector<model_part> parts;
	public:
		struct model_part
		{
			uint index_offset;
			uint index_count;
			uint vertex_offset;
			mat4 world;
			string name;
			model_part(uint io, uint ic, uint vo, const mat4& w, const string& n = "")
				: index_offset(io), index_count(ic), vertex_offset(vo), world(w), name(n)  {}
		};

		struct sys_model_part
		{
			sys_mesh<vertex_type, index_type> mesh;
			mat4 world;
			string name;
		};

		model_part& operator[] (int i)
		{
			return parts[i];
		}

		const model_part& operator[] (int i) const
		{
			return parts[i];
		}

		shared_data_model(const vector<sys_model_part>& mss, const string& n)
			: model(n)
		{
			glGenVertexArrays(1, &vtx_array);
			glBindVertexArray(vtx_array);
			vector<vertex_type> vs;
			vector<index_type> is;
			for(const auto& s : mss)
			{
				parts.push_back(model_part(is.size(), s.mesh.indices.size(), vs.size(), s.world, s.name)); //TODO: Get real world mats at init
				for (const auto& _v : s.mesh.vertices) vs.push_back(_v);	
				for (const auto& _i : s.mesh.indices) is.push_back(_i);
			}

			glGenBuffers(1, &vtx_buf);
			glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
			glBufferData(GL_ARRAY_BUFFER, vs.size()*sizeof(vertex_type), vs.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &idx_buf);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, is.size()*sizeof(index_type), is.data(), GL_STATIC_DRAW);

			auto vabs = vertex_type::get_vertex_attribs();
			for (const auto& v : vabs)
			{
				glEnableVertexAttribArray(v.idx);
				glVertexAttribPointer(v.idx, v.count, v.type, GL_FALSE, sizeof(vertex_type), (void*)v.offset);
			}
		}

		void draw(function<void(uint, const mat4& m, void*)> update_shader = 
					function<void(uint,const mat4& m, void*)>([&](uint,const mat4&, void*){}), const mat4& world = mat4(1),
				prim_draw_type pdt = prim_draw_type::triangle_list) override
		{
			glBindVertexArray(vtx_array);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			for (uint i = 0; i < parts.size(); ++i)
			{
				const auto& p = parts[i];
				update_shader(i, world*p.world, nullptr);
				glDrawRangeElementsBaseVertex((GLenum)pdt, p.index_offset, p.index_offset+p.index_count, p.index_count,
					gl_type_for_index_type<index_type>::value, (void*)0, p.vertex_offset);
			}
		}
		propr(GLuint, vertex_array, { return vtx_array; });

		~shared_data_model()
		{
			glDeleteBuffers(1, &vtx_buf);
			glDeleteBuffers(1, &idx_buf);
			glDeleteVertexArrays(1, &vtx_array);
		}
	};*/

	template <typename vertex_type, typename index_type>
	sys_mesh<vertex_type,index_type> generate_sphere(float radius, uint slice_count, uint stack_count)
	{
		sys_mesh<vertex_type, index_type> m;

		m.vertices.push_back(vertex_type(vec3(0.f, radius, 0.f), vec3(0, 1, 0), vec3(1, 0, 0), vec2(0, 0)));

		float dphi = pi<float>() / stack_count;
		float dtheta = 2.f*pi<float>() / slice_count;

		for (uint i = 1; i <= stack_count - 1; ++i)
		{
			float phi = i*dphi;
			for (uint j = 0; j <= slice_count; ++j)
			{
				float theta = j*dtheta;
				vec3 p = vec3(radius*sinf(phi)*cosf(theta),
							  radius*cosf(phi),
							  radius*sinf(phi)*sinf(theta));
				vec3 t = normalize(vec3(-radius*sinf(phi)*sinf(theta),
					0.f,
					radius*sinf(phi)*cosf(theta)));
				m.vertices.push_back(vertex_type(p, normalize(p), t, vec2(theta / (2.f*pi<float>()), phi / (2.f*pi<float>()))));

			}
		}

		m.vertices.push_back(vertex_type(vec3(0.f, -radius, 0.f), vec3(0, -1, 0), vec3(1, 0, 0), vec2(0, 1)));

		for (index_type i = 1; i <= slice_count; ++i)
		{
			m.indices.push_back(0);
			m.indices.push_back(i + 1);
			m.indices.push_back(i);
		}

		index_type bi = 1;
		index_type rvc = slice_count + 1;
		for (index_type i = 0; i < stack_count - 2; ++i)
		{
			for (uint j = 0; j < slice_count; ++j)
			{
				m.indices.push_back(bi + i*rvc + j);
				m.indices.push_back(bi + i*rvc + j + 1);
				m.indices.push_back(bi + (i + 1)*rvc + j);
				m.indices.push_back(bi + (i + 1)*rvc + j);
				m.indices.push_back(bi + (i*rvc + j + 1));
				m.indices.push_back(bi + (i + 1)*rvc + j + 1);
			}
		}

		index_type spi = (index_type)m.vertices.size() - 1;
		bi = spi - rvc;
		for (uint i = 0; i < slice_count; ++i)
		{
			m.indices.push_back(spi);
			m.indices.push_back(bi + i);
			m.indices.push_back(bi + i + 1);
		}

		return m;
	}
	
	template <typename vertex_type, typename index_type>
	sys_mesh<vertex_type, index_type> generate_plane(vec2 dims, vec2 div, vec3 norm = vec3(0,1,0))
	{
		sys_mesh<vertex_type, index_type> m;

		vec3 nw = normalize(norm);
		vec3 t = (fabsf(nw.x) > .1 ? vec3(0, 1, 0) : vec3(1, 0, 0));
		vec3 nu = normalize(cross(t, nw));
		vec3 nv = cross(nw, nu);

		vec2 hdims = .5f*dims;

		vec2 dxy = dims / (div - vec2(1));

		vec2 duv = 1.f / (div - vec2(1));

		for (float i = 0; i < div.y; ++i)
		{
			float y = hdims.y - i*dxy.y;
			for (float j = 0; j < div.x; ++j)
			{
				float x = hdims.x - j*dxy.x;
				
				vec3 p = nu*x + nv*y;

				m.vertices.push_back(vertex_type(p, -nw, nu, vec2(j,i)*duv));
			}
		}

		for (uint i = 0; i < div.x - 1; ++i)
		{
			for (uint j = 0; j < div.y - 1; ++j)
			{
				m.indices.push_back(i*(uint16)div.y + j);
				m.indices.push_back(i*(uint16)div.y + j + 1);
				m.indices.push_back((i + 1)*(uint16)div.y + j);

				m.indices.push_back((i + 1)*(uint16)div.y + j);
				m.indices.push_back(i*(uint16)div.y + j + 1);
				m.indices.push_back((i + 1)*(uint16)div.y + j + 1);
			}
		}

		reverse(m.indices.begin(), m.indices.end());

		return m;
	}

	template <typename vertex_type, typename index_type>
	sys_mesh<vertex_type, index_type> generate_torus(vec2 r, int div)
	{
		int ring_count = div;
		int stack_count = div;
		sys_mesh<vertex_type, index_type> m;

		vector<vertex_position_normal_tangent_texture> frvtx;
		for (int i = 0; i < div + 1; ++i)
		{
			vec4 p = vec4(r.y, 0.f, 0.f,1.f)*rotate(mat4(1), radians(i*360.f / (float)div), vec3(0, 0, 1))
				+ vec4(r.x, 0.f, 0.f, 1.f);
			vec2 tx = vec2(0, (float)i / div);
			vec4 tg = vec4(0.f, -1.f, 0.f, 1.f)*rotate(mat4(1), radians(i*360.f / (float)div), vec3(0, 0, 1));
			vec3 n = cross(vec3(tg), vec3(0.f, 0.f, -1.f));
			m.vertices.push_back(vertex_type(vec3(p), vec3(n), vec3(tg), tx));
			frvtx.push_back(vertex_position_normal_tangent_texture(vec3(p), n, vec3(tg), tx));
		}

		for (int ring = 1; ring < ring_count + 1; ++ring)
		{
			mat4 rot = rotate(mat4(1), radians(ring*360.f / (float)div), vec3(0, 1, 0));
			for (int i = 0; i < stack_count + 1; ++i)
			{
				vec4 p = vec4(frvtx[i].pos.x, frvtx[i].pos.y, frvtx[i].pos.z, 1.f);
				vec4 nr = vec4(frvtx[i].norm.x, frvtx[i].norm.y, frvtx[i].norm.z, 0.f);
				vec4 tg = vec4(frvtx[i].tang.x, frvtx[i].tang.y, frvtx[i].tang.z, 0.f);
				p = p*rot;
				nr = nr*rot;
				tg = tg*rot;

				m.vertices.push_back(vertex_type(vec3(p), vec3(nr),
					vec3(tg), vec2(2.f*ring / (float)div, frvtx[i].tex.y)));
			}
		}

		for (int ring = 0; ring < ring_count; ++ring)
		{
			for (int i = 0; i < stack_count; ++i)
			{
				m.indices.push_back(ring*(div + 1) + i);
				m.indices.push_back((ring+1)*(div + 1) + i);
				m.indices.push_back(ring*(div + 1) + i +1);

				m.indices.push_back(ring*(div + 1) + i + 1);
				m.indices.push_back((ring+1)*(div + 1) + i);
				m.indices.push_back((ring+1)*(div + 1) + i + 1);
			}
		}
		
		return m;
	}

	template <typename vertex_type, typename index_type>
	sys_mesh<vertex_type, index_type> generate_screen_quad(vec2 offset, vec2 size)
	{
		sys_mesh<vertex_type, index_type> m;

		const vec2 z[] =
		{
			vec2(1, 1),
			vec2(1, -1),
			vec2(-1, -1),
			vec2(-1, 1),
		};
		const vec2 t[] =
		{
			vec2(0, 0),
			vec2(0, 1),
			vec2(1, 1),
			vec2(1, 0),
		};

		m.vertices.push_back(vertex_type(vec3(offset + size*z[0], 0), vec3(0, 1, 0),
			vec3(1, 0, 0), t[0]));
		m.vertices.push_back(vertex_type(vec3(offset + size*z[1], 0), vec3(0, 1, 0),
			vec3(1, 0, 0), t[1]));
		m.vertices.push_back(vertex_type(vec3(offset + size*z[2], 0), vec3(0, 1, 0),
			vec3(1, 0, 0), t[2]));
		m.vertices.push_back(vertex_type(vec3(offset + size*z[3], 0), vec3(0, 1, 0),
			vec3(1, 0, 0), t[3]));

		m.indices.push_back(0);
		m.indices.push_back(1);
		m.indices.push_back(2);

		m.indices.push_back(2);
		m.indices.push_back(3);
		m.indices.push_back(0);

		return m;
	}

	//mutable_interleaved_mesh<vertex_type, index_type>
	//	interleaved_mesh is a mesh that uses interleaved vertices, where each vertex is a struct
	//	vertex_type: vertex data, as a struct. see vertex_position and vertex_position_normal_texture for examples of implementation. 
	//	index_type:  type for index values. uint16 or uint32 usually
	//	mutable_interleaved_mesh allows for updating the mesh from the CPU
	template<typename vertex_type, typename index_type> class mutable_interleaved_mesh : public mesh
	{
		uint idx_cnt;
		uint vtx_cnt;
		GLuint vtx_buf;
		GLuint idx_buf;
	public:
		mutable_interleaved_mesh(const vector<vertex_type>& vs, const vector<index_type>& is)
			: mesh(), vtx_cnt(vs.size()), idx_cnt(is.size())
		{
			glBindVertexArray(vtx_array);
			glGenBuffers(1, &vtx_buf);
			glGenBuffers(1, &idx_buf);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, is.size()*sizeof(index_type), is.data(), GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
			glBufferData(GL_ARRAY_BUFFER, vs.size()*sizeof(vertex_type), vs.data(), GL_DYNAMIC_DRAW);

			auto vabs = vertex_type::get_vertex_attribs();
			for (const auto& v : vabs)
			{
				glEnableVertexAttribArray(v.idx);
				glVertexAttribPointer(v.idx, v.count, v.type, GL_FALSE, sizeof(vertex_type), (void*)v.offset);
			}
		}

		mutable_interleaved_mesh(device* _dev, const sys_mesh<vertex_type, index_type>& gm)
			: mutable_interleaved_mesh(_dev, gm.vertices, gm.indices)
		{}

		void update(device* _dev, const vector<vertex_type>& vs)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
			glBufferData(GL_ARRAY_BUFFER, vs.size()*sizeof(vertex_type), vs.data(), GL_DYNAMIC_DRAW);
		}

		void update(device* _dev, const vector<vertex_type>& vs, const vector<index_type>& is)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, is.size()*sizeof(index_type), is.data(), GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
			glBufferData(GL_ARRAY_BUFFER, vs.size()*sizeof(vertex_type), vs.data(), GL_DYNAMIC_DRAW);
		}


		void update(device* _dev, const sys_mesh<vertex_type, index_type>& gm)
		{
			update(_dev, gm.vertices, gm.indices);
		}
		
		void draw(prim_draw_type dt = prim_draw_type::triangle_list,
			int index_offset = 0, int oindex_count = -1, int vertex_offset = 0) override
		{
			glBindVertexArray(vtx_array);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buf);
			glDrawRangeElementsBaseVertex((GLenum)dt, index_offset, (oindex_count == -1 ? idx_cnt : index_offset + oindex_count), (oindex_count == -1 ? idx_cnt : oindex_count),
				gl_type_for_index_type<index_type>::value, (void*)0, vertex_offset);

		}


		~mutable_interleaved_mesh()
		{
			glDeleteBuffers(1, &vtx_buf);
			glDeleteBuffers(1, &idx_buf);
		}


		propr(uint, index_count, { return idx_cnt; });
		propr(uint, vertex_count, { return vtx_cnt; });

		propr(GLuint, vertex_buffer, { return vtx_buf; });
		propr(GLuint, index_buffer, { return idx_buf; });

	};
}


