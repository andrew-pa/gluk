#pragma once
#include "device.h"
#include "shader.h"
#include "texture.h"
#include "mesh.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include "freetype-gl.h"

namespace gluk {
	namespace graphics2d {
		class renderer2d;
		class font {
			//FT_Face ft_face;
			freetype_gl::texture_atlas_t* atlas;
			freetype_gl::texture_font_t* fnt;
		public:
			friend class renderer2d;
			font(renderer2d& _rndr, const string& name, float size);
			font(renderer2d& _rndr, const filedatasp data, float size);
			~font();
			//void set_size(float char_height);

		};

		class renderer2d {
			device* dev;
			shader shdr;

			struct quad_instance {
				vec4 screen_rect;
				vec4 texture_rect;
				vec4 color;
				vec4 texid_istext;
				quad_instance(vec2 of, vec2 s, vec2 tof, vec2 ts, const vec4& col, uint tid, bool istext) 
				: screen_rect(of,s), texture_rect(tof,ts), color(col), texid_istext((float)tid, istext?1.f:0.f,0.f,0.f) {}
				quad_instance(vec2 of, vec2 s, const vec4& tofs, const vec4& col, uint tid, bool istext)
					: screen_rect(of, s), texture_rect(tofs), color(col), texid_istext((float)tid, istext ? 1.f : 0.f, 0.f, 0.f) {}

				static vector<vertex_attrib> get_vertex_attribs();
			};
			instanced_interleaved_mesh<vertex_position_normal_texture,uint8,quad_instance>* quad;
			vector<quad_instance> quads;
			vector<GLuint> textures;

			//FT_Library ft_lib;
			bool blend_enabled;
		public:
			vec2 dpi;
			renderer2d(device* _dev, const package& pak);

			void begin_draw();
			void flush();
			void end_draw();

			void enable_blend();
			void disable_blend();
			
			void draw_rect(vec2 offset, vec2 size, const vec4& col = vec4(1.f), const texture<2>* tex = nullptr);
			
			void draw_string(vec2 offset, const wstring& str, font& fnt, const vec4& col = vec4(1.f));
			
			void update_dpi();

			friend class font;
		};
	}
}
