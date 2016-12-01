/*
* BearLibTerminal
* Copyright (C) 2013-2016 Cfyz
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

#include "BitmapTileset.hpp"
#include "LoadBitmap.hpp"
#include "Geometry.hpp"
#include "Resource.hpp"
#include "Utility.hpp"
#include "Palette.hpp"
#include "Encoding.hpp"
#include "Log.hpp"
#include <stdexcept>
#include <fstream>
#include <cmath>

#include <sys/time.h>

namespace BearLibTerminal
{
	BitmapTileset::BitmapTileset(char32_t offset, std::vector<uint8_t> data, OptionGroup& options):
		Tileset(offset)
	{
		std::unique_ptr<Encoding8> codepage;

		// Try to guess tile size and tileset codepage, it will be rewritten if supplied.
		{
			// Try to guess from the filename.
			std::wstring name = options.attributes[L"name"];
			std::wstring l1, l2;

			// Cut off extension.
			size_t n = name.find_last_of(L'.');
			if (n != std::wstring::npos)
				name = name.substr(0, n);

			// Last part -> codepage.
			n = name.find_last_of(L'_');
			if (n != std::wstring::npos)
			{
				if (n < name.length()-1)
					l1 = name.substr(n+1);
				name = name.substr(0, n);
			}

			// Now, last part -> size.
			n = name.find_last_of(L'_');
			if (n != std::wstring::npos)
			{
				if (n < name.length()-1)
					l2 = name.substr(n+1);
			}

			if (!l1.empty() || !l2.empty())
				LOG(Debug, "Bitmap tileset '" << options.attributes[L"name"] << "': guessing tile size (from '" << l2 << "') and codepage (from '" << l1 << "')");

			Size temp_size;
			if (try_parse(l2, temp_size))
			{
				m_bounding_box_size = temp_size;
				codepage = GetUnibyteEncoding(l1);
			}
			else
			{
				LOG(Debug, "Bitmap tileset: failed to parse guessed tile size, not an error");
			}
		}

		if (options.attributes.count(L"size") && !try_parse(options.attributes[L"size"], m_bounding_box_size))
			throw std::runtime_error("BitmapTileset: failed to parse 'size' attribute");

		Size resize_to;
		if (options.attributes.count(L"resize") && !try_parse(options.attributes[L"resize"], resize_to))
			throw std::runtime_error("BitmapTileset: failed to parse 'resize' attribute");

		ResizeFilter resize_filter = ResizeFilter::Bilinear;
		if (options.attributes.count(L"resize-filter") && !try_parse(options.attributes[L"resize-filter"], resize_filter))
			throw std::runtime_error("BitmapTileset: failed to parse 'resize-filter' attribute");

		ResizeMode resize_mode = ResizeMode::Stretch;
		if (options.attributes.count(L"resize-mode") && !try_parse(options.attributes[L"resize-mode"], resize_mode))
			throw std::runtime_error("BitmapTileset: failed to parse 'resize-mode' attribute");

		if (options.attributes.count(L"codepage"))
			codepage = GetUnibyteEncoding(options.attributes[L"codepage"]); // Should either return an encoding or throw

		if (!codepage)
			codepage = GetUnibyteEncoding(L"utf8");

		Size spacing{1, 1};
		if (options.attributes.count(L"spacing") && !try_parse(options.attributes[L"spacing"], spacing))
			throw std::runtime_error("BitmapTileset: failed to parse 'spacing' attribute");

		TileAlignment alignment = TileAlignment::Unknown;
		if (options.attributes.count(L"align") && !try_parse(options.attributes[L"align"], alignment))
			throw std::runtime_error("BitmapTileset: failed to parse 'alignment' attribute");

		Size raw_size;
		if (options.attributes.count(L"raw-size") && !try_parse(options.attributes[L"raw-size"], raw_size))
			throw std::runtime_error("BitmapTileset: failed to parse 'raw-size' attribute");

		Bitmap image = raw_size.Area()? Bitmap(raw_size, (const Color*)&data[0]): LoadBitmap(data);
		if (!image.GetSize().Area())
			throw std::runtime_error("BitmapTileset: loaded image is empty");

		if (options.attributes.count(L"transparent"))
		{
			std::wstring name = options.attributes[L"transparent"];
			if (name == L"auto")
			{
				if (!image.HasAlpha())
				{
					image.MakeTransparent(image(0, 0));
				}
			}
			else if (name != L"false")
			{
				image.MakeTransparent(Palette::Instance.Get(name));
			}
		}

		if (!m_bounding_box_size.Area())
			m_bounding_box_size = image.GetSize();
		else if (!Rectangle{image.GetSize()}.Contains(Rectangle{m_bounding_box_size}))
			throw std::runtime_error("Bitmap tileset: bitmap is smaller than tile size");

		if (m_bounding_box_size.width < 1 || m_bounding_box_size.height < 1)
			m_bounding_box_size = Size{1, 1};

		Size source_tile_size = m_bounding_box_size;
		if (resize_to.Area())
		{
			LOG(Debug, "BitmapTileset: changing tile size " << m_bounding_box_size << " -> " << resize_to);
			m_bounding_box_size = resize_to;
		}

		Size image_size = image.GetSize();
		int columns = image_size.width / source_tile_size.width;
		int rows = image_size.height / source_tile_size.height;
		Size grid_size = Size{columns, rows};
		LOG(Debug, "Tileset has " << columns << "x" << rows << " tiles");

		if (alignment == TileAlignment::Unknown)
		{
			if (grid_size.Area() > 1)
				alignment = (grid_size.Area() > 1? TileAlignment::Center: TileAlignment::TopLeft);
		}

		auto keep_tile = [&](int x, int y, char32_t code)
		{
			Point offset;
			if (alignment == TileAlignment::Center)
			{
				// TODO: round in a way to compensate state.half_cellsize rounding error
				offset = Point(-m_bounding_box_size.width/2, -m_bounding_box_size.height/2);
			}

			auto tile = std::make_shared<TileInfo>();
			tile->tileset = this;//shared_from_this();
			tile->bitmap = image.Extract(Rectangle{Point{x * source_tile_size.width, y * source_tile_size.height}, source_tile_size});
			if (resize_to.Area())
				tile->bitmap = tile->bitmap.Resize(resize_to, resize_filter, resize_mode);
			tile->offset = offset;
			tile->spacing = spacing;
			tile->alignment = alignment;
			m_cache[code] = tile;
		};

		if (Tileset::IsFontOffset(offset))
		{
			// Font.
			for (int y = 0; y < rows; y++)
			{
				for (int x = 0; x < columns; x++)
				{
					char32_t code = offset + codepage->Convert(y * columns + x);
					keep_tile(x, y, code);
				}
			}
		}
		else
		{
			// Tileset.
			for (int i = 0; m_cache.size() < grid_size.Area(); i++)
			{
				int index = codepage->Convert((wchar_t)i);

				if (index == -1)
					break;
				else if (index < 0 || index >= grid_size.Area())
					continue;

				int x = index % columns;
				int y = (index - x) / columns;
				keep_tile(x, y, offset + i);
			}
		}
	}

	Size BitmapTileset::GetBoundingBoxSize()
	{
		return m_bounding_box_size;
	}
}
