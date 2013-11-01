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

#include <functional>
#include <algorithm>
#include <stdexcept>
#include <istream>
#include <vector>
#include <memory>
#include "NanoJPEG.hpp"
#include "Geometry.hpp"
#include "Utility.hpp"
#include "Bitmap.hpp"
#include "Log.hpp"

namespace BearLibTerminal
{
	std::string to_string(const Jpeg::Decoder::DecodeResult& value)
	{
		switch (value)
		{
		case Jpeg::Decoder::OK: return "OK";
		case Jpeg::Decoder::NotAJpeg: return "not a JPEG";
		case Jpeg::Decoder::Unsupported: return "unsupported format";
		case Jpeg::Decoder::OutOfMemory: return "out of memory";
		case Jpeg::Decoder::InternalError: return "internal error";
		case Jpeg::Decoder::SyntaxError: return "syntax error";
		default: return "unknown error";
		}
	}

	Bitmap LoadJPEG(std::istream& stream)
	{
		std::istreambuf_iterator<char> eos;
		std::string contents(std::istreambuf_iterator<char>(stream), eos);
		std::unique_ptr<Jpeg::Decoder> decoder(new Jpeg::Decoder((const unsigned char*)contents.data(), contents.size()));

		if (decoder->GetResult() != Jpeg::Decoder::OK)
		{
			throw std::runtime_error(std::string("Failed to load JPEG resource: ") + to_string(decoder->GetResult()));
		}

		Size size(decoder->GetWidth(), decoder->GetHeight());

		if (size.width <= 0 || size.height <= 0)
		{
			throw std::runtime_error(std::string("Failed to load JPEG resource: internal loader error"));
		}

		Bitmap result(size, Color());
		uint8_t* data = decoder->GetImage();
		bool is_color = decoder->IsColor();

		for (int y = 0; y < size.height; y++)
		{
			for (int x = 0; x < size.width; x++)
			{
				if (is_color)
				{
					result(x, y) = Color(0xFF, data[2], data[1], data[0]);
					data += 3;
				}
				else
				{
					result(x, y) = Color(0xFF, data[0], data[0], data[0]);
					data += 1;
				}
			}
		}

		return result;
	}
}
