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

#ifdef USE_LIBPNG
#include <png.h>
#else
#include "PicoPNG.hpp"
#endif

#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>
//#include "LoadPNG.hpp"
#include "Bitmap.hpp"
#include "Utility.hpp"
#include "Log.hpp"

#include <sys/time.h>

namespace BearLibTerminal
{
#ifdef USE_LIBPNG
	static void LoadPNG_custom_error(png_structp png_ptr, png_const_charp s)
	{
		throw std::runtime_error(_FMT("[LoadPNG] libpng error: " << s));
	}

	static void LoadPNG_custom_read(png_structp pngPtr, png_bytep data, png_size_t length)
	{
		png_voidp a = png_get_io_ptr(pngPtr);
		((std::istream*)a)->read((char*)data, length);
	}

	static bool LoadPNG_validate(std::istream& stream)
	{
		char header[8];
		stream.read(header, 8);
		return !png_sig_cmp((const unsigned char*)header, 0, 8);
	}

	Bitmap LoadPNG(std::istream& stream)
	{
		if ( !LoadPNG_validate(stream) )
		{
			throw std::runtime_error("[LoadPNG] stream is not PNG");
		}

		png_infop info_ptr = nullptr;
		std::unique_ptr<png_struct, std::function<void(png_struct*)>> png_ptr
		(
			png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, LoadPNG_custom_error, LoadPNG_custom_error),
			[&](png_struct* p) -> void { png_destroy_read_struct(&p, &info_ptr, (png_infopp)nullptr); }
		);

		// Initialize stuff
		if ( !png_ptr || (info_ptr = png_create_info_struct(png_ptr.get())) == nullptr )
		{
			throw std::runtime_error("[LoadPNG] Can't initialize libpng");
		}

		png_set_read_fn(png_ptr.get(), (png_voidp)&stream, LoadPNG_custom_read);
		png_set_sig_bytes(png_ptr.get(), 8); // Were already consumed through validation

		png_read_info(png_ptr.get(), info_ptr);
		size_t width = png_get_image_width(png_ptr.get(), info_ptr);
		size_t height = png_get_image_height(png_ptr.get(), info_ptr);
		uint8_t color_type = png_get_color_type(png_ptr.get(), info_ptr);
		uint8_t bit_depth = png_get_bit_depth(png_ptr.get(), info_ptr);

		// Convert different packed types to RGB[A]
		if ( color_type == PNG_COLOR_TYPE_PALETTE )
		{
			png_set_palette_to_rgb(png_ptr.get());
		}
		else if ( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		{
			png_set_gray_to_rgb(png_ptr.get());
		}

		// Ascertain color depth is 8 bit per channel
		if ( bit_depth == 16 )
		{
			png_set_strip_16(png_ptr.get());
		}

		// Reverse RGB --> BGR
		png_set_bgr(png_ptr.get());

		// If alpha channel is not present, add it
		if ( color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_GRAY_ALPHA )
		{
			png_set_add_alpha(png_ptr.get(), 0xFF, PNG_FILLER_AFTER);
		}

		png_read_update_info(png_ptr.get(), info_ptr);

		std::vector<png_bytep> row_pointers(height);
		size_t line_stride = png_get_rowbytes(png_ptr.get(), info_ptr);
		std::vector<uint8_t> buffer(height*line_stride);
		for ( size_t i=0; i < height; i++ ) row_pointers[i] = buffer.data() + line_stride*i;
		png_read_image(png_ptr.get(), row_pointers.data());

		return Bitmap(Size(width, height), (Color*)buffer.data());
	}
#else
	Bitmap LoadPNG(std::istream& stream)
	{
		std::string in_buffer((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		std::vector<unsigned char> out_buffer;
		unsigned long width, height;

		timeval t1, t2;
		gettimeofday(&t1, NULL);
		if ( decodePNG(out_buffer, width, height, (const unsigned char*)in_buffer.c_str(), in_buffer.size()) )
		{
			throw std::runtime_error("PNG decode failed");
		}
		gettimeofday(&t2, NULL);
		uint64_t delay = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);
		LOG(Trace, "Loading " << width << "x" << height << " image took " << delay << " us");

		LOG(Trace, L"Loaded PNG image, " << width << L"x" << height);

		return Bitmap(Size(width, height), (Color*)out_buffer.data());
	}
#endif
}
