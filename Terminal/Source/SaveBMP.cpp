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
#include <ostream>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <cmath>

namespace BearLibTerminal
{
	void SaveBMP(const Bitmap& bitmap, std::ostream& stream)
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

		Size bitmap_size = bitmap.GetSize();
		int line_size = bitmap_size.width * 3;
		int padding = 4;
		int modulo = line_size % padding;
		int line_stride = line_size + (modulo > 0? padding-modulo: 0);
		padding = line_stride - line_size;

		BitmapFileHeader file;
		file.header = 0x4D42;

		BitmapInfoHeader header;
		header.header_size = sizeof(header);
		header.width = bitmap_size.width;
		header.height = bitmap_size.height;
		header.planes = 1;
		header.bpp = 24;
		header.compression = 0; // None
		header.image_size = bitmap_size.height * line_stride;
		header.xppm = 2835;
		header.yppm = 2835;
		header.colors_used = 0;
		header.colors_important = 0;

		file.offset_low = sizeof(file)+sizeof(header);
		file.offset_high = 0;

		uint32_t file_size = file.offset_low + header.image_size;
		file.size_low = file_size & 0x0000FFFF;
		file.size_high = (file_size & 0xFFFF0000) >> 16;

		stream.write((const char*)&file, sizeof(file));
		stream.write((const char*)&header, sizeof(header));

		for (int y=bitmap_size.height-1; y>=0; y--)
		{
			for (int x=0; x<bitmap_size.width; x++)
			{
				Color c = bitmap(x, y);
				if (c.a < 255)
				{
					float factor = c.a/255.0f;
					c = Color(c.r*factor, c.g*factor, c.b*factor);
				}
				uint8_t pixel[] = {c.b, c.g, c.r};
				stream.write((const char*)pixel, 3);
			}
			stream.write("....", padding);
		}
	}
}
