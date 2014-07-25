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

			m_container.atlas.Remove(i.second);
		}

		m_tiles.clear();
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

	Bitmap MakeDashLines(Size size, bool vertical, bool thick, int parts)
	{
		Bitmap result(size, Color(0, 0, 0, 0));

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

		auto put_rect = [&](int left, int top, int width, int height, int alpha)
		{
			for (int x=left; x<left+width; x++)
			{
				for (int y=top; y<top+height; y++)
				{
					result(x, y) = Color(alpha, 255, 255, 255);
				}
			}
		};

		int length = vertical? size.height: size.width;
		int n = (int)std::floor((length+1)/2.0f);
		if (n > parts) n = parts;
		float p0 = length / (float)n;
		float p1 = p0 / 2.0f;
		int gap = (int)std::floor(p1);
		if (gap < 1) gap = 1;
		int dash = (int)std::floor((length-gap*(n-1))/(float)n);
		int total = dash*n + gap*(n-1);
		int as = 0, ae = 0;
		if (total < length)
		{
			float extra = (length-total)/2.0f;
			as = (int)std::floor(extra);
			ae = (int)std::ceil(extra);
		}

		if (vertical)
		{
			int t = thick? 3: 1;
			int l = cx - (t-1)/2*thickness;
			int w = t*thickness;

			put_rect(l, 0, w, as, 255);
			for (int i=0; i<n; i++)
			{
				put_rect(l, as+i*(dash+gap), w, dash, 255);
			}
			put_rect(l, length-ae, w, ae, 255);
		}
		else
		{
			int t = thick? 3: 1;
			int p = cy - (t-1)/2*thickness;
			int h = t*thickness;

			put_rect(0, p, as, h, 255);
			for (int i=0; i<n; i++)
			{
				put_rect(as+i*(dash+gap), p, dash, h, 255);
			}
			put_rect(length-ae, p, ae, h, 255);
		}

		return result;
	}

	Bitmap MakeVerticalSplit(Size size, float from, float to)
	{
		Bitmap result(size, Color(0, 0, 0, 0));

		auto put_rect = [&](int left, int top, int width, int height, int alpha)
		{
			for (int x=left; x<left+width; x++)
			{
				for (int y=top; y<top+height; y++)
				{
					result(x, y) = Color(alpha, 255, 255, 255);
				}
			}
		};

		int tt = (int)std::floor(from*size.height);
		int tb = (int)std::ceil(from*size.height);
		int bt = (int)std::floor(to*size.height);
		int bb = (int)std::ceil(to*size.height);

		if (bt > tb) put_rect(0, tb, size.width, bt-tb, 255);
		if (tt < tb) put_rect(0, tt, size.width, 1, (tb-from)*255);
		if (bt < bb) put_rect(0, bt, size.width, 1, (to-bt)*255);

		return result;
	}

	Bitmap MakeHorisontalSplit(Size size, float from, float to)
	{
		Bitmap result(size, Color(0, 0, 0, 0));

		auto put_rect = [&](int left, int top, int width, int height, int alpha)
		{
			for (int x=left; x<left+width; x++)
			{
				for (int y=top; y<top+height; y++)
				{
					result(x, y) = Color(alpha, 255, 255, 255);
				}
			}
		};

		int ll = (int)std::floor(from*size.width);
		int lr = (int)std::ceil(from*size.width);
		int rl = (int)std::floor(to*size.width);
		int rr = (int)std::ceil(to*size.width);

		if (rl > lr) put_rect(lr, 0, rl-lr, size.height, 255);
		if (ll < lr) put_rect(ll, 0, 1, size.height, (lr-from)*255);
		if (rl < rr) put_rect(rr-1, 0, 1, size.height, (to-rl)*255);

		return result;
	}

	Bitmap MakeQuadrandTile(Size size, bool top_left, bool top_right, bool bottom_left, bool bottom_right)
	{
		Bitmap result(size, Color(0,0,0,0));//255,255,255));

		auto put_rect = [&](int left, int top, int width, int height, int alpha)
		{
			for (int x=left; x<left+width; x++)
			{
				for (int y=top; y<top+height; y++)
				{
					result(x, y) = Color(alpha, 255, 255, 255);
					//result(x, y).a += alpha;
				}
			}
		};

		float cx = size.width / 2.0f;
		float cy = size.height / 2.0f;

		int l = (int)std::floor(cx);
		int r = (int)std::ceil(cx);
		int t = (int)std::floor(cy);
		int b = (int)std::ceil(cy);

		// Round up to top-left corner
		r = l;
		b = t;

		if (top_left) put_rect(0, 0, l, t, 255);
		if (top_right) put_rect(r, 0, size.width-r, t, 255);
		if (bottom_left) put_rect(0, b, l, size.height-b, 255);
		if (bottom_right) put_rect(r, b, size.width-r, size.height-b, 255);

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

	Size DynamicTileset::GetSpacing()
	{
		return Size(1, 1);
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
		if (code >= 0x2580 && code <= 0x259F) return true;

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
		case 0x2504:
			tile = MakeDashLines(m_tile_size, false, false, 3); // Single triple horisontal dash
			break;
		case 0x2505:
			tile = MakeDashLines(m_tile_size, false, true, 3); // Wide triple horisontal dash
			break;
		case 0x2506:
			tile = MakeDashLines(m_tile_size, true, false, 3); // Single triple vertical dash
			break;
		case 0x2507:
			tile = MakeDashLines(m_tile_size, true, true, 3); // Wide triple horisontal dash
			break;
		case 0x2508:
			tile = MakeDashLines(m_tile_size, false, false, 4); // Singlee quadruple horisontal dash
			break;
		case 0x2509:
			tile = MakeDashLines(m_tile_size, false, true, 4); // Wide quadruple horisontal dash
			break;
		case 0x250A:
			tile = MakeDashLines(m_tile_size, true, false, 4); // Single quadruple vertical dash
			break;
		case 0x250B:
			tile = MakeDashLines(m_tile_size, true, true, 4); // Wide quadruple vertical dash
			break;
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
		case 0x254C:
			tile = MakeDashLines(m_tile_size, false, false, 2); // BOX DRAWINGS LIGHT DOUBLE DASH HORIZONTAL
			break;
		case 0x254D:
			tile = MakeDashLines(m_tile_size, false, true, 2); // BOX DRAWINGS HEAVY DOUBLE DASH HORIZONTAL
			break;
		case 0x254E:
			tile = MakeDashLines(m_tile_size, true, false, 2); // BOX DRAWINGS LIGHT DOUBLE DASH VERTICAL
			break;
		case 0x254F:
			tile = MakeDashLines(m_tile_size, true, true, 2); // BOX DRAWINGS HEAVY DOUBLE DASH VERTICAL
			break;
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
		// Light and heavy half lines
		case 0x2574:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0}); // Single left
			break;
		case 0x2575:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0}); // Single up
			break;
		case 0x2576:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,0,0,0,0, 0,0,0,0,0}); // Single right
			break;
		case 0x2577:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}); // Single down
			break;
		case 0x2578:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,0,0,0}); // Wide left
			break;
		case 0x2579:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0}); // Wide up
			break;
		case 0x257A:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,0,0,0}); // Wide right
			break;
		case 0x257B:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}); // Wide down
			break;
		// Mixed light and heavy lines
		case 0x257C:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,0,0,0}); // Single left and wide right
			break;
		case 0x257D:
			tile = MakeBoxLines(m_tile_size, {0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}); // Single up and wide down
			break;
		case 0x257E:
			tile = MakeBoxLines(m_tile_size, {0,0,0,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0}); // Wide left and single right
			break;
		case 0x257F:
			tile = MakeBoxLines(m_tile_size, {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0}); // Wide up and single down
			break;
		// Block elements
		case 0x2580:
			tile = MakeVerticalSplit(m_tile_size, 0.0f, 0.5f); // ▀ UPPER HALF BLOCK
			break;
		case 0x2581:
			tile = MakeVerticalSplit(m_tile_size, 1.0f-0.125f, 1.0f); // ▁ LOWER ONE EIGHTH BLOCK
			break;
		case 0x2582:
			tile = MakeVerticalSplit(m_tile_size, 0.75f, 1.0f); // ▂ LOWER ONE QUARTER BLOCK
			break;
		case 0x2583:
			tile = MakeVerticalSplit(m_tile_size, 1.0f-3*0.125f, 1.0f); // ▃ LOWER THREE EIGHTHS BLOCK
			break;
		case 0x2584:
			tile = MakeVerticalSplit(m_tile_size, 0.5f, 1.0f); // ▄ LOWER HALF BLOCK
			break;
		case 0x2585:
			tile = MakeVerticalSplit(m_tile_size, 1.0f-5*0.125f, 1.0f); // ▅ LOWER FIVE EIGHTHS BLOCK
			break;
		case 0x2586:
			tile = MakeVerticalSplit(m_tile_size, 0.25f, 1.0f); // ▆ LOWER THREE QUARTERS BLOCK
			break;
		case 0x2587:
			tile = MakeVerticalSplit(m_tile_size, 0.125f, 1.0f); // ▇ LOWER SEVEN EIGHTHS BLOCK
			break;
		case 0x2588:
			tile = MakeVerticalSplit(m_tile_size, 0.0f, 1.0f); // █ FULL BLOCK
			break;
		case 0x2589:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 7*0.125f); // ▉ LEFT SEVEN EIGHTHS BLOCK
			break;
		case 0x258A:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 0.75f); // ▊ LEFT THREE QUARTERS BLOCK
			break;
		case 0x258B:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 5*0.125f); // ▋ LEFT FIVE EIGHTHS BLOCK
			break;
		case 0x258C:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 0.5f); // ▌ LEFT HALF BLOCK
			break;
		case 0x258D:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 3*0.125f); // ▍ LEFT THREE EIGHTHS BLOCK
			break;
		case 0x258E:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 0.25f); // ▎ LEFT ONE QUARTER BLOCK
			break;
		case 0x258F:
			tile = MakeHorisontalSplit(m_tile_size, 0.0f, 0.125f); // ▏ LEFT ONE EIGHTH BLOCK
			break;
		case 0x2590:
			tile = MakeHorisontalSplit(m_tile_size, 0.5f, 1.0f); // ▐ RIGHT HALF BLOCK
			break;
		// Shade characters
		case 0x2591:
			tile = Bitmap(m_tile_size, Color(64, 255, 255, 255)); // ░ LIGHT SHADE
			break;
		case 0x2592:
			tile = Bitmap(m_tile_size, Color(128, 255, 255, 255)); // ▒ MEDIUM SHADE
			break;
		case 0x2593:
			tile = Bitmap(m_tile_size, Color(192, 255, 255, 255)); // ▓ DARK SHADE
			break;
		// Block elements
		case 0x2594:
			tile = MakeVerticalSplit(m_tile_size, 0.0f, 0.125f); // ▔ UPPER ONE EIGHTH BLOCK
			break;
		case 0x2595:
			tile = MakeHorisontalSplit(m_tile_size, 1.0f-0.125f, 1.0f); // ▕ RIGHT ONE EIGHTH BLOCK
			break;
		// Terminal graphic characters
		case 0x2596:
			tile = MakeQuadrandTile(m_tile_size, false, false, true, false); // ▖ QUADRANT LOWER LEFT
			break;
		case 0x2597:
			tile = MakeQuadrandTile(m_tile_size, false, false, false, true); // ▗ QUADRANT LOWER RIGHT
			break;
		case 0x2598:
			tile = MakeQuadrandTile(m_tile_size, true, false, false, false); // ▘ QUADRANT UPPER LEFT
			break;
		case 0x2599:
			tile = MakeQuadrandTile(m_tile_size, true, false, true, true); // ▙ QUADRANT UPPER LEFT AND LOWER LEFT AND LOWER RIGHT
			break;
		case 0x259A:
			tile = MakeQuadrandTile(m_tile_size, true, false, false, true); // ▚ QUADRANT UPPER LEFT AND LOWER RIGHT
			break;
		case 0x259B:
			tile = MakeQuadrandTile(m_tile_size, true, true, true, false); // ▛ QUADRANT UPPER LEFT AND UPPER RIGHT AND LOWER LEFT
			break;
		case 0x259C:
			tile = MakeQuadrandTile(m_tile_size, true, true, false, true); // ▜ QUADRANT UPPER LEFT AND UPPER RIGHT AND LOWER RIGHT
			break;
		case 0x259D:
			tile = MakeQuadrandTile(m_tile_size, false, true, false, false); // ▝ QUADRANT UPPER RIGHT
			break;
		case 0x259E:
			tile = MakeQuadrandTile(m_tile_size, false, true, true, false); // ▞ QUADRANT UPPER RIGHT AND LOWER LEFT
			break;
		case 0x259F:
			tile = MakeQuadrandTile(m_tile_size, false, true, true, true); // ▟ QUADRANT UPPER RIGHT AND LOWER LEFT AND LOWER RIGHT
			break;
		default:
			tile = MakeNotACharacterTile(m_tile_size);
			break;
		}

		if (tile.IsEmpty()) tile = MakeNotACharacterTile(m_tile_size);

		auto tile_slot = m_container.atlas.Add(tile, Rectangle(m_tile_size));
		tile_slot->alignment = TileSlot::Alignment::TopLeft;
		m_tiles[code] = tile_slot;
		m_container.slots[code] = tile_slot;
		LOG(Info, L"Added character tile for code " << code);
	}
}
