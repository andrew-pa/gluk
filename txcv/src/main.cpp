#define LINK
#include "cmmn.h"
#include "bo_file.h"
#include "resource.h"

#include "gli/gli.hpp"

using namespace gluk;

//txcv:	 reads in textures (DDS) and converts them into .tex files
//usage txcv <in_file> <out_file>

struct texture_header
{
	texture_dimension dim;
	uvec3 size;
	uint mip_count;
	uint array_count;
	pixel_format format;
};

pixel_format from_gli(gli::storage::format_type f)
{
	
}

bo_file* write_tex(gli::storage& s)
{
	if(s.layers() > 1)
	{
		throw exception("texture arrays not yet supported");
	}
	if(s.faces() > 1)
	{
		auto td = s.data();

		//cout << "[cubmap size=(" << s.dimensions(0).x << "x" << s.dimensions(0).y << ")]";

        texture_header hed;
        hed.dim = texture_dimension::texture_cube;
        hed.size = uvec3(s.dimensions(0));
        hed.mip_count = s.levels();
        hed.array_count = 6;
        hed.format = from_gli(s.format());

        bo_file* texf = new bo_file(bo_file::file_type::texture);
        bo_file::chunk hedc(0,
            new datablob<glm::byte>((glm::byte*)&hed, sizeof(texture_header)));
            
        for (int i = 0; i < 6; ++i)
        {
            glm::byte* dat = s.data() + i*s.faceSize(0, 0);

                
            bo_file::chunk dc(1,
                new datablob<glm::byte>(dat, s.faceSize(0, 0)));// *qeg::bytes_per_pixel(qeg::detail::convert(pif))));
            texf->chunks().push_back(dc);
        }
            
        //qeg::bo_file::chunk dc(1,
        //	new qeg::datablob<glm::byte>((glm::byte*)s.data(), s.size()));// *qeg::bytes_per_pixel(qeg::detail::convert(pif))));
        texf->chunks().push_back(hedc);
        //texf->chunks().push_back(dc);
        return texf;

    }
	else
	{
		auto pif = convert_pi(s.format());
		auto td = s.data();

		//cout << "[texture size=(" << s.dimensions(0).x << "x" << s.dimensions(0).y << "x" << s.dimensions(0).z << ")]";

		qeg::detail::texture_header hed;
		if      (s.dimensions(0).y == 0) hed.dim = qeg::texture_dimension::texture_1d;
		else if (s.dimensions(0).z == 1) hed.dim = qeg::texture_dimension::texture_2d;
		else hed.dim = qeg::texture_dimension::texture_3d;
		hed.size = uvec3(s.dimensions(0));
		hed.mip_count = s.levels();
		hed.array_count = 0;
		hed.format = pif;

		qeg::bo_file* texf = new qeg::bo_file(qeg::bo_file::file_type::texture);
		qeg::bo_file::chunk hedc(0,
			new qeg::datablob<glm::byte>((glm::byte*)&hed, sizeof(qeg::detail::texture_header)));
		qeg::bo_file::chunk dc(1,
			new qeg::datablob<glm::byte>((glm::byte*)s.data(), s.size()));// *qeg::bytes_per_pixel(qeg::detail::convert(pif))));
		texf->chunks().push_back(hedc);
		texf->chunks().push_back(dc);
		return texf;
	}
}

int main(int argc, char* argv[])
{

	vector<string> args;	
	for (int ij = 1; ij < argc; ++ij) args.push_back(argv[ij]);
	
	string in_file = args[0];
	string out_file = args[1];

	cout << "converting " << in_file << " to " << out_file << endl;
	
	try
	{
		auto i = gli::load_dds(in_file.c_str());

		auto bf = write_tex(i);
		auto d = bf->write();
		ofstream out(out_file, ios_base::binary);
		out.write((const char*)d.data, d.length);
		out.flush();
		out.close();
	}
	catch(const std::exception& e)
	{
		cout << "error@txcv::main => " << e.what() << endl;
	}
	return 0;
}