#include "cmmn.h"
#include "app.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "render_target.h"
#include "util.h"
#include "renderer2d.h"

using namespace gluk;

#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


inline vec3 to_glm3(const aiVector3D& v) { return vec3(v.x, v.y, v.z); }
inline vec2 to_glm2(const aiVector3D& v) { return vec2(v.x, v.y); }
inline vec3 to_glm3(const aiColor3D& v) { return vec3(v.r, v.g, v.b); }
inline vec4 to_glm4(const aiColor4D& v) { return vec4(v.r, v.g, v.b, v.a); }

struct model_part {
	mat4 world;
	shared_ptr<mesh> msh;
	vec3 diffuse_color;
	vec3 specular_color; float shininess;
	shared_ptr<texture2d> tex;

	model_part(const aiScene* sc, uint32_t idx, const package& pak) : world(1) {
		
		auto mh = sc->mMeshes[idx];

		auto mat = sc->mMaterials[mh->mMaterialIndex];
		aiColor3D c;
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, c);
		diffuse_color = to_glm3(c);
		mat->Get(AI_MATKEY_COLOR_SPECULAR, c);
		specular_color = to_glm3(c);
		mat->Get(AI_MATKEY_SHININESS, shininess);
		aiString pth;
		if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &pth) == aiReturn_SUCCESS) {
			tex = make_shared<texture2d>(pak.open(pth.C_Str()));
		}

		vector<uint32_t> is;
		for(int ix = 0; ix < mh->mNumFaces; ++ix) 
			for(int jx = 0; jx < mh->mFaces[ix].mNumIndices; ++jx) 
				is.push_back(mh->mFaces[ix].mIndices[jx]);

		vector<multistream_mesh_stream_desc> descs;
		if(mh->HasPositions()) {
			descs.push_back(multistream_mesh_stream_desc(mh->mVertices, mh->mNumVertices*sizeof(aiVector3D), 3, GL_FLOAT));
			if(mh->HasNormals()) {
				descs.push_back(multistream_mesh_stream_desc(mh->mNormals, mh->mNumVertices*sizeof(aiVector3D), 3, GL_FLOAT));
				if(mh->HasTextureCoords(0)) {
					unique_ptr<vec2> texc = unique_ptr<vec2>(new vec2[mh->mNumVertices]);
					for(int i = 0; i < mh->mNumVertices; ++i) texc.get()[i] = to_glm2(mh->mTextureCoords[0][i]);
					descs.push_back(multistream_mesh_stream_desc(texc.get(), mh->mNumVertices*sizeof(vec2), 2, GL_FLOAT));
					if(mh->HasTangentsAndBitangents()) {
						descs.push_back(multistream_mesh_stream_desc(mh->mTangents, mh->mNumVertices*sizeof(aiVector3D), 3, GL_FLOAT));
						msh = make_shared<multistream_mesh<uint32, vec3, vec3, vec2, vec3>>(is, descs);
					}
					else msh = make_shared<multistream_mesh<uint32, vec3, vec3, vec2>>(is, descs);
				}
				else msh = make_shared<multistream_mesh<uint32, vec3, vec3>>(is, descs);
			} 
			else msh = make_shared<multistream_mesh<uint32, vec3>>(is, descs);
		} else throw;
	}

	void draw(const mat4& lw, shader& s) {
		s.set_uniform("world", lw*world);
		s.set_uniform("diffuse_color", diffuse_color);
		s.set_uniform("specular_color", specular_color);
		s.set_uniform("shininess", shininess);
		if(tex) s.set_texture("diffuse_texture", *tex.get(), 0);
		msh->draw();
	}
};

static package sample_package = package(SMP_PATH);

struct simple_app : public app, public input_handler {
	package media_package;

	mesh *torus, *sphere, *floor;
	vector<unique_ptr<model_part>> car;
		
	unique_ptr<shader> s;
	util::perspective_camera cam;
	util::fps_camera_controller* fpscamctrl;

	render_texture2d* intrm;

