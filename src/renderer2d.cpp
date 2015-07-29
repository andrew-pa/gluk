#include "renderer2d.h"


namespace gluk {
	namespace graphics2d {
		renderer2d::renderer2d(device* _dev, const package& pak)
			: dev(_dev), shdr(_dev, pak, "renderer2d.vs.glsl", "renderer2d.ps.glsl"), blend_enabled(true) {
			quad = new instanced_interleaved_mesh<vertex_position_normal_texture, uint8, quad_instance>(
				generate_screen_quad<vertex_position_normal_texture,uint8>(vec2(0.f), vec2(1.f)));
			//FT_Init_FreeType(&ft_lib);
			
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
		
		vector<vertex_attrib> renderer2d::quad_instance::get_vertex_attribs() {
			static vector<vertex_attrib> va =
			{
				{ 0, 4, GL_FLOAT, offsetof(quad_instance, screen_rect) },
				{ 1, 4, GL_FLOAT, offsetof(quad_instance, texture_rect) },
				{ 2, 4, GL_FLOAT, offsetof(quad_instance, color) },
				{ 3, 4, GL_FLOAT, offsetof(quad_instance, texid_istext) },
			};
			return va;
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
			/*shdr.set_uniform("use_texture", tex != nullptr);
			shdr.set_uniform("vcolor", col);
			shdr.set_uniform("size", size);
			shdr.set_uniform("offset", offset);
			shdr.set_uniform("invres", 1.f / dev->size());
			if (tex != nullptr) shdr.set_texture("tex", *tex, 0);
			quad->draw();*/
			uint tid = 0;
			if (tex != nullptr) {
				auto f = find_if(textures.begin(), textures.end(), [&] (GLuint v) {
					return v == tex->id();
				});
				if(f == textures.end()) {
					if(textures.size() >= 32) flush();
					tid = textures.size();
					textures.push_back(tex->id());
				} else {
					tid = std::distance(textures.begin(), f);
				}
			}
			quads.push_back(quad_instance(offset, size, vec2(0.f), vec2(1.f), col, (tex == nullptr ? 256 : tid), false));
		}
		
		void renderer2d::draw_string(vec2 offset, const wstring& str, font& fnt, const vec4& col) {
			uint previous = 0;
			vec2 pen = offset;			
			float line_blext = -numeric_limits<float>::infinity();
			uint tid = 0;
			auto f = find_if(textures.begin(), textures.end(), [&](GLuint v) {
				return v == fnt.atlas->id;
			});
			if (f == textures.end()) {
				if (textures.size() >= 32) flush();
				tid = textures.size();
				textures.push_back(fnt.atlas->id);
			}
			else {
				tid = std::distance(textures.begin(), f);
			}

			for (int i = 0; i < str.length(); ++i) {
				if(str[i] == '\n') {
					pen.x = offset.x;
					pen.y -= line_blext*1.1f;
					line_blext = -numeric_limits<float>::infinity();
					continue;
				}
				freetype_gl::texture_glyph_t* gly =
					freetype_gl::texture_font_get_glyph(fnt.fnt, str[i]);
				if(gly != nullptr) {
					pen.x += (i>0) ?
						freetype_gl::texture_glyph_get_kerning(gly, str[i - 1]) : 0.f;
					line_blext = glm::max(line_blext, 2.f*(float)gly->height);
					quads.push_back(quad_instance(pen + vec2(gly->offset_x, -2.f*((float)gly->height - gly->offset_y)),
						vec2(gly->width, gly->height), vec4(gly->s0, gly->t0, gly->s1-gly->s0, gly->t1-gly->t0), col, tid, true));
					pen.x += gly->advance_x*2.f;
				}
			}
		}

		void renderer2d::flush() {
			shdr.set_uniform("invres", 1.f / dev->size());
			shdr.set_uniform("screen_offset", -dev->size());
			int i = 0;
			for (const auto& t : textures) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, t);
				shdr.set_uniform_array("textures", i, i);
				i++;
			}
			quad->update_instance_buffer(quads);
			quad->draw(prim_draw_type::triangle_list, quads.size());
			i = 0;
			for (const auto& t : textures) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
				i++;
			}
			quads.clear();
			textures.clear();
		}
	
		void renderer2d::end_draw() {
			flush();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			if(blend_enabled) glDisable(GL_BLEND);
		}
		
		font::font(renderer2d& _rndr, const string& name, float size) {
			atlas = freetype_gl::texture_atlas_new(512, 512, 1);
			fnt = freetype_gl::texture_font_new_from_file(atlas, size, name.c_str());
		}

		font::font(renderer2d& _rndr, const filedatasp data, float size) {
			atlas = freetype_gl::texture_atlas_new(512, 512, 1);
			fnt = freetype_gl::texture_font_new_from_memory(atlas, size, data->data<void>(), data->length());
		}

		font::~font() {
			freetype_gl::texture_font_delete(fnt);
			freetype_gl::texture_atlas_delete(atlas);
		}
		

		/*font::font(renderer2d& _rndr, const string& name, float size) 
			: rndr(&_rndr), atlas(new texture_atlas(pixel_format(pixel_components::r, pixel_type::unorm, 8), 9))
		{
			FT_New_Face(rndr->ft_lib, name.c_str(), 0, &ft_fc);
			ft_glyslot = ft_fc->glyph;
			has_kerning = FT_HAS_KERNING(ft_fc);
			set_size(size);
			atlas->bind(0);
			atlas->wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}
		font::font(renderer2d& _rndr, const filedatasp data, float size) 
			: rndr(&_rndr), atlas(new texture_atlas(pixel_format(pixel_components::r, pixel_type::unorm, 8), 9))
		{
			FT_New_Memory_Face(rndr->ft_lib, data->data<FT_Byte>(), data->length(), 0, &ft_fc);
			ft_glyslot = ft_fc->glyph;
			has_kerning = FT_HAS_KERNING(ft_fc);
			set_size(size);
			atlas->bind(0);
			atlas->wrap(GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}

		void font::set_size(float char_height) {
			FT_Set_Char_Size(ft_fc, 0, char_height*64.f,
				rndr->dpi.x, rndr->dpi.y);
		}

		font::cashed_glyph::cashed_glyph(font& f, uint idx, uint previous_idx) {
			FT_Load_Glyph(f.ft_fc, idx, FT_LOAD_RENDER);
			ivec2 bitmap_size = ivec2(f.ft_glyslot->bitmap.width, f.ft_glyslot->bitmap.rows);
			bitmapsize = vec2(bitmap_size);
			offset = vec2(f.ft_glyslot->bitmap_left, -(bitmapsize.y-f.ft_glyslot->bitmap_top) );
			advance = vec2(f.ft_glyslot->advance.x >> 6, f.ft_glyslot->advance.y >> 6);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			//tex = new texture2d(pixel_format(pixel_components::r, pixel_type::unorm, 8),
			//	bitmap_size, f.ft_glyslot->bitmap.buffer);
			char* buf = new char[abs(f.ft_glyslot->bitmap.pitch)*f.ft_glyslot->bitmap.rows];
			
			for (uint y = 0; y < f.ft_glyslot->bitmap.rows; ++y) {
				for (uint x = 0; x < f.ft_glyslot->bitmap.width; ++x) {
					uint nx = (f.ft_glyslot->bitmap.width-1) - x;
					buf[x + y*f.ft_glyslot->bitmap.pitch] = f.ft_glyslot->bitmap.buffer[nx + y*f.ft_glyslot->bitmap.pitch];
				}
			}
			tabounds = f.atlas->portions[f.atlas->add(bitmap_size, buf)];
			delete buf;
		}*/

	}
}