/*
 * Sprites.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

void TestSprites()
{
	terminal_set("window.title='Omni: sprites'");

	// TODO: background, bmp/png/jpeg images

	// Load background
	int screen_width = terminal_state(TK_WIDTH)*terminal_state(TK_CELL_WIDTH);
	int screen_height = terminal_state(TK_HEIGHT)*terminal_state(TK_CELL_HEIGHT);
	//terminal_setf("U+E000: Fallen_bg.png, resize=%dx%d", screen_width, screen_height);
	terminal_setf("U+E000: Fallen_bg.jpg, resize=%dx%d", screen_width, screen_height);
	terminal_set("U+E001: tile_01.png, resize=96x96, resize-filter=nearest");
	terminal_set("U+E002: tile_01.png, resize=48x48, resize-filter=nearest");
	terminal_set("U+E004: tile_01.png, resize=128x128, resize-filter=bilinear");

	//color_t c = color_from_argb(128, 64, 128, 192);
	//terminal_setf("U+E003: %#p, size=1x1, resize=64x64", &c);

	color_t c[] =
	{
		color_from_argb(128, 192, 64, 64),
		color_from_argb(128, 64, 192, 64),
		color_from_argb(128, 64, 64, 192),
		color_from_argb(128, 192, 192, 64)
	};
	terminal_setf("U+E003: %#p, size=2x2, resize=64x64", &c);

	terminal_clear();
	terminal_put(0, 0, 0xE000); // Background
	terminal_put(15, 5, 0xE001);
	terminal_put(25, 5, 0xE002);
	terminal_put(50, 5, 0xE004);
	terminal_layer(1);
	terminal_put(8, 4, 0xE003);
	terminal_layer(0);
	terminal_refresh();
	terminal_read();

	terminal_set("U+E000: none");
}