	unique_ptr<util::full_screen_quad_shader> postprocess_s;

	simple_app() : 
		app("car viewer", vec2(800, 600), 1),
		media_package(sample_package, "car/"),
		s(new shader(dev, media_package, "basic.vs.glsl", "basic.ps.glsl")),
		postprocess_s(new util::full_screen_quad_shader(dev, media_package.open("postprocess.ps.glsl"))),
		cam(dev->size(), vec3(0.01, 3.f, -7.f), vec3(0.f), vec3(0.f, 1.f, 0.f)),
		intrm(nullptr)
	{
		create_size_dpe_res();
		torus = new interleaved_mesh<vertex_position_normal_texture, uint16>(
			       generate_torus<vertex_position_normal_texture,uint16>(vec2(1.f, 0.5f), 64));
		sphere = new interleaved_mesh<vertex_position_normal_texture, uint32>(
				generate_sphere<vertex_position_normal_texture, uint32>(1.f, 32, 32));
		floor = new interleaved_mesh<vertex_position_normal_texture, uint32>(
				generate_plane<vertex_position_normal_texture, uint32>(vec2(16.f), vec2(8.f), vec3(0.f, -1.f, 0.f)));	
		
		Assimp::Importer imp;
		auto scn = imp.ReadFile(media_package.path_of("dodge_challenger\\car.fbx"), aiProcessPreset_TargetRealtime_Fast);
		for(int i = 0; i < scn->mNumMeshes; ++i)
		       car.push_back(unique_ptr<model_part>(
				new model_part(scn, i, media_package)));
		
		fpscamctrl = new util::fps_camera_controller(cam);
		input_handlers.push_back(fpscamctrl);
		input_handlers.push_back(this);
	}

	void resize() override {
		cam.update_proj(dev->size());
		create_size_dpe_res();
	}
	void create_size_dpe_res() {
		if(intrm != nullptr) delete intrm;
		intrm = new render_texture2d(uvec2(dev->size()));
	}


	void update(float t, float dt) override { 
		fpscamctrl->update(t, dt);
	}
	void key_handler(app* a, uint kc, input_action ac, input_mod md) {
		if(ac == input_action::release) {
			if(kc == GLFW_KEY_F3) {
				s.reset();
				s = unique_ptr<shader>(
					new shader(dev, media_package, "basic.vs.glsl", "basic.ps.glsl"));
				postprocess_s.reset();
				postprocess_s = unique_ptr<util::full_screen_quad_shader>(new util::full_screen_quad_shader(dev, media_package.open("postprocess.ps.glsl")));
			}
		}
	}
	void render(float t, float dt) override { 
		dev->push_render_target(intrm);
		s->bind();
		s->set_uniform("view_proj", cam.proj()*cam.view());
		s->set_uniform("camera_position", cam.position());
		s->set_uniform("light_direction", vec3(0.f, 1.f, 0.f));
		
		s->set_uniform("world", mat4(1));
		s->set_uniform("diffuse_color", vec3(0.2f, 0.2f, 0.1f));
		floor->draw();

		s->set_uniform("world", 
			rotate(translate(mat4(1), vec3(-2.f, 2.f, 0.f)), t, vec3(1.f, 0.3f, 0.6f)));
		s->set_uniform("diffuse_color", vec3(0.8f, 0.6f, 0.1f));
		torus->draw();
		
		s->set_uniform("world", translate(mat4(1), vec3(2.f, 1.f, 0.f)));
		s->set_uniform("diffuse_color", vec3(0.3f, 0.6f, 0.8f));
		sphere->draw();

		mat4 ct = translate(mat4(1), vec3(2.f, 1.f, 4.f));
		for(auto& p : car) p->draw(ct, *s);
		
		dev->pop_render_target();
		postprocess_s->bind();
		postprocess_s->set_texture("backbuffer", *intrm, 0);
		postprocess_s->draw(t);
	}	
};

int main() {
	simple_app a;
	a.run();
}
