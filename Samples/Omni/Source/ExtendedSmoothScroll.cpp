/*
 * ExtendedSmoothScroll.cpp
 *
 *  Created on: Dec 3, 2013
 *      Author: cfyz
 */

#include "Common.hpp"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

const int fps = 40;
const int speed_cap = 25;
const int map_size = 64;
const int tile_size = 32;

template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

void TestExtendedSmoothScroll()
{
	std::srand(std::time(nullptr));

	terminal_set("window.title='Omni: extended output / smooth scroll'");
	terminal_composition(TK_COMPOSITION_ON);

	// Load resources
	terminal_set("U+E000: dg_grounds32.png, size=32x32, alignment=top-left");

	int screen_width = terminal_state(TK_WIDTH)*terminal_state(TK_CELL_WIDTH);
	int screen_height = terminal_state(TK_HEIGHT)*terminal_state(TK_CELL_HEIGHT);
	int hspeed = 0, vspeed = 0;
	int hoffset = 0, voffset = 0;

	std::vector<int> tiles = {9, 54, 57, 60, 117, 120, 141, 168};
	std::vector<int> map(map_size*map_size, tiles[0]);
	for (int i=0; i<map_size*map_size/10; i++)
	{
		int x = std::rand()%map_size;
		int y = std::rand()%map_size;
		map[y*map_size+x] = tiles[std::rand()%tiles.size()];
	}

	for (bool proceed=true; proceed;)
	{
		hoffset -= hspeed;
		voffset -= vspeed;

		terminal_clear();

		int tx = hoffset%tile_size, ty = voffset%tile_size;
		int ix = (hoffset-tx)/tile_size, iy = (voffset-ty)/tile_size;
		int jx = ix < 0? (-ix)%map_size: map_size-(ix%map_size);
		int jy = iy < 0? (-iy)%map_size: map_size-(iy%map_size);
		int hc = std::ceil((screen_width+tile_size-tx)/(float)tile_size);
		int vc = std::ceil((screen_height+tile_size-ty)/(float)tile_size);

		terminal_printf(2, 1, "speed: %d, %d", hspeed, vspeed);
		terminal_printf(2, 2, "offset: %d/%d, %d/%d", ix, jx, iy, jy);

		for (int y=0; y<=vc; y++)
		{
			for (int x=0; x<=hc; x++)
			{
				int mx = (jx+x)%map_size;
				int my = (jy+y)%map_size;
				int c = map[my*map_size+mx];
				terminal_put_ext(0, 0, (x-1)*tile_size+tx, (y-1)*tile_size+ty, 0xE000+c, nullptr);
			}
		}

		terminal_refresh();

		while (proceed && terminal_has_input())
		{
			int key = terminal_read();
			if (key == TK_CLOSE || key == TK_ESCAPE)
			{
				proceed = false;
			}
		}

		if (terminal_state(TK_LEFT))
		{
			if (hspeed > -speed_cap) hspeed -= 1;
		}
		else if (terminal_state(TK_RIGHT))
		{
			if (hspeed < speed_cap) hspeed += 1;
		}
		else
		{
			hspeed -= sgn(hspeed);
		}

		if (terminal_state(TK_UP))
		{
			if (vspeed > -speed_cap) vspeed -= 1;
		}
		else if (terminal_state(TK_DOWN))
		{
			if (vspeed < speed_cap) vspeed += 1;
		}
		else
		{
			vspeed -= sgn(vspeed);
		}

		delay(1000/fps);
	}

	terminal_set("U+E000: none");
	terminal_composition(TK_COMPOSITION_OFF);
}
