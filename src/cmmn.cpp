#include "cmmn.h"

namespace gluk
{
#ifdef WIN32
	//Read in the data contained in filename, put it in to a datablob
	const datablob<byte>& read_data(
		const wstring& filename	//_In_ const wchar_t* filename
		)
	{
		CREATEFILE2_EXTENDED_PARAMETERS extendedParams = { 0 };
		extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
		extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
		extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
		extendedParams.lpSecurityAttributes = nullptr;
		extendedParams.hTemplateFile = nullptr;

		HANDLE file = CreateFile2(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams);

		if (file == INVALID_HANDLE_VALUE)
		{
			throw exception("DX::ReadData > Invalid File");
		}

		FILE_STANDARD_INFO fileInfo = { 0 };
		if (!GetFileInformationByHandleEx(
			file,
			FileStandardInfo,
			&fileInfo,
			sizeof(fileInfo)
			))
		{
			throw exception("DX::ReadData > GetFileInfo failed");
		}

		if (fileInfo.EndOfFile.HighPart != 0)
		{
			throw exception();
		}

		//datablob<byte> fileData = ref new Platform::Array<byte>(fileInfo.EndOfFile.LowPart);

		datablob<byte>* fileData = new datablob<byte>(fileInfo.EndOfFile.LowPart);

		DWORD al = 0;
		if (!ReadFile(
			file,
			fileData->data,
			fileData->length,
			&al,
			nullptr
			))
		{
			throw exception("DX::ReadData > Couldn't read file");
		}

		CloseHandle(file);
		return *fileData;
	}

	//Wrapper for read_data, but adds the executable path on to the file name
	const datablob<byte>& read_data_from_package(_In_ const wstring& filename)
	{
		static std::wstring fpath = L"";
		if (fpath.length() == 0)
		{
			wchar_t* modfn = new wchar_t[MAX_PATH];
			GetModuleFileName(NULL, modfn, MAX_PATH);
			fpath = std::wstring(modfn);
			int pl = -1;
			for (int i = fpath.length(); i > 0; i--)
			{
				if (fpath[i] == '\\')
				{
					pl = i + 1;
					break;
				}
			}
			fpath = fpath.substr(0, pl);
		}
		std::wstring fpn = fpath;
		fpn += filename;
		return read_data(fpn.data());
	}
#endif

