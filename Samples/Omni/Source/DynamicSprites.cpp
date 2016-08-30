/*
 * DynamicSprites.cpp
 *
 *  Created on: Dec 2, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <string>
#include <map>

std::vector<std::wstring> map =
{
	L"                             ",
	L" ------                      ", //       -  Wall
	L" |....|      ############    ", //       #  Unlit hallway
	L" |....|      #          #    ", //       .  Lit area
	L" |.$..+########         #    ", //       $  Some quantity of gold
	L" |....|       #      ---+--- ", //       +  A door
	L" ------       #      |.....| ", //       |  Wall
	L"              #      |.!...| ", //       !  A magic potion
	L"              #      |.....| ", //
	L"              #      |.....| ", //       @  The adventurer
	L"   ----       #      |.....| ", //
	L"   |..|       #######+.....| ", //       D  A red dragon
	L"   |..+###    #      |.....| ", //       <  Stairs to a higher level
	L"   ----  #    #      |.?...| ", //       ?  A magic scroll
	L"         ######      ------- ",
	L"                             "
};

struct symbol_t
{
	color_t color;
	int tile;
};

std::map<wchar_t, symbol_t> palette =
{
	{L'-', {0xFFAAAAAA, 8}},
	{L'|', {0xFFAAAAAA, 8}},
	{L'.', {0xFF808080, 20}},
	{L'#', {0xFF808080, 20}},
	{L'+', {0xFFFF8000, 21}},
	{L'!', {0xFFFF00FF, 22}},
	{L'D', {0xFFFF0000, 6}},
	{L'?', {0xFF008000, 23}},
	{L'$', {0xFFFFFF00, 29}},
	{L'<', {0xFFFFFFFF, 9}}
};

void TestDynamicSprites()
{
	terminal_set("window.title='Omni: dynamic sprites'");
	terminal_set("U+E000: ../Media/Tiles.png, size=32x32, align=top-left");

	int map_width = map[0].length();
	int map_height = map.size();
	std::vector<color_t> minimap(map_width*map_height, 0);
	int x0 = 0;
	int y0 = 0;
	int view_height = 10;
	int view_width = 14;
	int minimap_scale = 4;
	int panel_width = (terminal_state(TK_WIDTH) - view_width*4 - 1)*terminal_state(TK_CELL_WIDTH);
	int margin = (panel_width - map_width*minimap_scale)/2;

	auto DrawMap = [&]
	{
		terminal_color("white");
		for (int y=y0; y<y0+view_height; y++)
		{
			for (int x=x0; x<x0+view_width; x++)
			{
				wchar_t code = map[y][x];
				if (!palette.count(code)) continue;
				symbol_t s = palette[code];
				terminal_put((x-x0)*4, (y-y0)*2, 0xE000+s.tile);
			}
		}
	};

	auto BlendColors = [](color_t one, color_t two) -> color_t
	{
		uint8_t* pone = (uint8_t*)&one;
		uint8_t* ptwo = (uint8_t*)&two;
		float f = ptwo[3] / 255.0f;
		for (int i=0; i<3; i++) pone[i] = pone[i]*(1.0f-f) + ptwo[i]*f;
		return one;
	};

	auto MakeMinimap = [&]
	{
		for (int y=0; y<map_height; y++)
		{
			for (int x=0; x<map_width && x<map[y].length(); x++)
			{
				wchar_t code = map[y][x];
				color_t color = palette.count(code)? palette[code].color: 0xFF000000;
				minimap[y*map_width+x] = color;
			}
		}

		for (int y=y0; y<y0+view_height; y++)
		{
			for (int x=x0; x<x0+view_width; x++)
			{
				color_t& dst = minimap[y*map_width+x];
				dst = BlendColors(dst, 0x60FFFFFF);
			}
		}

		terminal_setf
		(
			"U+E100: %#p, raw-size=%dx%d, resize=%dx%d, resize-filter=nearest",
			(void*)minimap.data(),
			map_width, map_height,
			map_width*4, map_height*4
		);
	};

	while (true)
	{
		terminal_clear();

		DrawMap();
		terminal_color("light gray");
		for (int x=0; x<80; x++) terminal_put(x, view_height*2, 0x2580);
		for (int y=0; y<view_height*2; y++) terminal_put(view_width*4, y, 0x2588);

		MakeMinimap();
		terminal_color("white");
		terminal_put_ext(view_width*4+1, 0, margin, margin, 0xE100, nullptr);

		terminal_print(1, view_height*2+1, "[color=orange]Tip:[/color] use arrow keys to move viewport over the map");

		terminal_refresh();

		int key = terminal_read();

		if (key == TK_CLOSE || key == TK_ESCAPE)
		{
			break;
		}
		else if (key == TK_RIGHT && (x0 < map_width-view_width))
		{
			x0 += 1;
		}
		else if (key == TK_LEFT && x0 > 0)
		{
			x0 -= 1;
		}
		else if (key == TK_DOWN && (y0 < map_height-view_height))
		{
			y0 += 1;
		}
		else if (key == TK_UP && y0 > 0)
		{
			y0 -= 1;
		}
	}

	terminal_set("U+E000: none; U+E100: none;");
}
