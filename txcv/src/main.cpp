#define LINK
#include "cmmn.h"
#include "bo_file.h"
#include "resource.h"

#include "gli/gli.hpp"


//txcv:	 reads in textures (DDS) and converts them into .tex files
//usage txcv <in_file> <out_file>

//convert gli::format to pi_pixel_format
#undef RGB
#define RM pi_pixel_format::
#define pfb(bw, type) RM R##bw##_##type, \
	RM RG##bw##_##type, \
	RM RGB##bw##_##type, \
	RM RGBA##bw##_##type, \
	
qeg::detail::pi_pixel_format convert_pi(gli::format f)
{
	using qeg::detail::pi_pixel_format;
	using gli::format;
	pi_pixel_format conv_table[] = 
	{
		RM UNKNOWN,
		
		pfb(8, UNORM)
		pfb(16, UNORM)
		//pfb(32, UNORM)

		pfb(8, SNORM)
		pfb(16, SNORM)
		//pfb(32, SNORM)

		pfb(8, UINT)
		pfb(16, UINT)
		pfb(32, UINT)

		pfb(8, SINT)
		pfb(16, SINT)
		pfb(32, SINT)

		pfb(16, FLOAT)
		pfb(32, FLOAT)

		//depth formats
		//compressed formats
		//sRGB formats
	};
	return conv_table[(uint)f];
}

qeg::bo_file* write_tex(gli::storage& s)
{
	if(s.layers() > 1)
	{
		throw exception("texture arrays not yet supported");
	}
	if(s.faces() > 1)
	{
		auto pif = convert_pi(s.format());
		auto td = s.data();

		//cout << "[cubmap size=(" << s.dimensions(0).x << "x" << s.dimensions(0).y << ")]";

        qeg::detail::texture_header hed;
        hed.dim = qeg::texture_dimension::texture_cube;
        hed.size = uvec3(s.dimensions(0));
        hed.mip_count = s.levels();
        hed.array_count = 6;
        hed.format = pif;

        qeg::bo_file* texf = new qeg::bo_file(qeg::bo_file::file_type::texture);
        qeg::bo_file::chunk hedc(0,
            new qeg::datablob<glm::byte>((glm::byte*)&hed, sizeof(qeg::detail::texture_header)));
            
        for (int i = 0; i < 6; ++i)
        {
            glm::byte* dat = s.data() + i*s.faceSize(0, 0);

                
            qeg::bo_file::chunk dc(1,
                new qeg::datablob<glm::byte>(dat, s.faceSize(0, 0)));// *qeg::bytes_per_pixel(qeg::detail::convert(pif))));
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