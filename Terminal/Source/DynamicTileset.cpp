/*
 * DynamicTileset.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: Cfyz
 */

#include "DynamicTileset.hpp"
#include "Log.hpp"
#include "Bitmap.hpp"
#include "Utility.hpp"
#include "Geometry.hpp"
#include "Encoding.hpp"
#include <cmath>

namespace BearLibTerminal
{
	DynamicTileset::DynamicTileset(TileContainer& container, OptionGroup& group):
		StronglyTypedReloadableTileset(container)
	{
		if (!group.attributes.count(L"size"))
		{
			throw std::runtime_error("DynamicTileset: 'size' attribute is missing");
		}

		if (!try_parse(group.attributes[L"size"], m_tile_size))
		{
			throw std::runtime_error("DynamicTileset: failed to parse 'size' attribute");
		}
	}

	void DynamicTileset::Remove() // TODO: move to base class?
	{
		for (auto i: m_tiles)
		{
			if (m_container.slots.count(i.first) && (void*)m_container.slots[i.first].get() == (void*)i.second.get())
			{
				m_container.slots.erase(i.first);
			}
		}
	}

	void DrawBoxLine(Bitmap& bitmap, int x1, int y1, int x2, int y2, int type, int width)
	{
		Color white{255, 255, 255, 255};

		if (x1 == x2)
		{
			// Vertical line
			for (int y=y1; y<=y2; y++)
			{
				if (type == 1)
				{
					// One thin line
					for (int x=x1; x<x1+width; x++) bitmap(x, y) = white;
				}
				else if (type == 2)
				{
					// One thick line
					for (int x=x1-width; x<x1+width*2; x++) bitmap(x, y) = white;
				}
				else if (type == 3)
				{
					// Two thin lines
					for (int x=x1-width; x<x1; x++) bitmap(x, y) = white;
					for (int x=x1+width; x<x1+width*2; x++) bitmap(x, y) = white;
				}
			}
		}
		else
		{
			// Horisontal line
			for (int x=x1; x<=x2; x++)
			{
				if (type == 1)
				{
					// One thin line
					for (int y=y1; y<y1+width; y++) bitmap(x, y) = white;
				}
				else if (type == 2)
				{
					// One thick line
					for (int y=y1-width; y<y1+width*2; y++) bitmap(x, y) = white;
				}
				else if (type == 3)
				{
					// Two thin lines
					for (int y=y1-width; y<y1; y++) bitmap(x, y) = white;
					for (int y=y1+width; y<y1+width*2; y++) bitmap(x, y) = white;
				}
			}
		}
	}

	Bitmap MakeBoxLines(Size size, int left, int top, int right, int bottom)
	{
		Bitmap result(size, Color(0, 0, 0, 0));

		int thickness = (int)std::floor(size.width/7.0f);
		if (thickness == 0) thickness = 1;

		thickness = 1;

		LOG(Trace, "thickness = " << thickness);

		int cx = (int)std::floor(size.width/2.0f-thickness/2.0f); // 8/2.0f = 4 (fifth pixel), 3/2.0 = 1[.5] (second pixel)
		int cy = (int)std::floor(size.height/2.0f-thickness/2.0f);
		int cl = cx - thickness;
		int ct = cy - thickness;
		int cw = thickness * 3;
		int ch = thickness * 3;
		int cr = cl + cw;
		int cb = ct + ch;

		auto put_square = [&](int dx, int dy)
		{
			int left = cx + dx*thickness;
			int top = cy + dy*thickness;
			int right = left + thickness;
			int bottom = top + thickness;
			for (int x=left; x<right; x++)
			{
				for (int y=top; y<bottom; y++)
				{
					result(x, y) = Color(255, 255, 255, 255);
				}
			}
		};

		auto put_squares = [&](std::vector<int> pattern)
		{
			for (int i=0; i<9; i++)
			{
				int dx = i%3;
				int dy = (i-dx)/3;
				if (pattern[i])
				{
					put_square(dx-1, dy-1);
				}
			}
		};

		LOG(Trace, "cx=" << cx << ", cy=" << cy << ", cl=" << cl << ", ct=" << ct << ", cw=" << cw << ", ch=" << ch << ", cr=" << cr << ", cb=" << cb);

		// Nothing can be done for the extreme case.
		if (cw > size.width || ch > size.height) return result;

		if (left && cl > 0)
		{
			LOG(Trace, "left: " << 0 << "," << cy << "," << cl-1 << "," << cy);
			DrawBoxLine(result, 0, cy, cl-1, cy, left, thickness);
		}

		if (top && ct > 0)
		{
			DrawBoxLine(result, cx, 0, cx, ct-1, top, thickness);
		}

		if (right && cr < size.width)
		{
			LOG(Trace, "right: " << cr << "," << cy << "," << size.width-1 << "," << cy);
			DrawBoxLine(result, cr, cy, size.width-1, cy, right, thickness);
		}

		if (bottom && cb < size.height)
		{
			DrawBoxLine(result, cx, cb, cx, size.height-1, bottom, thickness);
		}

		if (top == 2 && right == 2)
		{
			put_squares({1,1,1, 1,1,1, 1,1,1});
		}

		if (left == 3 && bottom == 3)
		{
			put_squares({1,1,1, 0,0,1, 1,0,1});
		}
	}

