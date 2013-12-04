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

//int minimap_width = 8;
//int map_width = 0;
//int map_height = 0;

std::vector<int> GenerateMap(int width, int height)
{
	std::vector<int> result(width*height, 0);
	auto set = [&](int x, int y, int value)
	{
		result[y*width+x] = value;
	};

	for (int x=0; x<width; x++)
	{
		set(x, 0, 1);
		set(x, height-1, 1);
	}

	for (int y=0; y<height; y++)
	{
		set(0, y, 1);
		set(width-1, y, 1);
	}

	srand(time(NULL));
	for (int i=0; i<64; i++)
	{
		set(1+rand()%(width-2), 1+rand()%(height-2), 1);
	}

	return result;
}

void TestDynamicSprites()
{
	terminal_set("window.title='Omni: dynamic sprites'");

	int minimap_size = 24;
	int map_width = terminal_state(TK_WIDTH)-minimap_size;
	int map_height = terminal_state(TK_HEIGHT);

	std::vector<int> map = GenerateMap(map_width, map_height);
	std::vector<color_t> minimap(map.size(), 0);

	int resize_to_width = (minimap_size-2)*terminal_state(TK_CELL_WIDTH);
	float factor = resize_to_width / (float)map_width;
	int resize_to_height = map_height * factor;

	for (bool proceed=true; proceed;)
	{
		terminal_clear();
		terminal_color("white");

		memset(minimap.data(), 0, sizeof(color_t)*minimap.size());
		for (int y=0; y<map_height; y++)
		{
			for (int x=0; x<map_width; x++)
			{
				if (map[y*map_width+x])
				{
					terminal_put(x, y, '#');
					minimap[y*map_width+x] = color_from_argb(255, 255, 192, 64);
				}
			}
		}
		terminal_setf("U+E001: %#p, size=%dx%d, resize=%dx%d, resize-filter=nearest", minimap.data(), map_width, map_height, resize_to_width, resize_to_height);
		terminal_put(map_width+1, 1, 0xE001);

		terminal_refresh();

		do
		{
			int key = terminal_read();

			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}

		}
		while (proceed && terminal_has_input());
	}
}