	void __check_gl()
	{
		GLenum e;
		if ((e = glGetError()) != GL_NO_ERROR)
		{
			cerr << "GL error: " << e << endl;
			throw error_code_exception(e, "GL error");
		}
	}

#define GL_PX_FMT(C, T, D) GL_##C##D##T
#define ct(X) if(type == pixel_type::##X )
	GLenum pixel_format::get_gl_format_internal() const
	{
		if(depth == 8)
		{
			switch (comp)
			{
			case pixel_components::r:
				ct(unorm)    return GL_PX_FMT(R, , 8);
				ct(snorm)    return GL_PX_FMT(R, _SNORM, 8);
				ct(floatp)   throw;//return GL_PX_FMT(R, F, 8);
				ct(integer)  return GL_PX_FMT(R, I, 8);
				ct(uinteger) return GL_PX_FMT(R, UI, 8);
				break;
			case pixel_components::rg:
				ct(unorm)    return GL_PX_FMT(RG, , 8);
				ct(snorm)    return GL_PX_FMT(RG, _SNORM, 8);
				ct(floatp)   throw;//return GL_PX_FMT(R, F, 8);
				ct(integer)  return GL_PX_FMT(RG, I, 8);
				ct(uinteger) return GL_PX_FMT(RG, UI, 8);
				break;
			case pixel_components::rgb:
				ct(unorm)    return GL_PX_FMT(RGB, , 8);
				ct(snorm)    return GL_PX_FMT(RGB, _SNORM, 8);
				ct(floatp)   throw;// return GL_PX_FMT(R, F, 8);
				ct(integer)  return GL_PX_FMT(RGB, I, 8);
				ct(uinteger) return GL_PX_FMT(RGB, UI, 8);
				break;
			case pixel_components::rgba:
				ct(unorm)    return GL_PX_FMT(RGBA, , 8);
				ct(snorm)    return GL_PX_FMT(RGBA, _SNORM, 8);
				ct(floatp)   throw;// return GL_PX_FMT(RGBA, F, 8);
				ct(integer)  return GL_PX_FMT(RGBA, I, 8);
				ct(uinteger) return GL_PX_FMT(RGBA, UI, 8);
				break;
			}
		}
		else if(depth == 16)
		{
			switch (comp)
			{
			case pixel_components::r:
				ct(unorm)    return GL_PX_FMT(R, , 16);
				ct(snorm)    return GL_PX_FMT(R, _SNORM, 16);
				ct(floatp)   return GL_PX_FMT(R, F, 16);
				ct(integer)  return GL_PX_FMT(R, I, 16);
				ct(uinteger) return GL_PX_FMT(R, UI, 16);
				break;
			case pixel_components::rg:
				ct(unorm)    return GL_PX_FMT(RG, , 16);
				ct(snorm)    return GL_PX_FMT(RG, _SNORM, 16);
				ct(floatp)   return GL_PX_FMT(R, F, 16);
				ct(integer)  return GL_PX_FMT(RG, I, 16);
				ct(uinteger) return GL_PX_FMT(RG, UI, 16);
				break;
			case pixel_components::rgb:
				ct(unorm)    return GL_PX_FMT(RGB, , 16);
				ct(snorm)    return GL_PX_FMT(RGB, _SNORM, 16);
				ct(floatp)   return GL_PX_FMT(R, F, 16);
				ct(integer)  return GL_PX_FMT(RGB, I, 16);
				ct(uinteger) return GL_PX_FMT(RGB, UI, 16);
				break;
			case pixel_components::rgba:
				ct(unorm)    return GL_PX_FMT(RGBA, , 16);
				ct(snorm)    return GL_PX_FMT(RGBA, _SNORM, 16);
				ct(floatp)   return GL_PX_FMT(RGBA, F, 16);
				ct(integer)  return GL_PX_FMT(RGBA, I, 16);
				ct(uinteger) return GL_PX_FMT(RGBA, UI, 16);
				break;
			case pixel_components::depth:
				if (type != pixel_type::unorm) throw;
				return GL_DEPTH_COMPONENT16;
			}
		}
		else if(depth == 32)
		{
			switch (comp)
			{
			case pixel_components::r:
				ct(unorm)    throw;// return GL_PX_FMT(R, , 32);
				ct(snorm)    throw;// return GL_PX_FMT(R, _SNORM, 32);
				ct(floatp)   return GL_PX_FMT(R, F, 32);
				ct(integer)  return GL_PX_FMT(R, I, 32);
				ct(uinteger) return GL_PX_FMT(R, UI, 32);
				break;
			case pixel_components::rg:
				ct(unorm)    throw;// return GL_PX_FMT(RG, , 32);
				ct(snorm)    throw;// return GL_PX_FMT(RG, _SNORM, 32);
				ct(floatp)   return GL_PX_FMT(R, F, 32);
				ct(integer)  return GL_PX_FMT(RG, I, 32);
				ct(uinteger) return GL_PX_FMT(RG, UI, 32);
				break;
			case pixel_components::rgb:
				ct(unorm)    throw;// return GL_PX_FMT(RGB, , 32);
				ct(snorm)    throw;// return GL_PX_FMT(RGB, _SNORM, 32);
				ct(floatp)   return GL_PX_FMT(R, F, 32);
				ct(integer)  return GL_PX_FMT(RGB, I, 32);
				ct(uinteger) return GL_PX_FMT(RGB, UI, 32);
				break;
			case pixel_components::rgba:
				ct(unorm)    throw;// return GL_PX_FMT(RGBA, , 32);
				ct(snorm)    throw;// return GL_PX_FMT(RGBA, _SNORM, 32);
				ct(floatp)   return GL_PX_FMT(RGBA, F, 32);
				ct(integer)  return GL_PX_FMT(RGBA, I, 32);
				ct(uinteger) return GL_PX_FMT(RGBA, UI, 32);
				break;
			case pixel_components::depth:
				if (type == pixel_type::unorm) return GL_DEPTH_COMPONENT32;
				else if (type == pixel_type::floatp) return GL_DEPTH_COMPONENT32F;
				else throw;
				break;
			case pixel_components::depth_stencil:
				if (type == pixel_type::floatp)
					return GL_DEPTH32F_STENCIL8;
				else throw;
				break;
			}
		}
		else if(depth == 24)
		{
			if (comp == pixel_components::depth)
			{
				if(type == pixel_type::unorm)
					return GL_DEPTH_COMPONENT24;
				else throw;
			}
			else if(comp == pixel_components::depth_stencil)
			{
				if (type == pixel_type::unorm)
					return GL_DEPTH24_STENCIL8;
				else throw;
			}
			else throw;
		}
		else throw;
	}
	GLenum pixel_format::get_gl_type() const
	{
		if(type == pixel_type::floatp)
		{
			if (comp == pixel_components::depth_stencil)
				return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			if (depth == 16) return GL_HALF_FLOAT;
			else return GL_FLOAT;
		}
		else if(type == pixel_type::snorm)
		{
			if (depth == 16) return GL_SHORT;
			else if (depth == 8) return GL_BYTE;
			else return GL_INT;
		}
		else if (type == pixel_type::unorm)
		{
			if (depth == 24 && comp == pixel_components::depth_stencil)
				return GL_UNSIGNED_INT_24_8;
			if (depth == 16) return GL_UNSIGNED_SHORT;
			else if (depth == 8) return GL_UNSIGNED_BYTE;
			else return GL_UNSIGNED_INT;
		}
		else if(type == pixel_type::integer)
		{
			if (depth == 16) return GL_SHORT;
			else return GL_INT;
		}
		else if(type == pixel_type::uinteger)
		{
			if (depth == 16) return GL_UNSIGNED_SHORT;
			else return GL_UNSIGNED_INT;
		}
		return GL_INVALID_ENUM;
	}
	GLenum pixel_format::get_gl_format() const
	{
		if(type == pixel_type::integer || type == pixel_type::uinteger)
		{
			switch (comp)
			{
			case gluk::pixel_components::r:
				return GL_RED_INTEGER;
			case gluk::pixel_components::rg:
				return GL_RG_INTEGER;
			case gluk::pixel_components::rgb:
				return GL_RGB_INTEGER;
			case gluk::pixel_components::rgba:
				return GL_RGBA_INTEGER;
			}
		}
		else
		{
			switch (comp)
			{
			case gluk::pixel_components::r:
				return GL_RED;
			case gluk::pixel_components::rg:
				return GL_RG;
			case gluk::pixel_components::rgb:
				return GL_RGB;
			case gluk::pixel_components::rgba:
				return GL_RGBA;
			case pixel_components::depth:
				return GL_DEPTH_COMPONENT;
			case pixel_components::depth_stencil:
				return GL_DEPTH_STENCIL;
			}
		}
	}
};
