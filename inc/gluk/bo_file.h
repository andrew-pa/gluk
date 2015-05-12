#pragma once
#include "cmmn.h"

namespace gluk
{
	typedef unsigned char byte;
	class bo_file
	{
	public:
		enum class file_type : uint
		{
			generic = 0,
			texture = 8,
			model = 9,
		};
		struct chunk
		{
			uint type;
			vector<byte> data;
			chunk(uint t, const vector<byte>& d)
				: type(t), data(d){}
		};
	protected:
		struct header
		{
			file_type type;
			uint number_of_chunks;
		};
		struct chunk_desc
		{
			uint type;
			uint offset;
			uint length;
		};
		file_type _htype;
		vector<chunk> _chunks;
	public:

		bo_file(file_type t)
			: _htype(t){}

		/*bo_file(const datablob<byte>& d)
		{
			byte* bd = d.data;
			header* h = (header*)bd;
			bd += 2 * sizeof(uint);
			chunk_desc* cds = (chunk_desc*)bd;
			bd += sizeof(chunk_desc)*h->number_of_chunks;
			_htype = h->type;
			for (uint i = 0; i < h->number_of_chunks; ++i)
			{
				_chunks.push_back(chunk(cds->type, new datablob<byte>(bd + cds->offset, cds->length)));
				cds++; //move to next chunk
			}
		}*/

		bo_file(const filedatasp d)
		{
			const byte* bd = d->data<byte>();
			header* h = (header*)bd;
			bd += 2 * sizeof(uint);
			chunk_desc* cds = (chunk_desc*)bd;
			bd += sizeof(chunk_desc)*h->number_of_chunks;
			_htype = h->type;
			for (uint i = 0; i < h->number_of_chunks; ++i)
			{
				_chunks.push_back(chunk(cds->type, make_data_vector(bd + cds->offset, cds->length)));
				cds++; //move to next chunk
			}
		}

		void write(ostream& os)
		{
			size_t total_length = sizeof(header)+sizeof(chunk_desc)*_chunks.size();
			for (const auto& c : _chunks)
				total_length += c.data.size();

			byte* bd = new byte[total_length];
			byte* fbd = bd;
			header* h = (header*)bd;
			bd += sizeof(header);
			h->type = _htype;
			h->number_of_chunks = _chunks.size();

			byte* data_bd = bd + sizeof(chunk_desc)*_chunks.size();
			size_t current_offset = 0;

			for (const auto& c : _chunks)
			{
				chunk_desc* cd = (chunk_desc*)bd;
				cd->type = c.type;
				cd->offset = current_offset;
				cd->length = c.data.size();

				memcpy(data_bd, c.data.data(), c.data.size());

				current_offset += c.data.size();
				data_bd += c.data.size();
				bd += sizeof(chunk_desc);
			}

			os.write((const char*)fbd, total_length);
			//return *(new datablob<byte>(fbd, total_length));
		}

		proprw(file_type, type, { return _htype; });
		proprw(vector<chunk>, chunks, { return _chunks; });
	};
}