	bool DynamicTileset::Save()
	{
		int w2 = m_tile_size.width / 2;
		int h2 = m_tile_size.height / 2;

		// Add Unicode replacement character glyph (required)
		uint16_t code = kUnicodeReplacementCharacter;
		Bitmap canvas(m_tile_size, Color());
		for (int x=1; x<m_tile_size.width-1; x++)
		{
			canvas(x, 1) = Color(255, 255, 255, 255);
			canvas(x, m_tile_size.height-2) = Color(255, 255, 255, 255);
		}
		for (int y=1; y<m_tile_size.height-1; y++)
		{
			canvas(1, y) = Color(255, 255, 255, 255);
			canvas(m_tile_size.width-2, y) = Color(255, 255, 255, 255);
		}
		auto tile_slot = m_container.atlas.Add(canvas, Rectangle(m_tile_size));
		tile_slot->offset = Point(-w2, -h2);
		tile_slot->alignment = TileSlot::Alignment::Center;
		m_tiles[code] = tile_slot;
		m_container.slots[code] = tile_slot;
		LOG(Info, L"Added Unicode replacement character tile (" << m_tile_size << L")");
	}

	void DynamicTileset::Reload(DynamicTileset&& tileset)
	{
		throw std::runtime_error("DynamicTileset is not reloadable");
	}

	Size DynamicTileset::GetBoundingBoxSize()
	{
		return m_tile_size;
	}

	Tileset::Type DynamicTileset::GetType()
	{
		return Type::Dynamic;
	}

	bool DynamicTileset::Provides(uint16_t code)
	{
		// Unicode replacement character
		if (code == kUnicodeReplacementCharacter) return true;

		// Box drawing symbols
		if (code >= 0x2500 && code <= 0x257F) return true;

		return false;
	}

	void DynamicTileset::Prepare(uint16_t code)
	{
		if (!Provides(code))
		{
			throw std::runtime_error("DynamicTileset::Prepare: request for a tile which is not provided by this tileset");
		}

		if (m_tiles.count(code))
		{
			m_container.slots[code] = std::dynamic_pointer_cast<Slot>(m_tiles[code]);
		}

		Bitmap tile;

		switch (code)
		{
		case 0x2500:
			tile = MakeBoxLines(m_tile_size, 1, 0, 1, 0);
			break;
		case 0x2501:
			tile = MakeBoxLines(m_tile_size, 0, 1, 0, 1);
			break;
		case 0x2502:
			tile = MakeBoxLines(m_tile_size, 3, 0, 0, 3);
			break;
		case 0x2503:
			tile = MakeBoxLines(m_tile_size, 0, 2, 2, 0);
			break;
		default:
			tile = MakeBoxLines(m_tile_size, 1, 2, 3, 1);
		}

		int w2 = m_tile_size.width / 2;
		int h2 = m_tile_size.height / 2;
		auto tile_slot = m_container.atlas.Add(tile, Rectangle(m_tile_size));
		//*
		tile_slot->offset = Point(-w2, -h2);
		tile_slot->alignment = TileSlot::Alignment::Center;
		/*/
		tile_slot->alignment = TileSlot::Alignment::TopLeft;
		//*/
		m_tiles[code] = tile_slot;
		m_container.slots[code] = tile_slot;
		LOG(Info, L"Added character tile for code " << code);
	}
}
