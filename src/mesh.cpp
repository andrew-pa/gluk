#include "mesh.h"

namespace gluk
{

	vector<vertex_attrib> vertex_position::get_vertex_attribs()
	{
		static vector<vertex_attrib> va = 
		{
			{ 0, 3, GL_FLOAT, offsetof(vertex_position, pos) },
		};
		return va;
	}

	vector<vertex_attrib> vertex_position_normal_texture::get_vertex_attribs()
	{
		static vector<vertex_attrib> va =
		{
			{ 0, 3, GL_FLOAT, offsetof(vertex_position_normal_texture, pos) },
			{ 1, 3, GL_FLOAT, offsetof(vertex_position_normal_texture, norm) },
			{ 2, 2, GL_FLOAT, offsetof(vertex_position_normal_texture, tex) },
		};
		return va;
	}

	vector<vertex_attrib> vertex_position_normal_tangent_texture::get_vertex_attribs()
	{
		static vector<vertex_attrib> va =
		{
			{ 0, 3, GL_FLOAT, offsetof(vertex_position_normal_tangent_texture, pos) },
			{ 1, 3, GL_FLOAT, offsetof(vertex_position_normal_tangent_texture, norm) },
			{ 2, 3, GL_FLOAT, offsetof(vertex_position_normal_tangent_texture, tang) },
			{ 3, 2, GL_FLOAT, offsetof(vertex_position_normal_tangent_texture, tex) },
		};
		return va;
	}


	mesh::mesh(const string& n)
		: _name(n)
	{
		glGenVertexArrays(1, &vtx_array);
	}
	mesh::~mesh()
	{
		//glDeleteVertexArrays(1, &vtx_array); TODO: why is the commented out in libQEG?
	}
}
