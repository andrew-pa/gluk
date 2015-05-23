#include "renderer2d.h"


namespace gluk {
	namespace graphics2d {
		renderer2d::renderer2d(device* _dev, const package& pak)
			: dev(_dev), shdr(_dev, pak, "renderer2d.vs.glsl", "renderer2d.ps.glsl"), blend_enabled(true) {
			quad = new interleaved_mesh<vertex_position_normal_texture, uint8>(
				generate_screen_quad<vertex_position_normal_texture,uint8>(vec2(0.f), vec2(1.f)), "");
			FT_Init_FreeType(&ft_lib);
			
			update_dpi();
		}

		void renderer2d::update_dpi() {
			/*const float mm_to_inch = 1.f / 25.4f;
			auto mon = glfwGetPrimaryMonitor();
			auto mod = glfwGetVideoMode(mon);
			ivec2 physical_size;
			glfwGetMonitorPhysicalSize(mon, &physical_size.x, &physical_size.y);
			dpi = vec2(mod->width, mod->height) / (((vec2)physical_size)*mm_to_inch);*/
			//glfw is crazy
			dpi = vec2(96.f);
		}

		void renderer2d::enable_blend() {
			if(!blend_enabled) {
				blend_enabled = true;

				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}

		void renderer2d::disable_blend() {
			if(blend_enabled)
				glDisable(GL_BLEND);
		}
		
		void renderer2d::begin_draw() {
			glDisable(GL_CULL_FACE);	
			glDisable(GL_DEPTH_TEST);
			if(blend_enabled) {
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			shdr.bind();
		}

		void renderer2d::draw_rect(vec2 offset, vec2 size, const vec4& col, const texture<2>* tex) {
			//shdr.set_uniform("trnf", trf);
			shdr.set_uniform("use_texture", tex != nullptr);
			shdr.set_uniform("vcolor", col);
			shdr.set_uniform("size", size);
			shdr.set_uniform("offset", offset);
			shdr.set_uniform("invres", 1.f / dev->size());
			if (tex != nullptr) shdr.set_texture("tex", *tex, 0);
			quad->draw();
		}
		
		void renderer2d::draw_string(vec2 offset, const string& str, font& fnt, const vec4& col) {
			uint previous = 0;
			vec2 pen = offset;
			shdr.set_uniform("is_text", true);
			for(const auto c : str) {
				auto idx = FT_Get_Char_Index(fnt.ft_fc, c);
				auto cgly = fnt.glyph_cashe.find(idx);
				if(cgly == fnt.glyph_cashe.end()) {
					fnt.glyph_cashe[idx] = font::cashed_glyph(fnt, idx, previous);
					cgly = fnt.glyph_cashe.find(idx);
				}
				pen.x += cgly->second.kerning_delta.x;
				draw_rect(pen + cgly->second.offset, cgly->second.tex->size(), col, cgly->second.tex);
				pen.x += cgly->second.size.x*2.f;
				previous = idx;
			}
			shdr.set_uniform("is_text", false);
		}
	
		void renderer2d::end_draw() {
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			if(blend_enabled) glDisable(GL_BLEND);
		}

		font::font(renderer2d& _rndr, const string& name, float size) : rndr(&_rndr) {
			FT_New_Face(rndr->ft_lib, name.c_str(), 0, &ft_fc);
			ft_glyslot = ft_fc->glyph;
			has_kerning = FT_HAS_KERNING(ft_fc);
			set_size(size);
		}
		font::font(renderer2d& _rndr, const filedatasp data, float size) : rndr(&_rndr) {
			FT_New_Memory_Face(rndr->ft_lib, data->data<FT_Byte>(), data->length(), 0, &ft_fc);
			ft_glyslot = ft_fc->glyph;
			has_kerning = FT_HAS_KERNING(ft_fc);
			set_size(size);
		}

		void font::set_size(float char_height) {
			FT_Set_Char_Size(ft_fc, 0, char_height*64.f,
				rndr->dpi.x, rndr->dpi.y);
		}

		font::cashed_glyph::cashed_glyph(font& f, uint idx, uint previous_idx) {
			if (f.has_kerning && previous_idx && idx) {
				FT_Vector delta;
				FT_Get_Kerning(f.ft_fc, previous_idx, idx, FT_KERNING_DEFAULT, &delta);
				kerning_delta = vec2(delta.x >> 6, delta.y >> 6);
			}
			FT_Load_Glyph(f.ft_fc, idx, FT_LOAD_RENDER);
			offset = vec2(f.ft_glyslot->bitmap_left, f.ft_glyslot->bitmap_top);
			size = vec2(f.ft_glyslot->advance.x >> 6, f.ft_glyslot->advance.y >> 6);
			ivec2 bitmap_size = ivec2(f.ft_glyslot->bitmap.width, f.ft_glyslot->bitmap.rows);
			
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			tex = new texture2d(pixel_format(pixel_components::r, pixel_type::unorm, 8),
				bitmap_size, f.ft_glyslot->bitmap.buffer);
			tex->wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}
	}
}