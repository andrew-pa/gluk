#pragma once
#include "device.h"
#include "shader.h"
#include "texture.h"
#include "mesh.h"

#include "ft2build.h"
#include FT_FREETYPE_H

namespace gluk {
	namespace graphics2d {
		class renderer2d;
		class font {
			FT_Face ft_fc;
			FT_GlyphSlot ft_glyslot;
			bool has_kerning;
			renderer2d* rndr;
			struct cashed_glyph {
				texture2d* tex;
				vec2 size;
				vec2 offset;
				vec2 kerning_delta;
				cashed_glyph(font& f, uint idx, uint previous_idx);
				cashed_glyph() : tex(nullptr) {}
			};
			map<uint32, cashed_glyph> glyph_cashe;
		public:
			friend class renderer2d;
			font(renderer2d& _rndr, const string& name, float size);
			font(renderer2d& _rndr, const filedatasp data, float size);

			void set_size(float char_height);
		};

		class renderer2d {
			device* dev;
			shader shdr;
			mesh* quad;
			FT_Library ft_lib;
			bool blend_enabled;
		public:
			vec2 dpi;
			renderer2d(device* _dev, const package& pak);

			void begin_draw();
			void end_draw();

			void enable_blend();
			void disable_blend();
			
			void draw_rect(vec2 offset, vec2 size, const vec4& col = vec4(1.f), const texture<2>* tex = nullptr);
			
			void draw_string(vec2 offset, const string& str, font& fnt, const vec4& col = vec4(1.f));
			
			void update_dpi();

			friend class font;
		};
	}
}