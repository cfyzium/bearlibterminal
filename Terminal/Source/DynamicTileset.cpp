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

#include "DynamicTileset.hpp"
#include "Log.hpp"
#include "Bitmap.hpp"
#include "Utility.hpp"
#include "Geometry.hpp"
#include "Encoding.hpp"
#include <cmath>

namespace BearLibTerminal
{
	DynamicTileset::DynamicTileset(char32_t offset, OptionGroup& options):
		Tileset(offset)
	{
		if (!options.attributes.count(L"size"))
		{
			throw std::runtime_error("DynamicTileset: 'size' attribute is missing");
		}

		if (!try_parse(options.attributes[L"size"], m_tile_size))
		{
			throw std::runtime_error("DynamicTileset: failed to parse 'size' attribute");
		}
	}

	DynamicTileset::DynamicTileset(char32_t offset, Size cell_size):
		Tileset(offset),
		m_tile_size(cell_size)
	{ }

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

	Size DynamicTileset::GetBoundingBoxSize()
	{
		return m_tile_size;
	}

	bool DynamicTileset::Provides(char32_t code)
	{
		return IsDynamicTile(code);
	}

	const char box_lines[][25] =
	{
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2500
		{0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0}, // 2501
		{0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}, // 2502
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}, // 2503
		{0}, // 2504
		{0}, // 2505
		{0}, // 2506
		{0}, // 2507
		{0}, // 2508
		{0}, // 2509
		{0}, // 250A
		{0}, // 250B
		{0,0,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,0,0}, // 250C: Single right and bottom
		{0,0,0,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,0,0}, // 250D: Wide right and single bottom
		{0,0,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 250E: Single right and wide bottom
		{0,0,0,0,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}, // 250F: Wide right and bottom
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0}, // 2510: Single left and bottom
		{0,0,0,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,1,0,0}, // 2511: Wide left and single bottom
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}, // 2512: Single left and wide bottom
		{0,0,0,0,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}, // 2513: Wide left and bottom
		{0,0,1,0,0, 0,0,1,0,0, 0,0,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2514: Single top and right
		{0,0,1,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,0,0,0}, // 2515: Single top and wide right
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2516: Wide top and single right
		{0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,0,0,0,0}, // 2517: Wide top and right
		{0,0,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0}, // 2518: Single left and top
		{0,0,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,0,0,0}, // 2519: Wide left and single top
		{0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0}, // 251A: Single left and wide top
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,0,0,0,0}, // 251B: Wide left and top
		{0,0,1,0,0, 0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,0,0}, // 251C: Single top, right and bottom
		{0,0,1,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,0,0}, // 251D: Single top and bottom, wide right
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}, // 251E: Wide top, single right and bottom
		{0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 251F: Single top and right, wide bottom
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 2520: Wide top and bottom, single right
		{0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,0,1,0,0}, // 2521: Wide top and right, single bottom
		{0,0,1,0,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}, // 2522: Single top, wide right and bottom
		{0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}, // 2523: Wide top, right and bottom
		{0,0,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0}, // 2524: Single left, top and bottom
		{0,0,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,1,0,0}, // 2525: Wide left, single top and bottom
		{0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0}, // 2526: Single left and bottom, wide top
		{0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}, // 2527: Single left and top, wide bottom
		{0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}, // 2528: Single left, wide top and bottom
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,0,1,0,0}, // 2529: Wide left and top, single bottom
		{0,0,1,0,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}, // 252A: Wide left and bottom, single top
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}, // 252B: Wide left, top and bottom
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}, // 252C: Single left, right and bottom
		{0,0,0,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,1,0,0}, // 252D: Wide left, single right and bottom
		{0,0,0,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,1,0,0}, // 252E: Single left and bottom, wide right
		{0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}, // 252F: Wide left and right, signle bottom
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 2530: Single left and right, wide bottom
		{0,0,0,0,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,1,1,1,0}, // 2531: Wide left and bottom, single right
		{0,0,0,0,0, 0,1,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}, // 2532: Single left, wide right and bottom
		{0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}, // 2533: Wide left, right and bottom
		{0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2534: Single left, top and right
		{0,0,1,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0}, // 2535: Wide left, single top and right
		{0,0,1,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,0,0,0}, // 2536: Single left and top, wide right
		{0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0}, // 2537: Wide left and right, single top
		{0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2538: Single left and right, wide top
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,0,0,0,0}, // 2539: Wide left and top, single right
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 253A: Wide top and bottom, single right
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 1,1,1,1,0, 0,1,1,1,0}, // 253B: Wide left, top and bottom
		{0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}, // 253C: Single left, top, right and bottom
		{0,0,1,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,1,0,0}, // 253D: Wide left, single top, right and bottom
		{0,0,1,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,1,0,0}, // 253E: Single left, top and bottom, wide right
		{0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,1,0,0}, // 253F: Wide left and right, single top and bottom
		{0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0}, // 2540: Single left, right and bottom, wide top
		{0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 2541: Single left, top and right, wide bottom
		{0,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 0,1,1,1,0, 0,1,1,1,0}, // 2542: Single left and right, wide top and bottom
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,0,1,0,0}, // 2543: Wide left and top, single right and bottom
		{0,1,1,1,0, 0,1,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,0,1,0,0}, // 2544: Single left and bottom, wide top and right
		{0,0,1,0,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,1,1,1,0}, // 2545: Wide left and bottom, single top and right
		{0,0,1,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}, // 2546: Single left and top, wide right and bottom
		{0,1,1,1,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,1,0,0}, // 2547: Wide left, top and right, single bottom
		{0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}, // 2548: Wide left, right and bottom, single top
		{0,1,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 1,1,1,1,0, 0,1,1,1,0}, // 2549: Wide left, top and bottom, single right
		{0,1,1,1,0, 0,1,1,1,1, 1,1,1,1,1, 0,1,1,1,1, 0,1,1,1,0}, // 254A: Single left, wide top, right and bottom
		{0,1,1,1,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,1,1,1,0}, // 254B: Wide left, top, right and bottom
		{0}, // 254C
		{0}, // 254D
		{0}, // 254E
		{0}, // 254F
		{0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0}, // 2550: Horisontal double
		{0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0}, // 2551: Vertical double
		{0,0,0,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0}, // 2552: Double right, single bottom
		{0,0,0,0,0, 0,0,0,0,0, 0,1,1,1,1, 0,1,0,1,0, 0,1,0,1,0}, // 2553: Single right, double bottom
		{0,0,0,0,0, 0,1,1,1,1, 0,1,0,0,0, 0,1,0,1,1, 0,1,0,1,0}, // 2554: Double right and bottom
		{0,0,0,0,0, 1,1,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0}, // 2555: Double left and single bottom
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,0, 0,1,0,1,0, 0,1,0,1,0}, // 2556: Single left and double bottom
		{0,0,0,0,0, 1,1,1,1,0, 0,0,0,1,0, 1,1,0,1,0, 0,1,0,1,0}, // 2557: Double left and bottom
		{0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,1,1, 0,0,0,0,0}, // 2558: Single top and double right
		{0,1,0,1,0, 0,1,0,1,0, 0,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2559: Double top and single right
		{0,1,0,1,0, 0,1,0,1,1, 0,1,0,0,0, 0,1,1,1,1, 0,0,0,0,0}, // 255A: Double top and right
		{0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,0,0,0}, // 255B: Double left, single up
		{0,1,0,1,0, 0,1,0,1,0, 1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0}, // 255C: Single left and double top
		{0,1,0,1,0, 1,1,0,1,0, 0,0,0,1,0, 1,1,1,1,0, 0,0,0,0,0}, // 255D: Double left and top
		{0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0, 0,0,1,1,1, 0,0,1,0,0}, // 255E: Single top and bottom, double right
		{0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,1, 0,1,0,1,0, 0,1,0,1,0}, // 255F: Double top and bottom, single right
		{0,1,0,1,0, 0,1,0,1,1, 0,1,0,0,0, 0,1,0,1,1, 0,1,0,1,0}, // 2560: Double top, right and bottom
		{0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0, 1,1,1,0,0, 0,0,1,0,0}, // 2561: Double left, single top and bottom
		{0,1,0,1,0, 0,1,0,1,0, 1,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0}, // 2562: Single left, double top and bottom
		{0,1,0,1,0, 1,1,0,1,0, 0,0,0,1,0, 1,1,0,1,0, 0,1,0,1,0}, // 2563: Double left, top bottom
		{0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,1,0,0}, // 2564: Double left and right, single bottom
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,1,0,1,0, 0,1,0,1,0}, // 2565: Single left and right, double bottom
		{0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,0,1,1, 0,1,0,1,0}, // 2566: Double left, right and bottom
		{0,0,1,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0}, // 2567: Double left and right, single top
		{0,1,0,1,0, 0,1,0,1,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2568: Single left and right, double top
		{0,1,0,1,0, 1,1,0,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0}, // 2569: Double left, top and right
		{0,0,1,0,0, 1,1,1,1,1, 0,0,0,0,0, 1,1,1,1,1, 0,0,1,0,0}, // 256A: Double left and right, single top and bottom
		{0,1,0,1,0, 0,1,0,1,0, 1,1,0,1,1, 0,1,0,1,0, 0,1,0,1,0}, // 256B: Single left and right, double top and bottom
		{0,1,0,1,0, 1,1,0,1,1, 0,0,0,0,0, 1,1,0,1,1, 0,1,0,1,0}, // 256C: Double horisontal and vertical
		{0}, // 256D
		{0}, // 256E
		{0}, // 256F
		{0}, // 2570
		{0}, // 2571
		{0}, // 2572
		{0}, // 2573
		{0,0,0,0,0, 0,0,0,0,0, 1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0}, // 2574: Single left
		{0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0}, // 2575: Single up
		{0,0,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // 2576: Single right
		{0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}, // 2577: Single down
		{0,0,0,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,0,0,0}, // 2578: Wide left
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0}, // 2579: Wide up
		{0,0,0,0,0, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,0,0,0}, // 257A: Wide right
		{0,0,0,0,0, 0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}, // 257B: Wide down
		{0,0,0,0,0, 0,0,1,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,0,0,0}, // 257C: Single left and wide right
		{0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0}, // 257D: Single up and wide down
		{0,0,0,0,0, 1,1,1,0,0, 1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0}, // 257E: Wide left and single right
		{0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0}, // 257F: Wide up and single down
	};

	const float splits[][3] =
	{
		{0, 0.0f, 0.5f},          // 2580: ▀ UPPER HALF BLOCK
		{0, 1.0f-0.125f, 1.0f},   // 2581: ▁ LOWER ONE EIGHTH BLOCK
		{0, 0.75f, 1.0f},         // 2582: ▂ LOWER ONE QUARTER BLOCK
		{0, 1.0f-3*0.125f, 1.0f}, // 2583: ▃ LOWER THREE EIGHTHS BLOCK
		{0, 0.5f, 1.0f},          // 2584: ▄ LOWER HALF BLOCK
		{0, 1.0f-5*0.125f, 1.0f}, // 2585: ▅ LOWER FIVE EIGHTHS BLOCK
		{0, 0.25f, 1.0f},         // 2586: ▆ LOWER THREE QUARTERS BLOCK
		{0, 0.125f, 1.0f},        // 2587: ▇ LOWER SEVEN EIGHTHS BLOCK
		{0, 0.0f, 1.0f},          // 2588: █ FULL BLOCK
		{1, 0.0f, 7*0.125f},      // 2589: ▉ LEFT SEVEN EIGHTHS BLOCK
		{1, 0.0f, 0.75f},         // 258A: ▊ LEFT THREE QUARTERS BLOCK
		{1, 0.0f, 5*0.125f},      // 258B: ▋ LEFT FIVE EIGHTHS BLOCK
		{1, 0.0f, 0.5f},          // 258C: ▌ LEFT HALF BLOCK
		{1, 0.0f, 3*0.125f},      // 258D: ▍ LEFT THREE EIGHTHS BLOCK
		{1, 0.0f, 0.25f},         // 258E: ▎ LEFT ONE QUARTER BLOCK
		{1, 0.0f, 0.125f},        // 258F: ▏ LEFT ONE EIGHTH BLOCK
		{1, 0.5f, 1.0f},          // 2590: ▐ RIGHT HALF BLOCK
		{0}, // 2591
		{0}, // 2592,
		{0}, // 2593
		{0, 0.0f, 0.125f},         // 2594: ▔ UPPER ONE EIGHTH BLOCK
		{1, 1.0f-0.125f, 1.0f}     // 2595:▕ RIGHT ONE EIGHTH BLOCK
	};

	const bool quadrants[][4] =
	{
		{false, false, true,  false}, // 2596: ▖ QUADRANT LOWER LEFT
		{false, false, false, true},  // 2597: ▗ QUADRANT LOWER RIGHT
		{true,  false, false, false}, // 2598: ▘ QUADRANT UPPER LEFT
		{true,  false, true,  true},  // 2599: ▙ QUADRANT UPPER LEFT AND LOWER LEFT AND LOWER RIGHT
		{true,  false, false, true},  // 259A:  ▚ QUADRANT UPPER LEFT AND LOWER RIGHT
		{true,  true,  true,  false}, // 259B: ▛ QUADRANT UPPER LEFT AND UPPER RIGHT AND LOWER LEFT
		{true,  true,  false, true},  // 259C: ▜ QUADRANT UPPER LEFT AND UPPER RIGHT AND LOWER RIGHT
		{false, true,  false, false}, // 259D: ▝ QUADRANT UPPER RIGHT
		{false, true,  true,  false}, // 259E: ▞ QUADRANT UPPER RIGHT AND LOWER LEFT
		{false, true,  true,  true}   // 259F: ▟ QUADRANT UPPER RIGHT AND LOWER LEFT AND LOWER RIGHT
	};

	bool IsDynamicTile(char32_t code)
	{
		code = (code & Tileset::kCharOffsetMask);

		return (code >= 0x2500 && code <= 0x259F) || code == kUnicodeReplacementCharacter;
	}

	Bitmap GenerateDynamicTile(char32_t code, Size size)
	{
		code = (code & Tileset::kCharOffsetMask);

		if ((code >= 0x2500 && code <= 0x2503) ||
			(code >= 0x250C && code <= 0x254B) ||
			(code >= 0x2550 && code <= 0x256C) ||
			(code >= 0x2574 && code <= 0x257F))
		{
			int i = code - 0x2500;
			std::vector<int> tmp;
			for (int j=0; j<25; j++)
				tmp.push_back(box_lines[i][j]);
			return MakeBoxLines(size, tmp);
		}
		else if ((code >= 0x2580 && code <= 0x2590) || (code >= 0x2594 && code <= 0x2595))
		{
			int i = code - 0x2580;
			if (splits[i][0] > 0)
				return MakeHorisontalSplit(size, splits[i][1], splits[i][2]);
			else
				return MakeVerticalSplit(size, splits[i][1], splits[i][2]);
		}
		else if (code >= 0x2596 && code <= 0x259F)
		{
			int i = code - 0x2596;
			return MakeQuadrandTile(size, quadrants[i][0], quadrants[i][1], quadrants[i][2], quadrants[i][3]);
		}
		else
		{
			switch (code)
			{
			// U+2500..U+2503: Light and heavy solid lines
			// U+2504..U+250B: Light and heavy dashed lines
			case 0x2504:
				return MakeDashLines(size, false, false, 3); // Single triple horisontal dash
				break;
			case 0x2505:
				return MakeDashLines(size, false, true, 3); // Wide triple horisontal dash
				break;
			case 0x2506:
				return MakeDashLines(size, true, false, 3); // Single triple vertical dash
				break;
			case 0x2507:
				return MakeDashLines(size, true, true, 3); // Wide triple horisontal dash
				break;
			case 0x2508:
				return MakeDashLines(size, false, false, 4); // Singlee quadruple horisontal dash
				break;
			case 0x2509:
				return MakeDashLines(size, false, true, 4); // Wide quadruple horisontal dash
				break;
			case 0x250A:
				return MakeDashLines(size, true, false, 4); // Single quadruple vertical dash
				break;
			case 0x250B:
				return MakeDashLines(size, true, true, 4); // Wide quadruple vertical dash
				break;
			// U+250C..U+254B: Light and heavy line box components
			// U+254C..U+254F: Light and heavy dashed lines
			case 0x254C:
				return MakeDashLines(size, false, false, 2); // BOX DRAWINGS LIGHT DOUBLE DASH HORIZONTAL
				break;
			case 0x254D:
				return MakeDashLines(size, false, true, 2); // BOX DRAWINGS HEAVY DOUBLE DASH HORIZONTAL
				break;
			case 0x254E:
				return MakeDashLines(size, true, false, 2); // BOX DRAWINGS LIGHT DOUBLE DASH VERTICAL
				break;
			case 0x254F:
				return MakeDashLines(size, true, true, 2); // BOX DRAWINGS HEAVY DOUBLE DASH VERTICAL
				break;
			// U+2550..U+2551: Double lines
			// U+2552..U+256C: Light and double line box components
			// U+2574..U+257B: Light and heavy half lines
			// U+257C..U+257F: Mixed light and heavy lines
			// U+2580..U+2590: Block elements 1
			// U+2591..U+2593: Shade characters
			case 0x2591:
				return Bitmap(size, Color(64, 255, 255, 255)); // ░ LIGHT SHADE
				break;
			case 0x2592:
				return Bitmap(size, Color(128, 255, 255, 255)); // ▒ MEDIUM SHADE
				break;
			case 0x2593:
				return Bitmap(size, Color(192, 255, 255, 255)); // ▓ DARK SHADE
				break;
			// U+2594..U+2595: Block elements 2
			// U+2596..U+259F: Block elements 3 (quadrants)
			default:
				return MakeNotACharacterTile(size);
				break;
			}
		}

		return MakeNotACharacterTile(size);
	}

	std::shared_ptr<TileInfo> DynamicTileset::Get(char32_t code)
	{
		if (!Provides(code))
		{
			throw std::runtime_error("DynamicTileset::Prepare: request for a tile which is not provided by this tileset");
		}

		auto i = m_cache.find(code);
		if (i != m_cache.end())
		{
			return i->second;
		}

		Size spacing{1, 1};
		char32_t font_offset = (code & Tileset::kFontOffsetMask);
		auto j = g_tilesets.find(font_offset);\
		if (j != g_tilesets.end())
		{
			spacing = j->second->GetSpacing();
		}

		Bitmap tile = GenerateDynamicTile(code, m_tile_size * spacing);

		auto tile_ref = std::make_shared<TileInfo>();
		tile_ref->tileset = this;
		tile_ref->alignment = TileAlignment::TopLeft;
		tile_ref->spacing = spacing;
		tile_ref->bitmap = tile;
		m_cache[code] = tile_ref;
		return tile_ref;
	}
}
