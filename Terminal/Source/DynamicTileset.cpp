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

		if (top == 1 && bottom == 1) put_squares({0,1,0, 0,1,0, 0,1,0});
		if (left == 1 && right == 1) put_squares({0,0,0, 1,1,1, 0,0,0});

		if (left == 1 && top == 1) put_squares({0,1,0, 1,1,0, 0,0,0});
		if (top == 1 && right == 1) put_squares({0,1,0, 0,1,1, 0,0,0});
		if (right == 1 && bottom == 1) put_squares({0,0,0, 0,1,1, 0,1,0});
		if (bottom == 1 && left == 1) put_squares({0,0,0, 1,1,0, 0,1,0});

		if ((left == 2 && top == 2) ||
			(top == 2 && right == 2) ||
			(right == 2 && bottom == 2) ||
			(bottom == 2 && left == 2) ||
			(top == 2 && bottom == 2) ||
			(left == 2 && right == 2))
		{
			put_squares({1,1,1, 1,1,1, 1,1,1});
		}

		if (left == 1 && top == 2) put_squares({1,1,1, 1,1,0, 0,0,0});
		if (left == 2 && top == 1) put_squares({1,1,0, 1,1,0, 1,0,0});
		if (top == 1 && right == 2) put_squares({0,1,1, 0,1,1, 0,0,1});
		if (top == 2 && right == 1) put_squares({1,1,1, 0,1,1, 0,0,0});
		if (right == 1 && bottom == 2) put_squares({0,0,0, 0,1,1, 1,1,1});
		if (right == 2 && bottom == 1) put_squares({0,0,1, 0,1,1, 0,1,1});
		if (bottom == 1 && left == 2) put_squares({1,0,0, 1,1,0, 1,1,0});
		if (bottom == 2 && left == 1) put_squares({0,0,0, 1,1,0, 1,1,1});

		if (left == 3 && top ==3 && right == 3 && bottom == 3)
		{
			put_squares({1,0,1, 0,0,0, 1,0,1});
		}
		else
		{

			if (left == 3 && right == 3) put_squares({1,1,1, 0,0,0, 1,1,1});
			if (top == 3 && bottom == 3) put_squares({1,0,1, 1,0,1, 1,0,1});

			if (left == 3 && top == 3) put_squares({1,0,1, 0,0,1, 1,1,1});
			if (top == 3 && right == 3) put_squares({1,0,1, 1,0,0, 1,1,1});
			if (right == 3 && bottom == 3) put_squares({1,1,1, 1,0,0, 1,0,1});
			if (bottom == 3 && left == 3) put_squares({1,1,1, 0,0,1, 1,0,1});
		}
	}

	Bitmap MakeBoxLines(Size size, std::vector<int> pattern)
	{
		Bitmap result(size, Color(0, 0, 0, 0));
		if (pattern.size() < 25) return result;

		int thickness = (int)std::floor(size.width/7.0f);
		if (thickness == 0) thickness = 1;

		thickness = 1;

		int cx = (int)std::floor(size.width/2.0f-thickness/2.0f); // 8/2.0f = 4 (fifth pixel), 3/2.0 = 1[.5] (second pixel)
		int cy = (int)std::floor(size.height/2.0f-thickness/2.0f);
		int cl = cx - thickness;
		int ct = cy - thickness;
		int cw = thickness * 3;
		int ch = thickness * 3;
		int cr = cl + cw;
		int cb = ct + ch;

		auto put_rect = [&](int left, int top, int width, int height)
		{
			for (int x=left; x<left+width; x++)
			{
				for (int y=top; y<top+height; y++)
				{
					result(x, y) = Color(255, 255, 255, 255);
				}
			}
		};

		for (int dy=-1; dy<=1; dy++)
		{
			for (int dx=-1; dx<=1; dx++)
			{
				int i = (dy+2)*5+(dx+2);
				if (pattern[i]) put_rect(cx+dx*thickness, cy+dy*thickness, thickness, thickness);
			}
		}

		// Left and right
		for (int dy=-1; dy<=1; dy++)
		{
			int i1 = (dy+2)*5;
			if (pattern[i1])
			{
				for (int x=0; x<cl; x++) put_rect(x, cy+dy*thickness, 1, thickness);
			}

			int i2 = (dy+2)*5 + 4;
			if (pattern[i2])
			{
				for (int x=cr; x<size.width; x++) put_rect(x, cy+dy*thickness, 1, thickness);
			}
		}

		// Top and bottom
		for (int dx=-1; dx<=1; dx++)
		{
			int i1 = dx+2;
			if (pattern[i1])
			{
				for (int y=0; y<ct; y++) put_rect(cx+dx*thickness, y, thickness, 1);
			}

			int i2 = 4*5 + dx+2;
			if (pattern[i2])
			{
				for (int y=cb; y<size.height; y++) put_rect(cx+dx*thickness, y, thickness, 1);
			}
		}

		return result;
	}

	Bitmap MakeNotACharacterTile(Size size)
	{
		Bitmap result(size, Color());
		for (int x=1; x<size.width-1; x++)
		{
			result(x, 1) = Color(255, 255, 255, 255);
			result(x, size.height-2) = Color(255, 255, 255, 255);
		}
		for (int y=1; y<size.height-1; y++)
		{
			result(1, y) = Color(255, 255, 255, 255);
			result(size.width-2, y) = Color(255, 255, 255, 255);
		}
		return result;
	}

	bool DynamicTileset::Save()
	{
		int w2 = m_tile_size.width / 2;
		int h2 = m_tile_size.height / 2;

		// Add Unicode replacement character glyph (required)
		uint16_t code = kUnicodeReplacementCharacter;
		auto tile_slot = m_container.atlas.Add(MakeNotACharacterTile(m_tile_size), Rectangle(m_tile_size));
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
		// Light and heavy solid lines
		case 0x2500:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Single horisontal
			break;
		case 0x2501:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0}); // Wide horisontal
			break;
		case 0x2502:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}); // Single vertical
			break;
		case 0x2503:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}); // Wide vertical
			break;
		// Light and heavy dashed lines
		// 0x2504..0x250B FIXME: NYI
		// Light and heavy line box components
		case 0x250C:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,0,0}); // Single right and bottom
			break;
		case 0x250D:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,0,0}); // Wide right and single bottom
			break;
		case 0x250E:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Single right and wide bottom
			break;
		case 0x250F:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}); // Wide right and bottom
			break;
		case 0x2510:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0}); // Single left and bottom
			break;
		case 0x2511:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,1,0,0}); // Wide left and single bottom
			break;
		case 0x2512:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}); // Single left and wide bottom
			break;
		case 0x2513:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}); // Wide left and bottom
			break;
		case 0x2514:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 0,0,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Single top and right
			break;
		case 0x2515:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,0,0,0}); // Single top and wide right
			break;
		case 0x2516:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Wide top and single right
			break;
		case 0x2517:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,0,0,0,0}); // Wide top and right
			break;
		case 0x2518:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0}); // Single left and top
			break;
		case 0x2519:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,0,0,0}); // Wide left and single top
			break;
		case 0x251A:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0}); // Single left and wide top
			break;
		case 0x251B:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,0,0,0,0}); // Wide left and top
			break;
		case 0x251C:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,0,0}); // Single top, right and bottom
			break;
		case 0x251D:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,0,0}); // Single top and bottom, wide right
			break;
		case 0x251E:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}); // Wide top, single right and bottom
			break;
		case 0x251F:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Single top and right, wide bottom
			break;
		case 0x2520:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Wide top and bottom, single right
			break;
		case 0x2521:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,0,1,0,0}); // Wide top and right, single bottom
			break;
		case 0x2522:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}); // Single top, wide right and bottom
			break;
		case 0x2523:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,0,0,0,0}); // Wide top, right and bottom
			break;
		case 0x2524:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0}); // Single left, top and bottom
			break;
		case 0x2525:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,1,0,0}); // Wide left, single top and bottom
			break;
		case 0x2526:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0}); // Single left and bottom, wide top
			break;
		case 0x2527:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}); // Single left and top, wide bottom
			break;
		case 0x2528:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}); // Single left, wide top and bottom
			break;
		case 0x2529:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,0,1,0,0}); // Wide left and top, single bottom
			break;
		case 0x252A:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}); // Wide left and bottom, single top
			break;
		case 0x252B:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}); // Wide left, top and bottom
			break;
		case 0x252C:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}); // Single left, right and bottom
			break;
		case 0x252D:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,1,0,0}); // Wide left, single right and bottom
			break;
		case 0x252E:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,1,0,0}); // Single left and bottom, wide right
			break;
		case 0x252F:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}); // Wide left and right, signle bottom
			break;
		case 0x2530:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Single left and right, wide bottom
			break;
		case 0x2531:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,1,1,1,0}); // Wide left and bottom, single right
			break;
		case 0x2532:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,1,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}); // Single left, wide right and bottom
			break;
		case 0x2533:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}); // Wide left, right and bottom
			break;
		case 0x2534:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Single left, top and right
			break;
		case 0x2535:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0}); // Wide left, single top and right
			break;
		case 0x2536:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,0,0,0}); // Single left and top, wide right
			break;
		case 0x2537:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0}); // Wide left and right, single top
			break;
		case 0x2538:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Single left and right, wide top
			break;
		case 0x2539:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,0,0,0,0}); // Wide left and top, single right
			break;
		case 0x253A:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Wide top and bottom, single right
			break;
		case 0x253B:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}); // Wide left, top and bottom
			break;
		case 0x253C:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}); // Single left, top, right and bottom
			break;
		case 0x253D:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,1,0,0}); // Wide left, single top, right and bottom
			break;
		case 0x253E:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,1,0,0}); // Single left, top and bottom, wide right
			break;
		case 0x253F:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,1,0,0}); // Wide left and right, single top and bottom
			break;
		case 0x2540:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}); // Single left, right and bottom, wide top
			break;
		case 0x2541:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Single left, top and right, wide bottom
			break;
		case 0x2542:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}); // Single left and right, wide top and bottom
			break;
		case 0x2543:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,0,1,0,0}); // Wide left and top, single right and bottom
			break;
		case 0x2544:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,0,1,0,0}); // Single left and bottom, wide top and right
			break;
		case 0x2545:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,1,1,1,0}); // Wide left and bottom, single top and right
			break;
		case 0x2546:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}); // Single left and top, wide right and bottom
			break;
		case 0x2547:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,1,0,0}); // Wide left, top and right, single bottom
			break;
		case 0x2548:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}); // Wide left, right and bottom, single top
			break;
		case 0x2549:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,1,1,1,0}); // Wide left, top and bottom, single right
			break;
		case 0x254A:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}); // Single left, wide top, right and bottom
			break;
		case 0x254B:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}); // Wide left, top, right and bottom
			break;
		// Light and heavy dashed lines
		// 0x254C..0x254F
		// FIXME: NYI
		// 0x254C: // BOX DRAWINGS LIGHT DOUBLE DASH HORIZONTAL
		// 0x254D: // BOX DRAWINGS HEAVY DOUBLE DASH HORIZONTAL
		// 0x254E: // BOX DRAWINGS LIGHT DOUBLE DASH VERTICAL
		// 0x254F: // BOX DRAWINGS HEAVY DOUBLE DASH VERTICAL
		// Double lines
		case 0x2550:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0}); // Horisontal double
			break;
		case 0x2551:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0}); // Vertical double
			break;
		// Light and double line box components
		case 0x2552:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0}); // Double right, single bottom
			break;
		case 0x2553:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 0,1,1,1,1, 0,1,0,1,0, 0,1,0,1,0}); // Single right, double bottom
			break;
		case 0x2554:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,1,1,1,1, 0,1,0,0,0, 0,1,0,1,1, 0,1,0,1,0}); // Double right and bottom
			break;
		case 0x2555:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0}); // Double left and single bottom
			break;
		case 0x2556:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,0, 0,1,0,1,0, 0,1,0,1,0}); // Single left and double bottom
			break;
		case 0x2557:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,0, 0,0,0,1,0, 1,1,0,1,0, 0,1,0,1,0}); // Double left and bottom
			break;
		case 0x2558:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,1,1, 0,0,0,0,0}); // Single top and double right
			break;
		case 0x2559:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 0,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Double top and single right
			break;
		case 0x255A:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,1, 0,1,0,0,0, 0,1,1,1,1, 0,0,0,0,0}); // Double top and right
			break;
		case 0x255B:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,0,0,0}); // Double left, single up
			break;
		case 0x255C:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0}); // Single left and double top
			break;
		case 0x255D:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 1,1,0,1,0, 0,0,0,1,0, 1,1,1,1,0, 0,0,0,0,0}); // Double left and top
			break;
		case 0x255E:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0}); // Single top and bottom, double right
			break;
		case 0x255F:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,1, 0,1,0,1,0, 0,1,0,1,0}); // Double top and bottom, single right
			break;
		case 0x2560:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,1, 0,1,0,0,0, 0,1,0,1,1, 0,1,0,1,0}); // Double top, right and bottom
			break;
		case 0x2561:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0}); // Double left, single top and bottom
			break;
		case 0x2562:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 1,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0}); // Single left, double top and bottom
			break;
		case 0x2563:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 1,1,0,1,0, 0,0,0,1,0, 1,1,0,1,0, 0,1,0,1,0}); // Double left, top bottom
			break;
		case 0x2564:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,1,0,0}); // Double left and right, single bottom
			break;
		case 0x2565:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,1,0,1,0, 0,1,0,1,0}); // Single left and right, double bottom
			break;
		case 0x2566:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,0,1,1, 0,1,0,1,0}); // Double left, right and bottom
			break;
		case 0x2567:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0}); // Double left and right, single top
			break;
		case 0x2568:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Single left and right, double top
			break;
		case 0x2569:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 1,1,0,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0}); // Double left, top and right
			break;
		case 0x256A:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,1,0,0}); // Double left and right, single top and bottom
			break;
		case 0x256B:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 0,1,0,1,0, 1,1,0,1,1, 0,1,0,1,0, 0,1,0,1,0}); // Single left and right, double top and bottom
			break;
		case 0x256C:
			tile = MakeBoxLines(m_tile_size, {0,1,0,1,0, 1,1,0,1,1, 0,0,0,0,0, 1,1,0,1,1, 0,1,0,1,0}); // Double horisontal and vertical
			break;
		default:
			tile = MakeNotACharacterTile(m_tile_size);
			break;
		}

		auto tile_slot = m_container.atlas.Add(tile, Rectangle(m_tile_size));
		/*
		int w2 = m_tile_size.width / 2;
		int h2 = m_tile_size.height / 2;
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
