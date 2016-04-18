/*
* BearLibTerminal
* Copyright (C) 2013 Cfyz
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

#include <stdexcept>
#include <cstring>
#include "LoadBitmap.hpp"
#include "Log.hpp"

// Forward declarations. See LoadXXX.cpp files for definitions.
namespace BearLibTerminal
{
	Bitmap LoadBMP(std::istream& stream);
	Bitmap LoadPNG(std::istream& stream);
	Bitmap LoadJPEG(std::istream& stream);
}

namespace BearLibTerminal
{
	Bitmap LoadBitmap(const std::vector<uint8_t>& data)
	{
		if (data.size() < 4)
			throw std::runtime_error("LoadBitmap: invalid data size");

		unsigned char magic_bytes[4] = {data[0], data[1], data[2], data[3]};

		// FIXME: rewrite bitmap loading routines. Maybe just use stb_image?
		std::istringstream stream{std::string((const char*)&data[0], data.size())};

		if (!strncmp((const char*)magic_bytes, "\x89PNG", 4))
		{
			// This must be PNG resource
			return LoadPNG(stream);
		}
		else if (!strncmp((const char*)magic_bytes, "BM", 2))
		{
			// This must be BMP DIB resource
			return LoadBMP(stream);
		}
		else if (!strncmp((const char*)magic_bytes, "\xFF\xD8", 2))
		{
			// Must be a JPEG rsource
			return LoadJPEG(stream);
		}
		else
		{
			throw std::runtime_error("unsupported image format");
		}
	}
}
