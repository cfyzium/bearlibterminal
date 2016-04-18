/*
 * BitmapTileset.cpp
 *
 *  Created on: Oct 17, 2013
 *      Author: Cfyz
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
	BitmapTileset::BitmapTileset(char32_t offset, OptionGroup& options):
		Tileset(offset)
	{
		std::wstring name = options.attributes[L"name"]; // FIXME: come up with better name for the default/main attribute of a group
		if (name.empty())
			throw std::runtime_error("BitmapTileset: missing or empty main value attribute");

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

		/*
		uint64_t address = 0;
		if (name.find(L".") == std::wstring::npos && try_parse(name, address))
		{
			LOG(Debug, "Bitmap tileset name \"" << name << "\" is a memory address");

			Size raw_size;
			if (group.attributes.count(L"raw-size") && !try_parse(group.attributes[L"raw-size"], raw_size))
			{
				throw std::runtime_error("BitmapTileset: failed to parse 'raw-size' attribute");
			}

			if (!raw_size.Area() && m_tile_size.Area())
			{
				raw_size = m_tile_size;
			}

			if (!raw_size.Area())
			{
				throw std::runtime_error("BitmapTileset: cannot guess bitmap dimensions for raw bitmap resource");
			}

			const Color* pixels = (const Color*)address;
			m_cache = Bitmap(raw_size, pixels);
		}
		else
		{
			m_cache = LoadBitmap(*Resource::Open(name, L"tileset-"));
		}
		//*/

		Bitmap image = LoadBitmap(*Resource::Open(name, L"tileset-"));

		if (!image.GetSize().Area())
			throw std::runtime_error("BitmapTileset: loaded image is empty");

		if (options.attributes.count(L"transparent"))
		{
			std::wstring& name = options.attributes[L"transparent"];
			Color mask = (name == L"auto"? image(0, 0): Palette::Instance[name]);
			image.MakeTransparent(mask);
		}

		if (resize_to.Area())
		{
			LOG(Trace, "BitmapTileset: resizing tileset image to " << resize_to << " with " << resize_filter << " filter, " << resize_mode << " mode");
			Size prev = image.GetSize();
			image = image.Resize(resize_to, resize_filter, resize_mode);
		}

		if (!m_bounding_box_size.Area())
			m_bounding_box_size = image.GetSize();
		else if (!Rectangle{image.GetSize()}.Contains(Rectangle{m_bounding_box_size}))
			throw std::runtime_error("Bitmap tileset: bitmap is smaller than tile size");

		if (m_bounding_box_size.width < 1 || m_bounding_box_size.height < 1)
			m_bounding_box_size = Size{1, 1};

		// Adjust tile size accordingly
		if (resize_to.Area())
		{
			float hf = resize_to.width/(float)m_bounding_box_size.width;
			float vf = resize_to.height/(float)m_bounding_box_size.height;
			m_bounding_box_size.width *= hf;
			m_bounding_box_size.height *= vf;
		}

		Size image_size = image.GetSize();
		int columns = image_size.width / m_bounding_box_size.width;
		int rows = image_size.height / m_bounding_box_size.height;
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
			tile->bitmap = image.Extract(Rectangle{Point{x * m_bounding_box_size.width, y * m_bounding_box_size.height}, m_bounding_box_size});
			tile->offset = offset;
			tile->spacing = spacing;
			tile->alignment = alignment;
			m_cache[code] = tile;
		};

		if ((offset & 0x00FFFFFF) == 0)
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
