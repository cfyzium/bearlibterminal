/*
* BearLibTerminal
* Copyright (C) 2015 Cfyz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "BOM.hpp"
#include <stdint.h>
#include <string.h>

namespace BearLibTerminal
{
	static uint8_t BOM_Bytes[][4] =
	{
		{0xEF, 0xBB, 0xBF},       // UTF-8
		{0xFF, 0xFE},             // UTF-16 LE
		{0xFE, 0xFF},             // UTF-16 BE
		{0xFF, 0xFE, 0x00, 0x00}, // UTF-32 LE
		{0x00, 0x00, 0xFE, 0xFF}  // UTF-32 BE
	};

	size_t GetBOMSize(BOM bom)
	{
		switch (bom)
		{
		case BOM::UTF8:
			return 3;
		case BOM::UTF16LE:
		case  BOM::UTF16BE:
			return 2;
		case BOM::UTF32LE:
		case BOM::UTF32BE:
			return 4;
		default:
			return 0;
		}
	}

	BOM DetectBOM(std::istream& stream)
	{
		uint8_t mark[4] = {0};
		size_t mark_size = 0;

		// Try to read some bytes and rewind.
		stream.read((char*)mark, 4);
		mark_size = stream.gcount();
		stream.seekg(0); // Assumes the stream was at its beginning.
		stream.clear(); // Because EOF is possible.

		if (mark_size == 4) // May be UTF-32 (four byte BOM)
		{
			if (::memcmp(mark, BOM_Bytes[BOM::UTF32LE], 4) == 0)
				return BOM::UTF32LE;
			else if (::memcmp(mark, BOM_Bytes[BOM::UTF32BE], 4) == 0)
				return BOM::UTF32BE;
		}
		else if (mark_size >= 3) // May be UTF-8 (three byte BOM)
		{
			if (::memcmp(mark, BOM_Bytes[BOM::UTF8], 3) == 0)
				return BOM::UTF8;
		}
		else if (mark_size >= 2) // May be UTF-16 (two byte BOM)
		{
			if (::memcmp(mark, BOM_Bytes[BOM::UTF16LE], 2) == 0)
				return BOM::UTF16LE;
			else if (::memcmp(mark, BOM_Bytes[BOM::UTF16BE], 2) == 0)
				return BOM::UTF16BE;
		}

		// Failed to find an exact BOM marker. Try to guess file encoding
		// based on values of first bytes, assuming the first one or two
		// characters will be ASCII.
		if (mark_size >= 4)
		{
			if (!mark[0] && !mark[1] && !mark[2] && mark[3] <= 0x7f)     // 0 0 0 ?
				return BOM::ASCII_UTF32BE;
			else if(!mark[0] && mark[1] && !mark[2] && mark[3])          // 0 ? 0 ?
				return BOM::ASCII_UTF16BE;
			else if(mark[0] <= 0x7f && !mark[1] && !mark[2] && !mark[3]) // ? 0 0 0
				return BOM::ASCII_UTF32LE;
			else if(mark[0] && !mark[1] && mark[2] && !mark[3])          // ? 0 ? 0
				return BOM::ASCII_UTF16LE;
		}

		// If both are ASCII, assume compatible UTF-8.
		if (mark_size >= 2)
		{
			if (mark[0] <= 0x7f && mark[1] <= 0x7f)
				return BOM::ASCII_UTF8;
		}

		return BOM::None;
	}
}
