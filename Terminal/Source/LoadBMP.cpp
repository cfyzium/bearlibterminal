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

#include "Bitmap.hpp"
#include <istream>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <cmath>

namespace BearLibTerminal
{
	Bitmap LoadBMP(std::istream& stream)
	{
		struct BitmapFileHeader
		{
			uint16_t header;					// 0x42 0x4D in hexadecimal, same as BM in ASCII
			uint16_t size_low, size_high;		// The size of the BMP file in bytes
			uint16_t reserved1, reserved2;		// Reserved
			uint16_t offset_low, offset_high;	// The offset of the byte where the bitmap image data can be found.
		};

		struct BitmapInfoHeader
		{
			uint32_t header_size;				// The size of this header (40 bytes)
			int32_t width;						// The bitmap width (signed integer).
			int32_t height;						// The bitmap height (signed integer).
			uint16_t planes;					// The number of color planes. Must be set to 1.
			uint16_t bpp;						// The number of bits per pixel. Typical values are 1, 4, 8, 16, 24 and 32.
			uint32_t compression;				// The compression method.
			uint32_t image_size;				// The size of the raw bitmap data.
			int32_t xppm;						// The horizontal resolution of the image (pixel per meter, signed integer).
			int32_t yppm;						// The vertical resolution of the image.
			uint32_t colors_used;				// The number of colors in the color palette, or 0 to default to 2n.
			uint32_t colors_important;			// The number of important colors used; generally ignored.
		};

		BitmapFileHeader file_header;
		BitmapInfoHeader info_header;
		std::vector<Color> palette;

		stream.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
		stream.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

		if ( !stream.good() )
		{
			throw std::runtime_error("invalid bitmap image");
		}

		if ( info_header.header_size != 40 )
		{
			throw std::runtime_error("unsupported DIB image format");
		}

		if ( info_header.bpp != 8 && info_header.bpp != 24 && info_header.bpp != 32 )
		{
			throw std::runtime_error("unsupported color depth");
		}

		if ( info_header.compression != 0 )
		{
			throw std::runtime_error("unsupported compression level");
		}

		if ( info_header.bpp < 24 )
		{
			size_t palette_entries = info_header.colors_used;
			if ( palette_entries == 0 ) palette_entries = (1 << info_header.bpp);
			palette.reserve(palette_entries);

			for ( size_t i = 0; i < palette_entries; i++ )
			{
				Color entry;
				stream.read(reinterpret_cast<char*>(&entry), sizeof(entry));
				entry.a = 0xFF;
				palette.push_back(entry);
			}
		}
		
		size_t offset = (file_header.offset_high << 16) + file_header.offset_low;
		std::streamoff position = stream.tellg();
		stream.ignore(offset-position);

		if ( !stream.good() )
		{
			throw std::runtime_error("invalid bitmap image");
		}

		Bitmap result(Size(info_header.width, (int)std::abs(info_header.height)), Color());

		std::function<void(Point)> pixel_readers[] =
		{
			[&](Point pt) -> void
			{
				// 32-bit BGRA, full Color struct
				Color value;
				stream.read(reinterpret_cast<char*>(&value), 4);
				result(pt) = value;
			},
			[&](Point pt) -> void
			{
				// 24-bit BGR, 3/4 Color struct
				Color value;
				stream.read(reinterpret_cast<char*>(&value), 3);
				value.a = 0xFF;
				result(pt) = value;
			},
			[&](Point pt) -> void
			{
				// Palletized I, lookup
				uint8_t index;
				stream.read(reinterpret_cast<char*>(&index), 1);
				result(pt) = palette[index];
			}
		};

		std::function<void(Point)> pixel_reader;
		switch ( info_header.bpp )
		{
		case 32:
			pixel_reader = pixel_readers[0];
			break;

		case 24:
			pixel_reader = pixel_readers[1];
			break;

		case 8:
			pixel_reader = pixel_readers[2];
			break;
		}

		size_t line_size = (info_header.bpp/8) * info_header.width;
		int padding = 4;
		int modulo = line_size % padding;
		size_t line_stride = line_size + (modulo > 0? padding-modulo: 0);
		size_t line_padding = line_stride - line_size;

		auto line_reader = [&](int y) -> void
		{
			for ( int x = 0; x < (int)info_header.width; x++ )
			{
				pixel_reader(Point(x, y));
			}
			stream.ignore(line_padding);
		};

		if ( info_header.height > 0 )
		{
			// For positive height values rows are stored from bottom to top
			for ( int y = info_header.height-1; y >= 0; y-- ) line_reader(y);
		}
		else 
		{
			// For negative height rows are top to bottom
			for ( int y = 0; y < -info_header.height; y++ ) line_reader(y);
		}

		return result;
	}
}
